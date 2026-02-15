# SBShell User Guide

Soundb0ard Shell is my Unix shell inspired music making environment. Rather than being the shell around an operating system, it is a shell around an audio mixing desk with several music instruments - drum machine, granular looper, FM and subtractive synths; and a javascript-like programming language called Slang, to control them.

You launch it from the command line and enter an interactive shell where you can type commands. The color scheme is designed for a dark terminal.

Once you have successfully followed the [BUILD](BUILD.md) instructions, you launch it from the projet root with:
```bash
build/Sbsh
```

---

## 0. Where am i?

You're goto command is `ps`. In Unix this would be 'process status', for SBShell, it's more like 'program status' - it shows you mixer stats like volume and BPM; it shows you environment variables, which can be standard objects like numbers, strings and booleans, and also its special sauce --  sound generator objects like FMSynth, MiniSynth, DrumSynth, or Sampler; and it shows the running Processes. More about all of that below..

## 1. Quick Start / First Sounds

Let's make some noise! Start SoundB0ard and type these commands at the `SB#>` prompt:

```javascript
// whats going on?
ps

// you'll see various synths already created in the environment.
// such as dx, dx2 and dx2, sbdrum and more.
```
 ![SoundB0ard ps command](images/sbshell_ps_full.png "ps command output")


```javascript
// Trigger a kick drum on a drum synth    (note 0)
note_on(sbdrum, 0);

// clap (note 2)
note_on(sbdrum, 2);

// what else?
info sbdrum;

// bd(0) // sd(1) // cp(2) // hh(3) // hh2(4) ..

// play a long bass note on an FM Synth (midi 20 - G#)
note_on(dx, 20, dur = 5000);
```


```javascript
// Load a preset
load_preset(sbdrum, "TR808");
note_on(sbdrum, 0);

// Try different velocities (0-127)
note_on(sbdrum, 1, vel = 127);  // Loud snare
note_on(sbdrum, 1, vel = 40);   // Quiet ghost note

```

---

## 2. Sound Generators

SoundB0ard has four main types of sound generators:

### Drum Machine - drum()
Synthesized drums with 9 voices: kick, snare, closed hat, clap, open hat, 3 FM toms, and a laser zap.

```javascript
let drums = drum();
load_preset(drums, "TR808");   // Classic 808
load_preset(drums, "TR909");   // Punchy 909
load_preset(drums, "DILLA");   // Warm, lo-fi

list_presets(drums); // to see all presets
save_preset(drums, "NEWPRESETNAME"); // to save a new preset

// Drum voices (MIDI note numbers):
// 0 = Kick
// 1 = Snare
// 2 = Clap
// 3 = Closed Hi-Hat
// 4 = Open Hi-Hat
// 5-7 = FM Toms
// 8 = Laser
```

### FM Synth - fmsynth()
6-operator FM synthesis for bells, bass, keys, and experimental sounds.

```javascript
let dx1 = fmsynth();
load_preset(dx1, "BASS");
note_on(dx1, 36);  // C1 - deep bass note
note_on(dx1, 60);  // C3 - middle C
note_on(dx1, notes_in_chord(40, 36)); // play a E(40) chord in the key of C(36)

midi_ref(); // midi note reference and other info
Midi Notes:
- C:0  C#:1  D:2  D#:3  E:4  F:5  F#:6  G:7  G#:8  A:9  A#:10 B:11
0 C:12 C#:13 D:14 D#:15 E:16 F:17 F#:18 G:19 G#:20 A:21 A#:22 B:23
1 C:24 C#:25 D:26 D#:27 E:28 F:29 F#:30 G:31 G#:32 A:33 A#:34 B:35
2 C:36 C#:37 D:38 D#:39 E:40 F:41 F#:42 G:43 G#:44 A:45 A#:46 B:47
3 C:48 C#:49 D:50 D#:51 E:52 F:53 F#:54 G:55 G#:56 A:57 A#:58 B:59
4 C:60 C#:61 D:62 D#:63 E:64 F:65 F#:66 G:67 G#:68 A:69 A#:70 B:71
5 C:72 C#:73 D:74 D#:75 E:76 F:77 F#:78 G:79 G#:80 A:81 A#:82 B:83
Chord Progressions: I-IV-V, I-V-vi-IV, I-vi-IV-V, vi-ii-V-I vi-IV-I-V
Chord Mods: None(0), Seventh(1), Seventh Inv(2) Root Inv(3) Power(4)
Key Mods: None(0), Natural Minor(1), Harmonic Minor Inv(2) Melodic Minor(3) Phrygian(4)
Filters: LPF1, HPF1, LPF2, HPF2, BPF2, BSF2, LPF4, HPF4, BPF4
Major Scale: W W H W W W H // Minor Scale: W H W W H W W

SB#> notes_in_key(36)
SB#> [36, 38, 40, 41,  43, 45, 47, 48]
print_notes(notes_in_key(36))
SB#> Notes:[36, 38, 40, 41,  43, 45, 47, 48]
C D E F G A B

```

