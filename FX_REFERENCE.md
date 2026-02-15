# SoundB0ard FX Reference

Quick reference for all audio effects and their parameters.

## How to Use Effects

```javascript
let synth_name = minisynth();
add_fx(synth_name, "delay");

SB#> ps
synth_name = MiniSynth(default) vol:1 pan:0 voice:SQR3(1)
     fx0 Delay! ms:23 fb:0 rat:0 mx:0.5 mode:norm sync:0 sync_len:1/16

# set synth_name:fx<num>:value;  // Configure parameters
set synth_name:fx0:fb 70;
```

---

## Saturation & Distortion

### distort - Multi-Mode Distortion

```javascript
add_fx(inst, "distort");
set inst:fx0:mode 0-3;         // Algorithm selection
set inst:fx0:threshold 0.01-1.0;  // Clipping point
set inst:fx0:drive 1.0-10.0;   // Input gain
```

**Modes:**
- **0: HARD_CLIP** - Brick wall limiter (classic)
- **1: SOFT_CLIP** - Smooth tanh saturation
- **2: TUBE** - Asymmetric tube warmth
- **3: FOLDBACK** - Wavefold/foldback distortion

### waveshape - Arctan Waveshaper

```javascript
add_fx(inst, "waveshape");
set inst:fx0:k_pos 0.1-20;     // Positive curve shaping
set inst:fx0:k_neg 0.1-20;     // Negative curve shaping
set inst:fx0:stages 1-10;      // Number of stages
set inst:fx0:invert 0-1;       // Invert alternate stages
```

---

## Degradation / Lo-Fi

### lofi - LoFi Crusher

```javascript
add_fx(inst, "lofi");  // Also: bitcrush, decimate, lofi_crusher
set inst:fx0:bitdepth 1-16;           // Bit depth
set inst:fx0:sample_hold 0.0-1.0;  // Sample rate
set inst:fx0:destruct 0.0-1.0;        // Destruction amount
```

**Examples:**
- Vintage lo-fi: `bitdepth=12, sample_hold=0.9, destruct=0.0`
- Extreme crush: `bitdepth=3, sample_hold=0.2, destruct=0.8`

---

## Time-Based Effects

### delay - Stereo Delay

```javascript
add_fx(inst, "delay");
set inst:fx0:ms 1-10000; 
set inst:fx0:fb 0-100;
set inst:fx0:rat 0.1-0.9;
set inst:fx0:mx 0.0-1.0;      // Wet/dry mix
set inst:fx0:mode 0;
set inst:fx0:sync 1;
set inst:fx0:sync_len 1;
```

### moddelay - Modulated Delay

Chorus/flanger effects via modulated delay time.

```javascript
add_fx(inst, "moddelay");
set inst:moddelay_time 0-1000;         // Base delay (ms)
set inst:moddelay_feedback 0-99;       // Feedback %
set inst:moddelay_mod_depth 0-100;     // Modulation depth
set inst:moddelay_mod_rate 0.0-20.0;   // Modulation rate (Hz)
```

### reverb - Reverb

```javascript
add_fx(inst, "reverb");
set inst:reverb_roomsize 0.0-1.0;   // Room size
set inst:reverb_damping 0.0-1.0;    // HF damping
set inst:reverb_wet 0.0-1.0;        // Wet/dry mix
set inst:reverb_width 0.0-1.0;      // Stereo width
```

### transverb - Transverb

Advanced delay/reverb hybrid.

```javascript
add_fx(inst, "transverb");
set inst:transverb_time 0-2000;        // Delay time (ms)
set inst:transverb_feedback 0-99;      // Feedback %
set inst:transverb_diffusion 0.0-1.0;  // Diffusion amount
```

---

## Creative / Experimental

### sculptor - Waveform Sculptor

Landmark-based waveform manipulation for creative sound design.

```javascript
add_fx(inst, "sculptor");  // Also: geometer, waveform_sculptor
set inst:waveform_sculptor_window_size 256-4096;      // Window size
set inst:waveform_sculptor_landmark_style 0-4;        // Detection method
set inst:waveform_sculptor_interp_style 0-5;          // Recreation method
set inst:waveform_sculptor_wet_mix 0.0-1.0;           // Wet/dry
```

**Landmark Styles:**
- 0: ExtNCross (extremities & zero-crossings)
- 1: Span (amplitude-based)
- 2: DyDx (derivative/slope)
- 3: Freq (regular frequency)
- 4: Random

**Interpolation Styles:**
- 0: Polygon (straight lines, lo-fi)
- 1: Wrongygon (backwards, harsh)
- 2: Sing (sine waves, tonal)
- 3: Reversi (reverse intervals)
- 4: Smoothie (smooth curves)
- 5: Pulse (just pulses)

### diffuser - Diffuser
Multi-stage diffusion/blur/reverb hybrid.

```javascript
add_fx(inst, "diffuser");  // Also: nnirror
set inst:diffuser_wet 0.0-1.0;        // Wet/dry mix
set inst:diffuser_size 0.0-1.0;       // Size/length
set inst:diffuser_feedback 0.0-1.0;   // Feedback
set inst:diffuser_unison 0.0-1.0;     // Unison voices
set inst:diffuser_diffuse0 0.0-1.0;   // Diffusion stage 1
set inst:diffuser_diffuse1 0.0-1.0;   // Diffusion stage 2
set inst:diffuser_diffuse2 0.0-1.0;   // Diffusion stage 3
set inst:diffuser_diffuse3 0.0-1.0;   // Diffusion stage 4
```

---

## Dynamics

### compressor - Compressor/Limiter

```javascript
add_fx(inst, "compressor");
set inst:compressor_threshold -60-0;      // Threshold (dB)
set inst:compressor_ratio 1.0-20.0;       // Compression ratio
set inst:compressor_attack 0.1-500;       // Attack (ms)
set inst:compressor_release 10-5000;      // Release (ms)
set inst:compressor_knee_width 0-20;      // Knee width (dB)
```

---

## Filters

### filter - Basic Filter

```javascript
add_fx(inst, filter);
set inst:basicfilter_cutoff 20-20000;    // Cutoff (Hz)
set inst:basicfilter_resonance 0.0-10.0;  // Resonance/Q
```

### modfilter - Modulated Filter

Filter with LFO modulation.

```javascript
add_fx(inst, modfilter);
set inst:modfilter_cutoff 20-20000;       // Base cutoff (Hz)
set inst:modfilter_resonance 0.0-10.0;    // Resonance/Q
set inst:modfilter_mod_depth 0-5000;      // Modulation depth (Hz)
set inst:modfilter_mod_rate 0.0-20.0;     // Modulation rate (Hz)
```

---

## Effect Chains

Multiple effects are processed in the order they're added:

```javascript
let lead = fmsynth();
add_fx(lead, "distort");      // 1st: Distortion
add_fx(lead, "moddelay");     // 2nd: Modulated delay
add_fx(lead, "reverb");       // 3rd: Reverb

```

---

