#!/usr/bin/env python3
"""
Hand-crafted classic Minimoog-style presets for SoundB0ard's subsynth.

Voice modes:  0=Saw3  1=Sqr3  2=Saw2Sqr  3=Tri2Saw  4=Tri2Sqr  5=Sin2Sqr
Filter types: 6=LPF4 (Moog ladder, default)
LFO waveforms: 0=SINE  1=SAW1  4=TRI  5=SQUARE

Usage:
  python3 tools/add_classic_moog_presets.py [output.json]
  If output.json is omitted, merges into settings/moog_presets.json.
"""

import json
import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).parent
PROJECT_ROOT = SCRIPT_DIR.parent
DEFAULT_OUTPUT = PROJECT_ROOT / "settings" / "moog_presets.json"

LPF4 = 6   # Moog ladder filter


def preset(
    voice_mode=0,
    monophonic=True,
    fc_control=5000.0,
    q_control=2.0,
    attack=10.0,
    decay=300.0,
    sustain=0.7,
    release=200.0,
    detune_cents=0.0,
    pulse_width_pct=50.0,
    sub_osc_db=-82.0,
    noise_osc_db=-82.0,
    eg1_filter_enabled=False,
    eg1_filter_intensity=0.0,
    eg1_osc_enabled=False,
    eg1_osc_intensity=0.0,
    lfo1_rate=5.0,
    lfo1_amp=0.0,
    lfo1_waveform=0,
    lfo1_osc_pitch_enabled=False,
    lfo1_osc_pitch_intensity=0.0,
    lfo1_filter_fc_enabled=False,
    lfo1_filter_fc_intensity=0.0,
    lfo1_amp_enabled=False,
    lfo1_amp_intensity=0.0,
    lfo1_pulsewidth_enabled=False,
    lfo1_pulsewidth_intensity=0.0,
    portamento_time_msec=0.0,
    filter_type=LPF4,
    filter_saturation=0.0,
    filter_keytrack=False,
    filter_keytrack_intensity=0.0,
    octave=2.0,
    volume_db=0.5,
    pitchbend_range=2.0,
    legato_mode=False,
):
    return {
        "voice_mode": voice_mode,
        "monophonic": monophonic,
        "lfo1_waveform": float(lfo1_waveform),
        "lfo1_dest": 0.0,
        "lfo1_mode": 0.0,
        "lfo1_rate": lfo1_rate,
        "lfo1_amp": lfo1_amp,
        "lfo1_osc_pitch_intensity": lfo1_osc_pitch_intensity,
        "lfo1_osc_pitch_enabled": lfo1_osc_pitch_enabled,
        "lfo1_filter_fc_intensity": lfo1_filter_fc_intensity,
        "lfo1_filter_fc_enabled": lfo1_filter_fc_enabled,
        "lfo1_amp_intensity": lfo1_amp_intensity,
        "lfo1_amp_enabled": lfo1_amp_enabled,
        "lfo1_pan_intensity": 0.0,
        "lfo1_pan_enabled": False,
        "lfo1_pulsewidth_intensity": lfo1_pulsewidth_intensity,
        "lfo1_pulsewidth_enabled": lfo1_pulsewidth_enabled,
        "lfo2_waveform": 0.0,
        "lfo2_dest": 0.0,
        "lfo2_mode": 0.0,
        "lfo2_rate": 3.0,
        "lfo2_amp": 0.0,
        "lfo2_osc_pitch_intensity": 0.0,
        "lfo2_osc_pitch_enabled": False,
        "lfo2_filter_fc_intensity": 0.0,
        "lfo2_filter_fc_enabled": False,
        "lfo2_amp_intensity": 0.0,
        "lfo2_amp_enabled": False,
        "lfo2_pan_intensity": 0.0,
        "lfo2_pan_enabled": False,
        "lfo2_pulsewidth_intensity": 0.0,
        "lfo2_pulsewidth_enabled": False,
        "attack_time_msec": attack,
        "decay_time_msec": decay,
        "release_time_msec": release,
        "sustain_level": sustain,
        "volume_db": volume_db,
        "fc_control": fc_control,
        "q_control": q_control,
        "delay_time_msec": 0.0,
        "delay_feedback_pct": 0.0,
        "delay_ratio": 0.5,
        "delay_wet_mix": 0.0,
        "delay_mode": 0.0,
        "detune_cents": detune_cents,
        "pulse_width_pct": pulse_width_pct,
        "sub_osc_db": sub_osc_db,
        "noise_osc_db": noise_osc_db,
        "eg1_osc_intensity": eg1_osc_intensity,
        "eg1_osc_enabled": eg1_osc_enabled,
        "eg1_filter_intensity": eg1_filter_intensity,
        "eg1_filter_enabled": eg1_filter_enabled,
        "eg1_dca_intensity": 1.0,
        "eg1_dca_enabled": True,
        "filter_keytrack_intensity": filter_keytrack_intensity,
        "octave": octave,
        "pitchbend_range": pitchbend_range,
        "legato_mode": legato_mode,
        "reset_to_zero": False,
        "filter_keytrack": filter_keytrack,
        "filter_type": float(filter_type),
        "filter_saturation": filter_saturation,
        "nlp": False,
        "velocity_to_attack_scaling": False,
        "note_number_to_decay_scaling": False,
        "portamento_time_msec": portamento_time_msec,
        "sustain_override": False,
        "sustain_time_ms": 0.0,
        "sustain_time_sixteenth": 0.0,
    }


