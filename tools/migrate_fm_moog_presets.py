#!/usr/bin/env python3
"""Migrate fmpresets.dat and moogpresets.dat to JSON flat-map format."""

import json
import sys
from pathlib import Path

SETTINGS_DIR = Path(__file__).parent.parent / "settings"

FM_DAT = SETTINGS_DIR / "fmpresets.dat"
FM_JSON = SETTINGS_DIR / "fm_presets.json"

MOOG_DAT = SETTINGS_DIR / "moogpresets.dat"
MOOG_JSON = SETTINGS_DIR / "moog_presets.json"

# FM fields that are C++ bool (must be JSON true/false, not 0.0/1.0)
FM_BOOL_FIELDS = {
    "m_velocity_to_attack_scaling",
    "m_note_number_to_decay_scaling",
    "m_reset_to_zero",
    "m_legato_mode",
    "m_op1_sustain_override",
    "m_op2_sustain_override",
    "m_op3_sustain_override",
    "m_op4_sustain_override",
}

# Moog fields that are C++ bool
MOOG_BOOL_FIELDS = {
    "monophonic",
    "lfo1_osc_pitch_enabled",
    "lfo1_filter_fc_enabled",
    "lfo1_amp_enabled",
    "lfo1_pan_enabled",
    "lfo1_pulsewidth_enabled",
    "lfo2_osc_pitch_enabled",
    "lfo2_filter_fc_enabled",
    "lfo2_amp_enabled",
    "lfo2_pan_enabled",
    "lfo2_pulsewidth_enabled",
    "eg1_osc_enabled",
    "eg1_filter_enabled",
    "eg1_dca_enabled",
    "legato_mode",
    "reset_to_zero",
    "filter_keytrack",
    "nlp",
    "velocity_to_attack_scaling",
    "note_number_to_decay_scaling",
    "sustain_override",
}


def parse_dat_line(line: str, bool_fields: set) -> tuple[str, dict] | None:
    """Parse one ::key=val:: line, return (name, dict) or None."""
    line = line.strip().strip(":")
    if not line:
        return None

    kv: dict = {}
    for token in line.split("::"):
        token = token.strip()
        if "=" not in token:
            continue
        key, _, val = token.partition("=")
        key = key.strip()
        val = val.strip()
        if not key:
            continue
        if key == "name":
            kv["__name__"] = val
        else:
            try:
                fval = float(val)
                kv[key] = bool(fval) if key in bool_fields else fval
            except ValueError:
                pass  # skip non-numeric

    name = kv.pop("__name__", None)
    if name is None:
        return None
    return name, kv


def migrate(dat_path: Path, json_path: Path, bool_fields: set, label: str):
    if not dat_path.exists():
        print(f"  ERROR: {dat_path} not found", file=sys.stderr)
        return

    result: dict = {}
    with dat_path.open() as f:
        for line in f:
            parsed = parse_dat_line(line, bool_fields)
            if parsed:
                name, preset = parsed
                result[name] = preset

    with json_path.open("w") as f:
        json.dump(result, f, indent=2)
        f.write("\n")

    print(f"  {label}: {len(result)} presets -> {json_path}")


if __name__ == "__main__":
    print("Migrating FM presets...")
    migrate(FM_DAT, FM_JSON, FM_BOOL_FIELDS, "FM")

    print("Migrating Moog presets...")
    migrate(MOOG_DAT, MOOG_JSON, MOOG_BOOL_FIELDS, "Moog")

    print("Done.")