### Subtractive Synth - minisynth()
Classic analog-style synthesis with oscillators and filters.

```javascript
let synth = minisynth();
load_preset(synth, "PAD");
note_on(synth, notes_in_chord(24, 24), vel = 100, dur = 2000);
```

### Granular Looper - loop()
Granular synthesis engine for texture and atmospheric sounds.

```javascript
let clavl = loop(perc/808clave.aif);
set clavl:len 8;
```

### More information
When you use `ps` you only see an overview of a sound generator. In order to view all parameters and their settings use `info(<sound_generator_name>)` e.g.
```javascript
info(dx);
```

![info dx command](images/infodx.png "info dx output")

```javascript
info(mo);
```
![info mo command](images/infomo.png "info mo output")

---

## 3. Basic Interaction

### Playing Notes

```javascript
// Basic syntax
note_on(<instrument_name>, <note_number>);

// e.g.
// With velocity (0-127, default 100)
note_on(dx, 20, vel = 80);

// With duration in milliseconds (default 100)
note_on(dx, 20, dur = 1000);

// With both
note_on(dx, 20, vel = 120, dur = 2000);
```

### Changing Parameters

Every sound generator has dozens of parameters you can tweak:

```javascript
let drums = drum();

// See all available parameters
info(drums);

// Set a parameter
set drums:bd_vol 1.0;        // Kick volume
set drums:bd_decay 200;      // Kick decay time
set drums:bd_pitch_env_range 12;  // Pitch sweep depth

```

### Working with Presets

```javascript
// List available presets
list_presets(<instrument name>);

// e.g.
list_presets(drums);

// Load a built-in preset
load_preset(drums, "TR909");

// Save your tweaked settings
save_preset(drums, "MY_KICKS");

// Load your custom preset
load_preset(drums, "MY_KICKS");

```

---

## 4. Timing & Sync

SoundB0ard runs on a global clock synced via Ableton Link.

### BPM and Tempo

```javascript
// Set tempo (also syncs with other Link-enabled apps)
bpm(120);

// See current BPM via `ps` output.
```

### Beat Divisions and pp

Within SBShell time is addressable in Midi ticks.
One loop, i.e. one bar, is 3840 midi ticks long. No matter what the BPM is, the midi clock will adjust to fill the space.
The most commonly addressed time division is a 16th, and 3840 / 16 = 240. This value is used so often I have it saved as a variable `pp` - 'pulses per'. This variable is set within the `startup.sb` file, which you can view and adjust yourself.

```javascript
// At 120 BPM:
// pp ≈ 125ms (one 16th note)
// pp * 2 = 8th note
// pp * 4 = quarter note
// pp * 16 = one bar

### Scheduling Notes with note_on_at()

// Schedule notes in musical time
note_on_at(drums, 0, 0);        // On the 1
note_on_at(drums, 1, pp * 4);   // Beat 2
note_on_at(drums, 0, pp * 8);   // Beat 3
note_on_at(drums, 1, pp * 12);  // Beat 4
```

While `note_on()` plays immediately, `note_on_at()` schedules notes at specific times:

