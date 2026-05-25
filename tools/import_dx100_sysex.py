#!/usr/bin/env python3
"""
Convert DX100 / DX21 / DX27 / TX81Z sysex (.syx) to SoundB0ard fm_presets.json.

Supported sysex formats:
  - VMEM bulk dump  (32 voices): F0 43 0n 04 20 00 [4096 bytes] checksum F7
  - Single voice edit buffer:    F0 43 0n 03 00 5D [93 bytes]   checksum F7

Voice parameter layout (VMEM, 128 bytes per voice):
  Bytes 0-51 : 4 operators × 13 bytes, order: OP4, OP3, OP2, OP1
  Bytes 52+  : voice globals (algorithm, feedback, LFO, name …)

This is a best-effort conversion.  Key approximations:
  - DX100 EG uses rates (0-31 = slowest→fastest); converted to ms via
    an exponential curve.  Envelope *character* is preserved but not
    bit-accurate to the hardware.
  - DX100 frequency coarse (1-31 → ratio 0.5-15.5) is mapped directly.
    Fine frequency is ignored (minor pitch shift only).
  - DX100/DX21/DX27 only have sine wave operators; TX81Z adds WS 0-7
    which is mapped to our waveform index.
  - Feedback is per-algorithm on the DX100 (0-7); written to all four
    op feedback slots scaled ×10 (→ 0-70).

Usage:
  python3 import_dx100_sysex.py path/to/file.syx [output.json]

  If output.json is omitted, presets are merged into:
    settings/fm_presets.json  (relative to the project root)

  Duplicate names are suffixed _2, _3, … to avoid collisions.
"""

import json
import math
import os
import sys
from pathlib import Path

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
SCRIPT_DIR = Path(__file__).parent
PROJECT_ROOT = SCRIPT_DIR.parent
DEFAULT_OUTPUT = PROJECT_ROOT / "settings" / "fm_presets.json"

# ---------------------------------------------------------------------------
# EG rate → millisecond conversion
# DX rates: 0 = slowest, 31 = fastest
# Release rates: 0-15 (RR), others 0-31
# ---------------------------------------------------------------------------
def rate_to_ms(rate: int, max_rate: int = 31, max_ms: float = 8000.0) -> float:
    """Exponential rate→time curve. rate 31 → ~1 ms, rate 0 → max_ms."""
    if rate >= max_rate:
        return 1.0
    frac = rate / max_rate          # 0.0 (slow) … 1.0 (fast)
    return round(max_ms * (1.0 - frac) ** 2.5, 1)

# ---------------------------------------------------------------------------
# Frequency coarse → ratio
# DX100: coarse 0 = fixed-freq mode (treat as 1.0), 1 = 0.5, 2 = 1.0, ...
# ---------------------------------------------------------------------------
def coarse_to_ratio(coarse: int, fixed: bool) -> float:
    if fixed:
        return 1.0   # fixed-frequency mode: use 1.0 (fundamental) as fallback
    if coarse == 0:
        return 0.5   # coarse 0 → lowest ratio 0.5
    return min(15.5, coarse * 0.5)

# ---------------------------------------------------------------------------
# DX detune (0-6, centre=3) → cents
# One unit ≈ 7 cents on the hardware
# ---------------------------------------------------------------------------
def det_to_cents(det: int) -> float:
    return (det - 3) * 7.0

# ---------------------------------------------------------------------------
# TX81Z waveform select (0-7) → our waveform index
# DX100 only has WS=0 (sine); TX81Z adds saw, double-sine, etc.
# Our synth: 0=sine 1=saw 2=tri 3=square (others clamped)
# ---------------------------------------------------------------------------
DX_WS_MAP = {0: 0, 1: 1, 2: 2, 3: 3, 4: 0, 5: 1, 6: 2, 7: 3}

def ws_to_waveform(ws: int) -> int:
    return DX_WS_MAP.get(ws & 0x07, 0)

