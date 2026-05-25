#!/usr/bin/env python3
"""Migrate drumpresets.dat to drum_kits.json and per-instrument preset files.

Usage:
    python3 tools/migrate_drum_presets.py [settings_dir]

Reads settings/drumpresets.dat, writes:
  settings/drum_kits.json       - full kits (DrumSettings)
  settings/drum_bd_presets.json - BdSettings keyed by kit name
  settings/drum_sd_presets.json - SdSettings keyed by kit name
  settings/drum_hh_presets.json - HhSettings keyed by kit name (closed hat)
  settings/drum_hh2_presets.json- HhSettings keyed by kit name (open hat)
  settings/drum_cp_presets.json - CpSettings keyed by kit name
  settings/drum_fm_presets.json - FmDrumSettings keyed by "<kit>_fm1/2/3"
  settings/drum_lz_presets.json - LazerSettings keyed by kit name
"""

import json
import os
import sys


# Fields declared as bool in the C++ structs — must be JSON true/false
BOOL_FIELDS = {
    "use_distortion", "hard_sync", "use_delay", "delay_sync_tempo",
}


PREFIXES = {
    "bd": ["bd_"],
    "sd": ["sd_"],
    "hh": ["hh_"],
    "hh2": ["hh2_"],
    "cp": ["cp_"],
    "fm1": ["fm1_"],
    "fm2": ["fm2_"],
    "fm3": ["fm3_"],
    "lz": ["lz_"],
}

# Map old .dat key prefixes to struct field names (strip prefix)
def extract_part(flat: dict, prefixes: list[str]) -> dict:
    result = {}
    for k, v in flat.items():
        for pfx in prefixes:
            if k.startswith(pfx):
                field = k[len(pfx):]
                # Convert 0.0/1.0 to proper JSON booleans for bool C++ fields
                if field in BOOL_FIELDS:
                    v = bool(v)
                result[field] = v
                break
    return result


def parse_dat_line(line: str) -> dict:
    """Parse one ::key=val:: line into a dict."""
    fields = {}
    for token in line.split("::"):
        token = token.strip()
        if "=" not in token:
            continue
        key, _, val = token.partition("=")
        key = key.strip()
        val = val.strip()
        if key == "name":
            fields[key] = val
        else:
            try:
                fields[key] = float(val)
            except ValueError:
                fields[key] = val
    return fields


def main():
    settings_dir = sys.argv[1] if len(sys.argv) > 1 else "settings"
    dat_path = os.path.join(settings_dir, "drumpresets.dat")

    if not os.path.exists(dat_path):
        print(f"Not found: {dat_path}")
        sys.exit(1)

    kits = {}
    bd_presets = {}
    sd_presets = {}
    hh_presets = {}
    hh2_presets = {}
    cp_presets = {}
    fm_presets = {}
    lz_presets = {}

    with open(dat_path) as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            flat = parse_dat_line(line)
            name = flat.get("name")
            if not name:
                continue

            # Build full kit dict matching DrumSettings JSON layout
            kit = {
                "name": name,
                "volume": flat.get("volume", 1.0),
                "bd": extract_part(flat, ["bd_"]),
                "sd": extract_part(flat, ["sd_"]),
                "hh": extract_part(flat, ["hh_"]),
                "hh2": extract_part(flat, ["hh2_"]),
                "cp": extract_part(flat, ["cp_"]),
                "fm1": extract_part(flat, ["fm1_"]),
                "fm2": extract_part(flat, ["fm2_"]),
                "fm3": extract_part(flat, ["fm3_"]),
                "lz": extract_part(flat, ["lz_"]),
            }
            kits[name] = kit
            bd_presets[name] = kit["bd"]
            sd_presets[name] = kit["sd"]
            hh_presets[name] = kit["hh"]
            hh2_presets[name] = kit["hh2"]
            cp_presets[name] = kit["cp"]
            fm_presets[f"{name}_fm1"] = kit["fm1"]
            fm_presets[f"{name}_fm2"] = kit["fm2"]
            fm_presets[f"{name}_fm3"] = kit["fm3"]
            lz_presets[name] = kit["lz"]

    def write(filename, data):
        path = os.path.join(settings_dir, filename)
        with open(path, "w") as f:
            json.dump(data, f, indent=2)
            f.write("\n")
        print(f"Wrote {len(data)} entries → {path}")

    write("drum_kits.json", kits)
    write("drum_bd_presets.json", bd_presets)
    write("drum_sd_presets.json", sd_presets)
    write("drum_hh_presets.json", hh_presets)
    write("drum_hh2_presets.json", hh2_presets)
    write("drum_cp_presets.json", cp_presets)
    write("drum_fm_presets.json", fm_presets)
    write("drum_lz_presets.json", lz_presets)

    print(f"\nMigrated {len(kits)} presets from {dat_path}")


if __name__ == "__main__":
    main()