```javascript
// Syntax: note_on_at(instrument, note, time_in_ms, vel, dur)
note_on_at(drums, 0, 0);           // Now
note_on_at(drums, 1, pp * 4);      // One beat later
note_on_at(drums, 2, pp * 2);      // Half beat later

// With swing (offset timing)
let swing = 15;  // midi_ticks
note_on_at(drums, 2, pp * 1 + swing);
note_on_at(drums, 2, pp * 3 - swing);
```

### Ableton Link

SoundB0ard automatically syncs with other apps via Ableton Link:

---

## 5. Samples

Load and play audio samples from the `wavs/` directory:

Add your own samples and directories here.

### List samples

```javascript
// list all directories within `wavs/`
ls

// list contents of specific directory
ls bd
```


### Loading Samples

```javascript
// preview a sound
play bd/mawkick.aiff;

// Load a sample using the relative path
let kick = sample(bd/kick8.aif);
let snare = sample(sd/2snare.aif);
let sh = sample(perc/chezShaker.aiff);

// Samples are organized in directories:
// bd/   - bass drums
// sd/   - snares
// cp/   - claps
// ch/   - closed hats
// oh/   - open hats
// perc/ - percussion
```

### Playing Samples

```javascript
// Trigger a sample
note_on(kick, 1);  // Note number ignored for samples

// Control playback
vol kick 0.8;
pan kick 0.2;   // Pan right (range -1.0 to 1.0)

// Pitch shifting
set kick:pitch 1.5;   // 1.5x speed (higher pitch)
set kick:pitch 0.5;   // 0.5x speed (lower pitch)
```

---

## 6. Patterns & Arrays

I've found arrays to be the most useful holder for patterns.

```javascript
// manually create an array of vals
let pat = [1, 2, 5, 0];

// access via idx
pat[0]
1

// length of array
len(pat);
4

// first value
head(pat);
1

// rest of array
tail(pat);
[2, 5, 0]

// last value
last(pat);
0

// create an empty array of 16 values
rand_array(16, 0, 0);
[0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0]

// create an range array of 16 values between 0 and 4 inclusive
rand_array(16, 0, 4);
[0, 4, 3, 2,  0, 4, 1, 4,  0, 4, 0, 3,  0, 1, 3, 0]

### Basic Pattern Arrays

```javascript
// 16-step kick pattern (1 = hit, 0 = rest)
let kicks = [1, 0, 0, 0,  0, 0, 1, 0,  0, 0, 0, 0,  1, 0, 0, 0];

// Classic 2 and 4 snare
let snares = [0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0];

// Hi-hat 8ths
let hats = [1, 0, 1, 0,  1, 0, 1, 0,  1, 0, 1, 0,  1, 0, 1, 0];
```

### Using Patterns in Loops

```javascript
let drums = drum();
let kicks = [1, 0, 0, 0,  0, 0, 1, 0,  0, 0, 0, 0,  1, 0, 0, 0];

for (let i = 0; i < 16; i++) {
    if (kicks[i] == 1) {
      print(kicks[i]);
      note_on_at(drums, 0, i * pp);
    }
}

```

### Velocity Patterns

```javascript
// Human-feeling velocity variations
let vels = [110, 95, 105, 100, 90, 108];
let vx = 0;

for (let i = 0; i < 16; i++) {
  if (kicks[i] == 1) {
    note_on_at(drums, 0, i * pp, vel = vels[vx]);
    vx = incr(vx, 0, len(vels));
  }
}
```

### Timing Offset Patterns (Swing)

```javascript

for (let i = 0; i < 16; i++) {
    let offs = 40;
    if (i % 2 == 0) {
        offs = 0;
    }
    note_on_at(drums, 3, i * pp + offs);
}
```

### Pattern Creation Functions

```javascript
// Euclidean rhythm generator
let pat = bjork(5, 16);  // 5 hits distributed over 16 steps
// Returns: [1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0]

// random kick pattern
rand_beat();