# ---------------------------------------------------------------------------
# LFO wave: 0=tri 1=sawdown 2=sawup 3=square 4=sine 5=samplehold
# Our LFO: 0=sine 1=tri 2=saw 3=square — best-effort map
# ---------------------------------------------------------------------------
DX_LFO_WAVE_MAP = {0: 1, 1: 2, 2: 2, 3: 3, 4: 0, 5: 0}

# ---------------------------------------------------------------------------
# Parse one operator block (13 bytes)
# Returns a dict of decoded values.
# ---------------------------------------------------------------------------
def parse_op(data: bytes, base: int) -> dict:
    # Per-operator layout: 10 bytes (confirmed empirically from real DX100 sysex)
    ar   = data[base + 0] & 0x1F       # attack rate   0-31
    d1r  = data[base + 1] & 0x1F       # decay 1 rate  0-31
    d2r  = data[base + 2] & 0x1F       # decay 2 rate  0-31
    rr   = data[base + 3] & 0x0F       # release rate  0-15
    d1l  = data[base + 4] & 0x0F       # decay 1 level (sustain breakpoint) 0-15
    ls   = data[base + 5] & 0x7F       # level scaling 0-99
    b6   = data[base + 6]              # packed RS/EBS/AME/KVS (ignored in conversion)
    out  = data[base + 7] & 0x7F       # output level  0-99
    b8   = data[base + 8]
    fixd = (b8 >> 5) & 0x01            # 0=ratio mode, 1=fixed-frequency mode
    freq = b8 & 0x1F                   # coarse frequency 0-31
    b9   = data[base + 9]
    ws   = (b9 >> 3) & 0x07            # waveform select (TX81Z; DX100=always 0=sine)
    det  = b9 & 0x07                   # detune 0-6 (centre=3 → 0 cents)

    return {
        "attack_ms":    rate_to_ms(ar, 31),
        "decay_ms":     rate_to_ms(d1r, 31),
        "sustain_lvl":  round(1.0 - d1l / 15.0, 4),   # D1L 0=full sustain, 15=no sustain
        "release_ms":   rate_to_ms(rr, 15, 4000.0),
        "output_lvl":   float(out),
        "ratio":        coarse_to_ratio(freq, bool(fixd)),
        "detune_cents": det_to_cents(det),
        "waveform":     ws_to_waveform(ws),
    }

