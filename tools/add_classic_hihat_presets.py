#!/usr/bin/env python3
"""
Hand-crafted HH (closed) and HH2 (open) hi-hat presets for SoundB0ard.

Signal chain: 6 square oscillators → BPF (midf) → HPF (hif, hif_q) → EG/DCA

Key parameters:
  sqamp  : oscillator drive (0-1). Higher = louder/crunchier input to filters.
  attack : EG attack (ms). Short = more click transient. Default ~10-20ms.
  decay  : EG decay (ms). THE main differentiator:
             closed: 2-20ms   open: 60-450ms
  midf   : BPF center Hz. Higher = more high-frequency content in the mix.
             6000-8000 = dark/body    10000-14000 = bright/crisp
  hif    : HPF cutoff Hz. Higher = thinner/airier (less low-end body).
             4000-6000 = some body    10000-16000 = super thin/airy
  hif_q  : HPF resonance. Higher adds a metallic "ping" at the cutoff.
             1.0 = neutral    4-8 = noticeable sizzle    10+ = very resonant

Usage:
  python3 tools/add_classic_hihat_presets.py
"""

import json
from pathlib import Path

SCRIPT_DIR = Path(__file__).parent
PROJECT_ROOT = SCRIPT_DIR.parent
HH_FILE  = PROJECT_ROOT / "settings" / "drum_hh_presets.json"
HH2_FILE = PROJECT_ROOT / "settings" / "drum_oh_presets.json"


def hh(sqamp=0.5, attack=10.0, decay=10.0, midf=10000.0, hif=8000.0,
       hif_q=1.0, distortion_threshold=0.5, vol=0.85, pan=0.0,
       use_delay=False, delay_mode=0, delay_ms=23.0,
       delay_feedback_pct=0.0, delay_ratio=0.0,
       delay_wetmix=0.5, delay_sync_tempo=True, delay_sync_len=0):
    return {
        "vol": vol, "pan": pan, "sqamp": sqamp,
        "attack": attack, "decay": decay,
        "midf": midf, "hif": hif, "hif_q": hif_q,
        "distortion_threshold": distortion_threshold,
        "use_delay": use_delay, "delay_mode": float(delay_mode),
        "delay_ms": delay_ms, "delay_feedback_pct": delay_feedback_pct,
        "delay_ratio": delay_ratio, "delay_wetmix": delay_wetmix,
        "delay_sync_tempo": delay_sync_tempo,
        "delay_sync_len": float(delay_sync_len),
    }


# ---------------------------------------------------------------------------
# Closed hi-hats
# ---------------------------------------------------------------------------
HH_PATCHES = {

    # Ultra-tight click — almost pure transient, minimal decay.
    # Useful for very fast hi-hat patterns where decay muddies the groove.
    "CLICK_CHH": hh(
        sqamp=0.7, attack=3, decay=2,
        midf=15000, hif=16000, hif_q=1.0,
        distortion_threshold=0.42,
    ),

    # 909-style tight tick — the classic house/techno closed hat.
    # Very short decay, ultra-bright filters.
    "NINE09_CHH": hh(
        sqamp=0.65, attack=5, decay=4,
        midf=12000, hif=14000, hif_q=1.5,
        distortion_threshold=0.45,
    ),

    # Standard closed — versatile, sits well in most genres.
    # Balanced brightness, neutral Q.
    "STANDARD_CHH": hh(
        sqamp=0.52, attack=10, decay=10,
        midf=9000, hif=10000, hif_q=1.2,
        distortion_threshold=0.5,
    ),

    # Sizzle — longer decay with resonant HPF.
    # The ringing Q creates a metallic sizzle on each hit.
    "SIZZLE_CHH": hh(
        sqamp=0.5, attack=8, decay=20,
        midf=9500, hif=11000, hif_q=6.0,
        distortion_threshold=0.48,
    ),

    # Dark — lower filters give more body and less air.
    # Works well in hip-hop and lo-fi contexts.
    "DARK_CHH": hh(
        sqamp=0.5, attack=15, decay=14,
        midf=6000, hif=5000, hif_q=1.2,
        distortion_threshold=0.5,
    ),

    # Trashy — high drive, low filters, some saturation.
    # Raw and gritty, good for industrial or punk-influenced stuff.
    "TRASHY_CHH": hh(
        sqamp=0.8, attack=8, decay=12,
        midf=4500, hif=3500, hif_q=2.0,
        distortion_threshold=0.28,
    ),
}

# ---------------------------------------------------------------------------
# Open hi-hats
# ---------------------------------------------------------------------------
HH2_PATCHES = {

    # Short open — almost like a tight closed but with a little more ring.
    # Useful as an accent or where you want just a hint of sustain.
    "SHORT_OHH": hh(
        sqamp=0.6, attack=8, decay=65,
        midf=10000, hif=12000, hif_q=1.5,
        distortion_threshold=0.45,
    ),

    # 909-style open — crisp, medium-length, bright.
    # The go-to open hat for house and techno.
    "NINE09_OHH": hh(
        sqamp=0.62, attack=10, decay=160,
        midf=11000, hif=13000, hif_q=2.0,
        distortion_threshold=0.45,
    ),

    # Sizzle open — resonant HPF gives sustained metallic shimmer.
    # Sounds like the hat is still ringing as it decays.
    "SIZZLE_OHH": hh(
        sqamp=0.52, attack=12, decay=220,
        midf=9000, hif=11000, hif_q=7.0,
        distortion_threshold=0.47,
    ),

    # Dark open — lower filter stack, more body in the decay.
    # Jazz-influenced or hip-hop territory.
    "DARK_OHH": hh(
        sqamp=0.5, attack=20, decay=180,
        midf=6000, hif=5000, hif_q=1.2,
        distortion_threshold=0.5,
    ),

    # Washy crash — very long decay, sounds like a ride or crash cymbal.
    # Use sparingly for dramatic accents or ride-cymbal feels.
    "WASHY_OHH": hh(
        sqamp=0.48, attack=25, decay=420,
        midf=8000, hif=6500, hif_q=1.0,
        distortion_threshold=0.5,
    ),

    # Trashy open — driven oscillators, low filters, heavy saturation.
    # Dirty, aggressive — good for industrial or heavy electronic.
    "TRASHY_OHH": hh(
        sqamp=0.78, attack=10, decay=175,
        midf=5000, hif=4000, hif_q=2.0,
        distortion_threshold=0.26,
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
    print("Adding closed HH presets...")
    merge_presets(HH_FILE, HH_PATCHES)
    print("Adding open HH2 presets...")
    merge_presets(HH2_FILE, HH2_PATCHES)


if __name__ == "__main__":
    main()