```

---

## 7. Computations (Live Coding)

Computations (`comp()`) allow you to live code rather than working directly in the repl, and allow you to create longer and more complex processes.

I find the best way to work is to have a split terminal screen, with an open SBShell repl on my left, and a text editor (vi) on my right.

 ![SoundB0ard split screen](images/splitscreen.png "splitscreen view")


### Basic Structure

The structure of a Computation is based on Processing and Arduino, wherein you have two functions, a setup() and a run(). Setup creates your environment and runs once, setting inital values, and run() is called once on every loop, i.e. once every 3840 midi ticks at the top of the bar.

Open a file in a text editor and create computations.

```javascript

$ vi SBTraxx/DEMO1.sb

let my_comp = comp()
{
  setup()
  {
    let pat = [1, 0, 1, 0,  1, 0, 1, 0];
    let pidx = 0;
  }
  run()
  {
    print("Pat value:", pat[pidx]);
    pidx = incr(pidx, 0, len(pat));
  }
}

```

Then within your SBShell window, monitor that file:

```javascript
SB#> monitor("SBTraxx/DEMO1.sb");

// now you can run it once:

SB#> my_comp()
Pat value:1
```

Or you can assign it to a process position and it will run continually until you reset the process.
```javascript

SB#> p1 # my_comp
SB#> Pat value:1
Pat value:0
Pat value:1
Pat value:0
```

### Simple Drum Pattern

```javascript
let drums = drum();
load_preset(drums, "TR808");

let kick_comp = comp()
{
  setup()
  {
    let kicks = [1, 0, 0, 0,  0, 0, 1, 0,  0, 0, 0, 0,  1, 0, 0, 0];
  }
  run()
  {
    for (let i = 0; i < 16; i++) {
      if (kicks[i] == 1) {
        note_on_at(drums, 0, i * pp, vel = 100);
      }
    }
  }
}

p2 # kick_comp

```

### Pattern Switching

```javascript
let snare_comp = comp()
{
  setup()
  {
    let s1 = [0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0];
    let s2 = [0, 0, 0, 0,  1, 0, 0, 1,  0, 0, 0, 0,  1, 0, 0, 0];
    let s3 = [0, 0, 0, 0,  1, 0, 1, 0,  0, 0, 0, 0,  1, 0, 1, 1];
  }
  run()
  {
    let spat = s1;

    # every 4th bar
    if (count % 4 == 3) {
      spat = s2;
    }
    if (count % 8 == 7) {
      spat = s3;
    }

    for (let i = 0; i < 16; i++) {
      if (spat[i] == 1) {
        note_on_at(drums, 1, i * pp, vel = 100 + rand(20));
      }
    }
  }
}
```

### The count Variable

There is a global variable called 'count' which is incremented for every bar. 
You can use this to make decisions over time, using a modulo operation, i.e. every 2nd bar would be `count % 2 == 0` or on the last bar of an 8 bar loop would be `count % 8 == 7`.

```javascript
run()
{
  print("This is bar:", count);

  // Do different things on different bars
  if (count % 8 == 0) {
    print("New 8-bar section!");
  }

  if (count == 16) {
    // drop the beat
  }
}
```

### Randomization and Evolution

```javascript
let evolving_comp = comp()
{
  setup()
  {
    let base_pattern = [1, 0, 0, 0,  1, 0, 0, 0];
    let variation = 0;
  }
  run()
  {
    // Every 4 bars, add more randomness
    if (count % 4 == 0) {
      variation = variation + 5;
    }

    for (let i = 0; i < 16; i++) {
      if (base_pattern[i % 8] == 1) {
        // Randomly offset timing
        let offset = rand(variation) - variation/2;
        note_on_at(drums, 0, i * pp + offset);
      }
    }
  }
}
```

---

## 8. Mixing & Routing

### Volume and Panning

```javascript
let drums = drum();
let bass = dxsynth();
let pad = minisynth();

// Set volumes (0.0 to 1.0+)
vol drums 0.9;
vol bass 0.7;
vol pad 0.4;

// Pan in stereo field (-1.0 = left, 0 = center, 1.0 = right)
pan drums 0.0;    // Center
pan bass -0.3;    // Slightly left
pan pad 0.4;      // Right
```

### Process IDs and Pattern Assignment

When you start computations, assign them to process IDs (p1-p99) to control them:

```javascript
let kick_comp = comp() { /* ... */ }
let snare_comp = comp() { /* ... */ }
let hats_comp = comp() { /* ... */ }