# ---------------------------------------------------------------------------
# Parse one 128-byte VMEM voice into a SoundB0ard preset dict
# ---------------------------------------------------------------------------
def parse_voice(data: bytes, offset: int) -> dict:
    v = data[offset: offset + 128]

    # Operators stored OP1 first, 10 bytes each (confirmed empirically)
    op = [parse_op(v,  0),   # OP1
          parse_op(v, 10),   # OP2
          parse_op(v, 20),   # OP3
          parse_op(v, 30)]   # OP4

    # Voice globals (bytes 40-56, confirmed empirically from real DX100 sysex)
    lfo_speed = v[40] & 0x7F    # 0-99
    lfo_delay = v[41] & 0x7F    # 0-99 (ignored — no direct equivalent)
    lfo_pmd   = v[42] & 0x7F    # pitch mod depth 0-99
    lfo_amd   = v[43] & 0x7F    # amp mod depth   0-99
    b44       = v[44]           # LFO wave/sync packed (best-effort)
    lfo_wave  = (b44 >> 1) & 0x03
    lfo_sync  = b44 & 0x01
    b45       = v[45]
    pms       = b45 & 0x07      # pitch mod sensitivity 0-7
    ams       = (b45 >> 3) & 0x03  # amp mod sensitivity 0-3
    fb        = (v[46] >> 3) & 0x07   # feedback 0-7
    alg       = v[47] & 0x07    # algorithm 0-7 → voice_mode 1-8
    pb_range  = v[48] & 0x0F    # pitch bend range 0-12
    port_time = v[49] & 0x7F    # portamento time 0-99

    # Name: 10 chars at bytes 57-66 (confirmed)
    name_bytes = v[57:67]
    name = "".join(chr(b) if 32 <= b < 127 else "" for b in name_bytes).strip().replace(" ", "_")
    name = name or "untitled"

    # Scale feedback 0-7 → 0-70
    fb_scaled = float(fb * 10)

    # LFO rate: DX speed 0-99 → our 0.02-20 Hz range (log scale)
    lfo_rate = round(0.02 + (lfo_speed / 99.0) ** 2 * 19.98, 3)
    # LFO intensity from PMD/AMD (use whichever is nonzero, normalise 0-99→0-1)
    lfo_intensity = round(max(lfo_pmd, lfo_amd) / 99.0, 4)

    preset = {
        "m_voice_mode":  alg + 1,          # 1-8
        "m_volume_db":   0.0,
        "m_pitchbend_range": int(pb_range),
        "m_portamento_time_ms": round(port_time * 50.0, 1),  # rough scale
        "m_velocity_to_attack_scaling": False,
        "m_note_number_to_decay_scaling": False,
        "m_reset_to_zero": False,
        "m_legato_mode": False,

        # OP1
        "m_op1_waveform":    op[0]["waveform"],
        "m_op1_ratio":       op[0]["ratio"],
        "m_op1_detune_cents": op[0]["detune_cents"],
        "m_eg1_attack_ms":   op[0]["attack_ms"],
        "m_eg1_decay_ms":    op[0]["decay_ms"],
        "m_eg1_sustain_lvl": op[0]["sustain_lvl"],
        "m_eg1_release_ms":  op[0]["release_ms"],
        "m_op1_output_lvl":  op[0]["output_lvl"],
        "m_op1_feedback":    fb_scaled,

        # OP2
        "m_op2_waveform":    op[1]["waveform"],
        "m_op2_ratio":       op[1]["ratio"],
        "m_op2_detune_cents": op[1]["detune_cents"],
        "m_eg2_attack_ms":   op[1]["attack_ms"],
        "m_eg2_decay_ms":    op[1]["decay_ms"],
        "m_eg2_sustain_lvl": op[1]["sustain_lvl"],
        "m_eg2_release_ms":  op[1]["release_ms"],
        "m_op2_output_lvl":  op[1]["output_lvl"],
        "m_op2_feedback":    fb_scaled,

        # OP3
        "m_op3_waveform":    op[2]["waveform"],
        "m_op3_ratio":       op[2]["ratio"],
        "m_op3_detune_cents": op[2]["detune_cents"],
        "m_eg3_attack_ms":   op[2]["attack_ms"],
        "m_eg3_decay_ms":    op[2]["decay_ms"],
        "m_eg3_sustain_lvl": op[2]["sustain_lvl"],
        "m_eg3_release_ms":  op[2]["release_ms"],
        "m_op3_output_lvl":  op[2]["output_lvl"],
        "m_op3_feedback":    fb_scaled,

        # OP4
        "m_op4_waveform":    op[3]["waveform"],
        "m_op4_ratio":       op[3]["ratio"],
        "m_op4_detune_cents": op[3]["detune_cents"],
        "m_eg4_attack_ms":   op[3]["attack_ms"],
        "m_eg4_decay_ms":    op[3]["decay_ms"],
        "m_eg4_sustain_lvl": op[3]["sustain_lvl"],
        "m_eg4_release_ms":  op[3]["release_ms"],
        "m_op4_output_lvl":  op[3]["output_lvl"],
        "m_op4_feedback":    fb_scaled,

        # LFO
        "m_lfo1_rate":       lfo_rate,
        "m_lfo1_intensity":  lfo_intensity,
        "m_lfo1_waveform":   DX_LFO_WAVE_MAP.get(lfo_wave, 0),
        "m_lfo1_mod_dest1":  1 if lfo_pmd > 0 else 0,   # 1=vibrato
        "m_lfo1_mod_dest2":  2 if lfo_amd > 0 else 0,   # 2=amp mod
        "m_lfo1_mod_dest3":  0,
        "m_lfo1_mod_dest4":  0,
    }
    return name, preset

