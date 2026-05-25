#!/usr/bin/env python3
"""
Hand-crafted SD (snare) and CP (clap) presets for SoundB0ard's drum synth.

SD synthesis:
  - Two oscillators (lo_osc at MIDI pitch, hi_osc at 2x) + noise through HPF2
  - `tone`     = HPF cutoff Hz — higher = brighter/airier noise
  - `decay`    = main oscillator EG decay (ms) — the "body" length
  - `noise_decay` = noise EG decay (ms) — the "rattle/snap" length
  - `noise_vol`   = noise amplitude vs tone balance
  - `octave`+`key` = MIDI pitch of tone oscillators (Midi2Freq((octave+1)*12 + key))
  - `attack` / `noise_attack` = EG attack times (usually very short)

CP synthesis:
  - Noise through BPF2, amplitude modulated by LFO (creates "multiple hands" texture)
  - `tone`   = BPF center Hz — the characteristic crack frequency
  - `fq`     = BPF Q — higher = more focused/resonant crack
  - `nvol`   = noise amplitude
  - `ndecay` = noise EG decay (ms)
  - `lfo_rate` = LFO Hz — higher = faster repeat flutter in the clap body
  - `lfo_type` = 1 = usaw (best for percussive attack repeats)
  - `eg_decay` = overall amplitude envelope decay

Usage:
  python3 tools/add_classic_drum_presets.py
"""

import json
from pathlib import Path

SCRIPT_DIR = Path(__file__).parent
PROJECT_ROOT = SCRIPT_DIR.parent
SD_FILE = PROJECT_ROOT / "settings" / "drum_sd_presets.json"
CP_FILE = PROJECT_ROOT / "settings" / "drum_cp_presets.json"


def sd(
    vol=0.75, pan=0.0,
    noise_vol=0.6, noise_decay=35.0, tone=1000.0, decay=85.0,
    octave=3, key=7,
    lo_osc_waveform=0, hi_osc_waveform=0,
    distortion_threshold=0.5,
    attack=1.0, noise_attack=1.5,
    use_delay=False, delay_mode=0, delay_ms=23.0,
    delay_feedback_pct=0.0, delay_ratio=0.0,
    delay_wetmix=0.5, delay_sync_tempo=True, delay_sync_len=0,
):
    return {
        "vol": vol, "pan": pan,
        "noise_vol": noise_vol, "noise_decay": noise_decay,
        "tone": tone, "decay": decay,
        "octave": float(octave), "key": float(key),
        "lo_osc_waveform": float(lo_osc_waveform),
        "hi_osc_waveform": float(hi_osc_waveform),
        "distortion_threshold": distortion_threshold,
        "attack": attack, "noise_attack": noise_attack,
        "use_delay": use_delay, "delay_mode": float(delay_mode),
        "delay_ms": delay_ms, "delay_feedback_pct": delay_feedback_pct,
        "delay_ratio": delay_ratio, "delay_wetmix": delay_wetmix,
        "delay_sync_tempo": delay_sync_tempo,
        "delay_sync_len": float(delay_sync_len),
    }


def cp(
    vol=0.75, pan=0.0,
    nvol=0.7, nattack=12.0, ndecay=110.0,
    tone=1200.0, fq=5.0,
    eg_attack=12.0, eg_decay=85.0, eg_sustain=0.35, eg_release=110.0,
    lfo_type=1, lfo_rate=4.0,
    distortion_threshold=0.5,
    use_delay=False, delay_mode=0, delay_ms=23.0,
    delay_feedback_pct=0.0, delay_ratio=0.0,
    delay_wetmix=0.5, delay_sync_tempo=True, delay_sync_len=0,
):
    return {
        "vol": vol, "pan": pan,
        "nvol": nvol, "nattack": nattack, "ndecay": ndecay,
        "tone": tone, "fq": fq,
        "eg_attack": eg_attack, "eg_decay": eg_decay,
        "eg_sustain": eg_sustain, "eg_release": eg_release,
        "lfo_type": float(lfo_type), "lfo_rate": lfo_rate,
        "distortion_threshold": distortion_threshold,
        "use_delay": use_delay, "delay_mode": float(delay_mode),
        "delay_ms": delay_ms, "delay_feedback_pct": delay_feedback_pct,
        "delay_ratio": delay_ratio, "delay_wetmix": delay_wetmix,
        "delay_sync_tempo": delay_sync_tempo,
        "delay_sync_len": float(delay_sync_len),
    }