# ---------------------------------------------------------------------------
# 20 hand-crafted classic Minimoog-style patches
# ---------------------------------------------------------------------------
PATCHES = {

    # -------------------------------------------------------------------------
    # BASS PATCHES
    # -------------------------------------------------------------------------

    # Classic fat Moog bass — 3 saws, filter EG opens on attack then closes.
    # The archetypal bass heard on countless 70s/80s records.
    "Classic_Bass": preset(
        voice_mode=0,           # Saw3
        monophonic=True,
        fc_control=350.0,       # starts closed
        q_control=3.5,
        attack=5.0, decay=250.0, sustain=0.3, release=100.0,
        eg1_filter_enabled=True, eg1_filter_intensity=0.75,
        sub_osc_db=-12.0,       # sub octave adds weight
        detune_cents=5.0,
    ),

    # Deep sub bass — high sub-osc mix, very low cutoff, minimal resonance.
    "Deep_Bass": preset(
        voice_mode=0,           # Saw3
        monophonic=True,
        fc_control=280.0,
        q_control=2.0,
        attack=5.0, decay=400.0, sustain=0.4, release=150.0,
        eg1_filter_enabled=True, eg1_filter_intensity=0.55,
        sub_osc_db=-6.0,
    ),

    # Funky square-wave bass — high resonance, fast filter EG decay, zero sustain.
    # Envelope fully closes the filter: tight, punchy, percussive.
    "Funky_Bass": preset(
        voice_mode=1,           # Sqr3
        monophonic=True,
        fc_control=260.0,
        q_control=7.0,
        attack=3.0, decay=160.0, sustain=0.0, release=80.0,
        eg1_filter_enabled=True, eg1_filter_intensity=0.9,
        pulse_width_pct=50.0,
    ),

    # Narrow pulse bass — tight, cutting. Pulse width 25% gives a hollow,
    # slightly nasal character distinct from the 50% square.
    "Pulse_Bass": preset(
        voice_mode=1,           # Sqr3
        monophonic=True,
        fc_control=320.0,
        q_control=2.5,
        attack=5.0, decay=220.0, sustain=0.3, release=100.0,
        eg1_filter_enabled=True, eg1_filter_intensity=0.65,
        pulse_width_pct=25.0,
    ),

    # Sub bass — sine + square, very low cutoff, huge sub-osc presence.
    "Sub_Bass": preset(
        voice_mode=5,           # Sin2Sqr
        monophonic=True,
        fc_control=200.0,
        q_control=1.0,
        attack=5.0, decay=500.0, sustain=0.5, release=250.0,
        sub_osc_db=-3.0,
    ),

    # LFO filter wobble bass — moderate rate LFO sweeping the cutoff.
    "Wobble_Bass": preset(
        voice_mode=1,           # Sqr3
        monophonic=True,
        fc_control=600.0,
        q_control=5.0,
        attack=5.0, decay=200.0, sustain=0.65, release=100.0,
        lfo1_rate=4.0, lfo1_amp=0.8, lfo1_waveform=0,
        lfo1_filter_fc_enabled=True, lfo1_filter_fc_intensity=0.7,
        pulse_width_pct=50.0,
    ),

    # -------------------------------------------------------------------------
    # LEAD PATCHES
    # -------------------------------------------------------------------------

    # Classic Moog lead — 3 saws with mild detuning, mid-open filter.
    # The general-purpose Minimoog solo sound.
    "Classic_Lead": preset(
        voice_mode=0,           # Saw3
        monophonic=True,
        fc_control=4500.0,
        q_control=2.5,
        attack=8.0, decay=350.0, sustain=0.75, release=200.0,
        eg1_filter_enabled=True, eg1_filter_intensity=0.3,
        detune_cents=8.0,
    ),

    # Fat detuned lead — wider spread between oscillators gives a lush,
    # slightly chorused quality. Keith Emerson / prog rock territory.
    "Fat_Lead": preset(
        voice_mode=0,           # Saw3
        monophonic=True,
        fc_control=2500.0,
        q_control=2.5,
        attack=12.0, decay=500.0, sustain=0.75, release=250.0,
        eg1_filter_enabled=True, eg1_filter_intensity=0.35,
        detune_cents=12.0,
    ),

    # Vibrato lead — sine LFO on pitch. Classic live-performance Moog sound,
    # vibrato kicks in and adds expression on sustained notes.
    "Vib_Lead": preset(
        voice_mode=0,           # Saw3
        monophonic=True,
        fc_control=4000.0,
        q_control=2.0,
        attack=15.0, decay=400.0, sustain=0.85, release=300.0,
        eg1_filter_enabled=True, eg1_filter_intensity=0.25,
        lfo1_rate=5.5, lfo1_amp=0.6, lfo1_waveform=0,
        lfo1_osc_pitch_enabled=True, lfo1_osc_pitch_intensity=0.12,
    ),

    # Portamento lead — smooth pitch glide between notes (legato, mono).
    # Perfect for melodic runs and slide phrases.
    "Portamento_Lead": preset(
        voice_mode=0,           # Saw3
        monophonic=True,
        fc_control=4000.0,
        q_control=3.0,
        attack=20.0, decay=400.0, sustain=0.8, release=350.0,
        eg1_filter_enabled=True, eg1_filter_intensity=0.3,
        portamento_time_msec=180.0,
        legato_mode=True,
    ),

    # Sync-style lead — saw+square mix with higher resonance and open filter
    # gives a bright, harmonically dense tone.
    "Sync_Lead": preset(
        voice_mode=2,           # Saw2Sqr
        monophonic=True,
        fc_control=6000.0,
        q_control=5.5,
        attack=5.0, decay=280.0, sustain=0.65, release=200.0,
        eg1_filter_enabled=True, eg1_filter_intensity=0.45,
    ),

    # Acid lead — extreme resonance, fast filter EG, zero sustain.
    # Sounds biting and aggressive.
    "Acid_Lead": preset(
        voice_mode=2,           # Saw2Sqr
        monophonic=True,
        fc_control=450.0,
        q_control=8.5,
        attack=3.0, decay=130.0, sustain=0.0, release=100.0,
        eg1_filter_enabled=True, eg1_filter_intensity=0.95,
    ),

    # Dirty lead — filter saturation adds harmonic distortion.
    # Aggressive mid-range grind.
    "Dirty_Lead": preset(
        voice_mode=2,           # Saw2Sqr
        monophonic=True,
        fc_control=2000.0,
        q_control=6.5,
        attack=5.0, decay=200.0, sustain=0.6, release=150.0,
        eg1_filter_enabled=True, eg1_filter_intensity=0.6,
        filter_saturation=0.6,
        detune_cents=7.0,
    ),

    # ELP lead — Keith Emerson-inspired. Wide filter envelope, detuned saws,
    # mild vibrato. Dramatic and expressive.
    "ELP_Lead": preset(
        voice_mode=0,           # Saw3
        monophonic=True,
        fc_control=3000.0,
        q_control=4.5,
        attack=20.0, decay=600.0, sustain=0.65, release=400.0,
        eg1_filter_enabled=True, eg1_filter_intensity=0.5,
        detune_cents=10.0,
        lfo1_rate=4.5, lfo1_amp=0.4, lfo1_waveform=0,
        lfo1_osc_pitch_enabled=True, lfo1_osc_pitch_intensity=0.09,
    ),

    # -------------------------------------------------------------------------
    # BRASS / STAB
    # -------------------------------------------------------------------------

    # Brass stab — fast attack, medium decay, no sustain. Punchy horn stab.
    # High filter resonance + strong EG gives that nasal "blatt".
    "Brass_Stab": preset(
        voice_mode=0,           # Saw3
        monophonic=True,
        fc_control=600.0,
        q_control=4.5,
        attack=15.0, decay=180.0, sustain=0.45, release=80.0,
        eg1_filter_enabled=True, eg1_filter_intensity=0.85,
    ),

    # -------------------------------------------------------------------------
    # PAD / TEXTURE
    # -------------------------------------------------------------------------

    # Warm pad — slow attack, tri+saw blend, wide detuning for chorus-like
    # spread, very slow LFO vibrato.
    "Pad_Warm": preset(
        voice_mode=3,           # Tri2Saw
        monophonic=False,
        fc_control=2500.0,
        q_control=1.5,
        attack=700.0, decay=300.0, sustain=0.9, release=900.0,
        detune_cents=15.0,
        lfo1_rate=0.5, lfo1_amp=0.3, lfo1_waveform=0,
        lfo1_osc_pitch_enabled=True, lfo1_osc_pitch_intensity=0.08,
    ),

    # Moog strings — tri+saw, heavy detuning, slow attack, filter keytrack
    # so higher notes open the filter naturally.
    "Moog_Strings": preset(
        voice_mode=3,           # Tri2Saw
        monophonic=False,
        fc_control=5000.0,
        q_control=1.0,
        attack=350.0, decay=200.0, sustain=0.85, release=700.0,
        detune_cents=22.0,
        filter_keytrack=True, filter_keytrack_intensity=1.0,
        lfo1_rate=0.8, lfo1_amp=0.5, lfo1_waveform=0,
        lfo1_osc_pitch_enabled=True, lfo1_osc_pitch_intensity=0.06,
    ),

    # Classic LFO filter sweep — slow sine LFO sweeping the cutoff.
    # Good for atmospheric textures and transitions.
    "Filter_Sweep": preset(
        voice_mode=0,           # Saw3
        monophonic=False,
        fc_control=800.0,
        q_control=5.0,
        attack=50.0, decay=300.0, sustain=0.7, release=300.0,
        lfo1_rate=0.25, lfo1_amp=0.7, lfo1_waveform=0,
        lfo1_filter_fc_enabled=True, lfo1_filter_fc_intensity=0.8,
    ),

    # -------------------------------------------------------------------------
    # SHORT / PERCUSSIVE
    # -------------------------------------------------------------------------

    # Percussive pluck — instant attack, fast filter EG decay, zero sustain.
    # Saw+square blend gives a bright initial transient.
    "Perc_Pluck": preset(
        voice_mode=2,           # Saw2Sqr
        monophonic=False,
        fc_control=3500.0,
        q_control=3.5,
        attack=1.0, decay=200.0, sustain=0.0, release=180.0,
        eg1_filter_enabled=True, eg1_filter_intensity=0.75,
    ),

    # -------------------------------------------------------------------------
    # MELODIC / WIND
    # -------------------------------------------------------------------------

    # Moog flute — tri+square blend, high cutoff, low resonance, slight noise
    # for breathiness, slow LFO vibrato.
    "Moog_Flute": preset(
        voice_mode=4,           # Tri2Sqr
        monophonic=True,
        fc_control=9000.0,
        q_control=0.5,
        attack=80.0, decay=100.0, sustain=0.85, release=220.0,
        noise_osc_db=-20.0,
        lfo1_rate=5.5, lfo1_amp=0.5, lfo1_waveform=0,
        lfo1_osc_pitch_enabled=True, lfo1_osc_pitch_intensity=0.07,
    ),
}


def unique_name(name, existing):
    if name not in existing:
        return name
    n = 2
    while f"{name}_{n}" in existing:
        n += 1
    return f"{name}_{n}"


def main():
    out_path = Path(sys.argv[1]) if len(sys.argv) >= 2 else DEFAULT_OUTPUT

    existing = {}
    if out_path.exists():
        try:
            existing = json.loads(out_path.read_text())
        except Exception as e:
            print(f"  WARNING: could not read {out_path}: {e}")

    existing_names = set(existing.keys())
    added = 0
    for raw_name, p in PATCHES.items():
        name = unique_name(raw_name, existing_names)
        existing[name] = p
        existing_names.add(name)
        added += 1
        if name != raw_name:
            print(f"  Renamed duplicate '{raw_name}' → '{name}'")

    out_path.write_text(json.dumps(existing, indent=2) + "\n")
    print(f"  Wrote {added} presets to {out_path}  (total: {len(existing)})")


if __name__ == "__main__":
    main()