# ---------------------------------------------------------------------------
# Parse a .syx file — returns list of (name, preset_dict)
# ---------------------------------------------------------------------------
def parse_syx(path: Path) -> list[tuple[str, dict]]:
    raw = path.read_bytes()
    results = []
    i = 0

    while i < len(raw):
        # Find next F0
        if raw[i] != 0xF0:
            i += 1
            continue

        # Need at least 6 header bytes
        if i + 6 >= len(raw):
            break

        manufacturer = raw[i + 1]
        if manufacturer != 0x43:          # not Yamaha
            i += 1
            continue

        sub_status = raw[i + 2] & 0xF0   # ignore channel nibble
        fmt        = raw[i + 3]
        byte_cnt_hi = raw[i + 4]
        byte_cnt_lo = raw[i + 5]
        data_len = (byte_cnt_hi << 7) | byte_cnt_lo

        if fmt == 0x04 and data_len == 4096:
            # VMEM bulk dump: 32 voices × 128 bytes
            data_start = i + 6
            data_end   = data_start + 4096
            if data_end + 2 > len(raw):
                print(f"  WARNING: truncated bulk dump at offset {i}", file=sys.stderr)
                break
            data = raw[data_start:data_end]
            for v in range(32):
                name, preset = parse_voice(data, v * 128)
                results.append((name, preset))
            i = data_end + 2   # skip checksum + F7

        elif fmt == 0x03 and data_len == 93:
            # Single voice edit buffer dump
            data_start = i + 6
            data_end   = data_start + 93
            if data_end + 2 > len(raw):
                print(f"  WARNING: truncated single voice at offset {i}", file=sys.stderr)
                break
            # Single voice uses the same 93-byte unpacked format;
            # pad to 128 so parse_voice can reuse the same offsets.
            padded = raw[data_start:data_end] + bytes(128 - 93)
            name, preset = parse_voice(padded, 0)
            results.append((name, preset))
            i = data_end + 2

        else:
            i += 1   # unknown format, skip

    return results

# ---------------------------------------------------------------------------
# Unique name helper
# ---------------------------------------------------------------------------
def unique_name(name: str, existing: set) -> str:
    if name not in existing:
        return name
    n = 2
    while f"{name}_{n}" in existing:
        n += 1
    return f"{name}_{n}"

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
def main():
    if len(sys.argv) < 2:
        print("Usage: import_dx100_sysex.py <file.syx> [output.json]")
        sys.exit(1)

    syx_path = Path(sys.argv[1])
    if not syx_path.exists():
        print(f"ERROR: {syx_path} not found", file=sys.stderr)
        sys.exit(1)

    out_path = Path(sys.argv[2]) if len(sys.argv) >= 3 else DEFAULT_OUTPUT

    # Parse sysex
    print(f"Parsing {syx_path} ...")
    patches = parse_syx(syx_path)
    if not patches:
        print("No recognisable DX100/TX81Z sysex found in file.", file=sys.stderr)
        sys.exit(1)
    print(f"  Found {len(patches)} patches")

    # Load existing JSON (or start fresh)
    existing: dict = {}
    if out_path.exists():
        try:
            existing = json.loads(out_path.read_text())
        except Exception as e:
            print(f"  WARNING: could not read {out_path}: {e}")

    existing_names = set(existing.keys())
    added = 0
    for raw_name, preset in patches:
        name = unique_name(raw_name, existing_names)
        existing[name] = preset
        existing_names.add(name)
        added += 1
        if name != raw_name:
            print(f"  Renamed duplicate '{raw_name}' → '{name}'")

    out_path.write_text(json.dumps(existing, indent=2) + "\n")
    print(f"  Wrote {added} presets to {out_path}  (total: {len(existing)})")

if __name__ == "__main__":
    main()