# ---------------------------------------------------------------------------
# Snare presets
# Pitch reference: Midi2Freq((octave+1)*12 + key)
#   octave=2 key=7  → G2  ~98Hz   (deep/fat)
#   octave=3 key=5  → F3  ~175Hz  (mid, classic)
#   octave=3 key=7  → G3  ~196Hz  (standard)
#   octave=3 key=9  → A3  ~220Hz  (bright)
#   octave=4 key=0  → C4  ~261Hz  (sharp/electronic)
# ---------------------------------------------------------------------------
SD_PATCHES = {

    # Tight crisp snare — bright HPF, short noise decay, punchy.
    # Sits well in fast patterns without washing out.
    "SNAPPY": sd(
        noise_vol=0.78, noise_decay=18, tone=1900, decay=38,
        octave=3, key=9,
        attack=1.0, noise_attack=1.0,
        distortion_threshold=0.45,
    ),

    # Deep, fat snare — low-pitched oscillators, warm HPF, long body.
    # Funk/soul territory. Mix with a kick that has space below it.
    "FATBACK": sd(
        noise_vol=0.42, noise_decay=55, tone=480, decay=130,
        octave=2, key=7,
        attack=3.0, noise_attack=2.0,
        distortion_threshold=0.5,
    ),

    # Sharp rimshot crack — very high HPF, ultra-short everything.
    # Almost no tone body; just the transient crack.
    "RIMSHOT": sd(
        noise_vol=0.92, noise_decay=8, tone=4500, decay=14,
        octave=4, key=0,
        attack=1.0, noise_attack=1.0,
        distortion_threshold=0.38,
    ),

    # Saturated electronic snare — driven distortion, mid HPF.
    # Classic drum machine grit without going fully digital.
    "ELECTRO": sd(
        noise_vol=0.72, noise_decay=22, tone=1400, decay=52,
        octave=3, key=7,
        attack=1.0, noise_attack=1.0,
        distortion_threshold=0.28,
    ),

    # Roomy snare — warmer HPF, longer noise and body decay.
    # Sounds like it was recorded in a live room with some ambience.
    "ROOM_SD": sd(
        noise_vol=0.55, noise_decay=85, tone=650, decay=155,
        octave=3, key=5,
        attack=2.0, noise_attack=2.0,
        distortion_threshold=0.5,
    ),

    # Ghost note — low volume, subtle. Use alongside SNAPPY/FATBACK
    # to add soft in-between hits without adding a new character.
    "GHOST": sd(
        vol=0.32, noise_vol=0.5, noise_decay=20, tone=1000, decay=40,
        octave=3, key=7,
        attack=1.0, noise_attack=1.0,
        distortion_threshold=0.5,
    ),
}

# ---------------------------------------------------------------------------
# Clap presets
# LFO rate creates the "multiple hands" layered texture:
#   slow (2-4Hz)  = spacious, fewer layers
#   mid  (5-8Hz)  = classic 808/909 feel
#   fast (10-15Hz) = tight, almost rattly
# Tone (BPF center):
#   low  (500-900Hz)   = dark/boxy
#   mid  (1000-2000Hz) = classic crack
#   high (3000-8000Hz) = crisp/electronic
# ---------------------------------------------------------------------------
CP_PATCHES = {

    # Snappy, dry clap — high BPF, tight noise, fast LFO.
    # Cuts through dense mixes without much tail.
    "SNAP_CP": cp(
        nvol=0.88, nattack=2, ndecay=32, tone=2800, fq=9.0,
        eg_attack=2, eg_decay=28, eg_sustain=0.0, eg_release=25,
        lfo_type=1, lfo_rate=14.0,
        distortion_threshold=0.42,
    ),

    # 909-style clap — brighter tone, higher Q than 808, faster LFO.
    # The classic house/techno clap on the 2 and 4.
    "NINE09": cp(
        nvol=0.78, nattack=8, ndecay=75, tone=1750, fq=8.0,
        eg_attack=8, eg_decay=55, eg_sustain=0.28, eg_release=70,
        lfo_type=1, lfo_rate=7.0,
        distortion_threshold=0.45,
    ),

    # Finger snap — very high BPF, ultra-short decay, maximum Q.
    # More of an accent hit than a full clap.
    "FINGERSNAP": cp(
        nvol=0.9, nattack=1, ndecay=12, tone=5200, fq=10.0,
        eg_attack=1, eg_decay=16, eg_sustain=0.0, eg_release=14,
        lfo_type=1, lfo_rate=12.0,
        distortion_threshold=0.4,
    ),

    # Deep, boxy clap — low BPF, wide Q, slow LFO, long tail.
    # Works well in hip-hop and slow-tempo tracks.
    "DEEP_CP": cp(
        nvol=0.56, nattack=20, ndecay=200, tone=580, fq=2.5,
        eg_attack=20, eg_decay=160, eg_sustain=0.22, eg_release=180,
        lfo_type=1, lfo_rate=2.5,
        distortion_threshold=0.5,
    ),

    # Roomy/live clap — medium tone, wide Q, long decay for ambience.
    # Sounds like a real room clap recorded with some reverb.
    "ROOM_CP": cp(
        nvol=0.6, nattack=15, ndecay=280, tone=950, fq=3.0,
        eg_attack=15, eg_decay=230, eg_sustain=0.18, eg_release=260,
        lfo_type=1, lfo_rate=3.0,
        distortion_threshold=0.5,
    ),

    # Dirty/saturated clap — heavy distortion, mid tone.
    # Gritty, raw sound for hip-hop, lo-fi, and grime.
    "DIRTY_CP": cp(
        nvol=0.82, nattack=8, ndecay=90, tone=1500, fq=7.0,
        eg_attack=8, eg_decay=72, eg_sustain=0.28, eg_release=88,
        lfo_type=1, lfo_rate=6.0,
        distortion_threshold=0.2,
    ),
}


def merge_presets(path, patches):
    existing = {}
    if path.exists():
        try:
            existing = json.loads(path.read_text())
        except Exception as e:
            print(f"  WARNING: could not read {path}: {e}")
    added = 0
    for name, p in patches.items():
        if name in existing:
            print(f"  Skipping '{name}' — already exists")
            continue
        existing[name] = p
        added += 1
    path.write_text(json.dumps(existing, indent=2) + "\n")
    print(f"  Wrote {added} presets to {path}  (total: {len(existing)})")


def main():
    print("Adding SD presets...")
    merge_presets(SD_FILE, SD_PATCHES)
    print("Adding CP presets...")
    merge_presets(CP_FILE, CP_PATCHES)


if __name__ == "__main__":
    main()