// Assign to processes
p31 # kick_comp;   // Start on process 31
p32 # snare_comp;  // Start on process 32
p33 # hats_comp;   // Start on process 33

// Stop a process
p32 # "";          // Stop snares

// Restart it
p32 # snare_comp;  // Bring snares back
```

### Master Arrangement Computation

```javascript
let main_comp = comp()
{
  setup()
  {
    let bar = 0;
  }
  run()
  {
    // Build up arrangement over time
    if (bar == 0) {
      p31 # kick_comp;
      p32 # snare_comp;
    }
    if (bar == 4) {
      p33 # hats_comp;
    }
    if (bar == 8) {
      p34 # bass_comp;
    }
    if (bar == 16) {
      // Drop everything except kick
      p32 # "";
      p33 # "";
      p34 # "";
    }
    if (bar == 20) {
      // Bring it all back
      p32 # snare_comp;
      p33 # hats_comp;
      p34 # bass_comp;
    }

    print("Section bar:", bar);
    bar++;
  }
}
```

---

## 9. Effects

There are a number of FX that can be added to each sound generator, and there are also 3 mixer level FX that can routed to.

### Adding Effects to Sound Generators

Use the `add_fx(<soundgenerator name>, "<fx_name>")` command to add effects to any instrument:

```javascript

add_fx(sbdrum, "distort");

add_fx(dx, "delay");
add_fx(dx, "reverb");

// view the effects under the soundgenerator when you view `ps`:

ps
dx = FmSynth(mo_jazz) vol:0.80 pan:0.00 algo:7
     fx0 Delay! ms:23 fb:0 rat:0 mx:0.5 mode:norm sync:0 sync_len:1/16
     fx1 Reverb! predelayms:40 reverbtime:100 wetmx:20

// you can address any parameter via the syntax <soundgenerator_name>:fx<num>:<param_name>
// e.g. 

set dx:fx0:ms 10;
set dx:fx1:predelayms 100;

// Multiple effects process in order
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

## 10. Control Flow & Programming

SoundB0ard has a full programming language built in.

### Variables

```javascript
// Declare with let
let x = 5;
let name = "kick";
let pattern = [1, 0, 1, 0];

// Variable scope:
// - Top-level: global, persists across commands
// - Inside setup(): persists for that computation
// - Inside run(): local to that bar
```

**Reserved words:** Don't use keywords (let, if, for, etc.) or process IDs (p1, p2, p31, etc.) as variable names!

### Conditionals

```javascript
if (count % 4 == 0) {
  print("New 4-bar section");
}

if (count < 8) {
  // Intro
} else if (count < 16) {
  // Verse
} else {
  // Chorus
}
```

### Loops

```javascript
// For loop
for (let i = 0; i < 16; i++) {
  note_on_at(drums, 2, i * pp);
}

```

### Useful Built-in Functions

```javascript
// Random
rand(10);           // Random 0-9
rand(20) - 10;      // Random -10 to 9

// Increment with wrapping
let x = 0;
x = incr(x, 0, 5);  // Increments 0->1->2->3->4->0->1...

// Random increment/decrement (drunk walk)
let offset = 0;
offset = rincr(offset, -40, 40);  // Randomly walk within range

// Array operations
len(array);         // Length
append(arr, val);   // Add element
min(a, b);          // Minimum
max(a, b);          // Maximum

// Euclidean rhythms
bjork(5, 16);       // 5 hits over 16 steps

// See all functions
funcz();
```

### Debugging

```javascript
// Print to console
print("Value:", x);
print("Bar:", count, "Pattern:", pattern);

// Check instrument state
info(drums);

```

---

## 11. File-Based Workflow

Move beyond the REPL and write reusable scripts.

### Writing .sb Files

Create files with `.sb` extension in the `SBTraxx/` directory:

**my_beat.sb:**
```javascript
# This is a comment

let drums = drum();
load_preset(drums, "TR808");

let kick_comp = comp()
{
  setup()
  {
    let kicks = [1, 0, 0, 0,  0, 0, 1, 0,  0, 0, 0, 0,  1, 0, 0, 0];
  }
  run()
  {
    for (let i = 0; i < 16; i++) {
      if (kicks[i] == 1) {
        note_on_at(drums, 0, i * pp, vel = 100);
      }
    }
  }
}

// Start it
p31 # kick_comp;
```


### Project Organization

```
SoundB0ard/
├── SBTraxx/              # Your tracks and patterns
│   ├── my_beat.sb
│   └── bass_lines.sb
├── settings/             # Presets
│   ├── drumpresets.dat
│   └── BEAT_STYLE_DRUM_KITS.txt
└── wavs/                 # Sample library
    ├── bd/
    ├── sd/
    └── vox/
```

### Startup Script

Edit `startup.sb` in the root directory - it runs automatically on launch:

```javascript
// startup.sb - auto-loads on startup
print("Welcome to my SoundB0ard setup!");
bpm(120);

// Load your default instruments
let drums = drum();
load_preset(drums, "TR808");
```

---

## 12. Advanced Topics

### Live Coding with Track Watching

Watch a file for changes and automatically reload it:

```javascript
// Start watching a file
monitor("SBTraxx/my_beat.sb");

// Now edit my_beat.sb in your editor
// Save the file - contents reloaded

```

### Modulation

Use LFOs and envelopes for movement:

```javascript
let synth = minisynth();

// LFO modulation
set synth:lfo_rate 2.0;    // Hz
set synth:lfo_depth 0.5;   // Amount

// Envelope
set synth:attack 10;       // ms
set synth:decay 200;
set synth:sustain 0.5;     // 0-1
set synth:release 500;
```

---

## 13. Reference

### Quick Command Cheat Sheet

```javascript
// Sound Generators
drum()                  // Drum machine
dxsynth()              // FM synth
minisynth()            // Subtractive synth
granular()             // Granular sampler
sample("path")           // Load audio sample

// Playback
note_on(inst, note, vel, dur)
note_on_at(inst, note, time, vel, dur)

// Parameters
set inst:param value
set inst:fx0:param value
info(inst)

// Presets
load_preset(inst, "NAME")
save_preset(inst, "NAME")
list_presets(inst)

// Mixing
vol inst value
pan inst value

// Timing
bpm(tempo)
pp                     // Pulses per 16th note

// Patterns
comp() { setup() {} run() {} }
p31 # comp_name
count                  // Current bar number

```

### Common Patterns

**Basic Four-on-Floor:**
```javascript
let kicks = [1, 0, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0];
```

**2 and 4 Snare:**
```javascript
let snares = [0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0];
```

**Eighth-Note Hats:**
```javascript
let hats = [1, 0, 1, 0,  1, 0, 1, 0,  1, 0, 1, 0,  1, 0, 1, 0];
```

**Euclidean Rhythm (5 hits in 16 steps):**
```javascript
let pattern = bjork(5, 16);
```

### Keyboard Shortcuts

```
Ctrl+D    - Exit SoundB0ard
Tab       - Auto-complete
↑/↓       - Command history
```

### Sample Library Organization

```
wavs/
├── bd/       - Bass drums (kick8.aif, wuk77.aiff, etc.)
├── sd/       - Snares (2snare.aif, etc.)
├── cp/       - Claps (clap17.aif, kNackr.aiff, etc.)
├── ch/       - Closed hi-hats (2hat.aif, etc.)
├── oh/       - Open hi-hats
├── perc/     - Percussion (uus.wav, chezShaker.aiff, etc.)
├── vox/      - Vocals and voice samples
└── noises/   - Sound effects (powerup.wav, etc.)
```

### Function Categories

Use `help()` to explore functions by category:

- **Arrays**: len, append, bjork
- **Math**: min, max, abs, pow, sqrt
- **Random**: rand, rincr
- **Control**: incr, dincr
- **Timing**: note_on, note_on_at, bpm
- **Sound**: drum, sample, dxsynth, minisynth
- **Effects**: set, getparam
- **File I/O**: run, track, save_preset, load_preset
- **Debug**: print, info

---

## Where to Go Next

- If you got this far, wow! Hit me up!
