#include "help.h"

#include <sstream>

#include "defjams.h"

std::string build_help() {
  std::stringstream ss;
  // clang-format off
  ss << COOL_COLOR_GREEN << "\n=== SoundB0ard Help ===\n\n" << ANSI_COLOR_RESET;

  ss << COOL_COLOR_GREEN << "Documentation:\n";
  ss << ANSI_COLOR_WHITE  << "  USER_GUIDE.md    " << COOL_COLOR_ORANGE << "- Complete user guide with examples\n";
  ss << ANSI_COLOR_WHITE  << "  FX_REFERENCE.md  " << COOL_COLOR_ORANGE << "- Quick reference for all effects\n\n";

  ss << COOL_COLOR_GREEN << "Quick Start:\n";
  ss << ANSI_COLOR_WHITE  << "  let drums = drumsynth();              " << COOL_COLOR_ORANGE << "// Create drum machine\n";
  ss << ANSI_COLOR_WHITE  << "  load_preset(drums, \"TR808\");     " << COOL_COLOR_ORANGE << "// Load preset\n";
  ss << ANSI_COLOR_WHITE  << "  note_on(drums, 0);               " << COOL_COLOR_ORANGE << "// Trigger kick\n";
  ss << ANSI_COLOR_WHITE  << "  add_fx(drums, \"distort\");              " << COOL_COLOR_ORANGE << "// Add distortion\n";
  ss << ANSI_COLOR_WHITE  << "  set drums:fx0:mode 2;" << COOL_COLOR_ORANGE << "// Tube saturation\n\n";

  ss << COOL_COLOR_GREEN << "Sound Generators:\n";
  ss << ANSI_COLOR_WHITE  << "  drumsynth()       " << COOL_COLOR_ORANGE << "- Drum synthesizer (9 voices)\n";
  ss << ANSI_COLOR_WHITE  << "  subsynth()   " << COOL_COLOR_ORANGE << "- Subtractive synthesizer\n";
  ss << ANSI_COLOR_WHITE  << "  fmsynth()    " << COOL_COLOR_ORANGE << "- FM synthesizer (4 operators)\n";
  ss << ANSI_COLOR_WHITE  << "  wavsynth()   " << COOL_COLOR_ORANGE << "- Wavetable/sample synth (8-voice poly)\n";
  ss << ANSI_COLOR_WHITE  << "  loop(<sample_name>)   " << COOL_COLOR_ORANGE << "- Granular sampler\n";
  ss << ANSI_COLOR_WHITE  << "  sample(path) " << COOL_COLOR_ORANGE << "- Load audio sample\n\n";

  ss << COOL_COLOR_GREEN << "Effects:\n";
  ss << ANSI_COLOR_WHITE  << "  distort    " << COOL_COLOR_ORANGE << "- Multi-mode distortion (4 algorithms)\n";
  ss << ANSI_COLOR_WHITE  << "  lofi       " << COOL_COLOR_ORANGE << "- Lo-fi crusher (bit depth + sample rate)\n";
  ss << ANSI_COLOR_WHITE  << "  sculptor   " << COOL_COLOR_ORANGE << "- Waveform manipulation\n";
  ss << ANSI_COLOR_WHITE  << "  diffuser   " << COOL_COLOR_ORANGE << "- Multi-stage diffusion\n";
  ss << ANSI_COLOR_WHITE  << "  reverb     " << COOL_COLOR_ORANGE << "- Reverb\n";
  ss << ANSI_COLOR_WHITE  << "  delay      " << COOL_COLOR_ORANGE << "- Stereo delay\n";
  ss << ANSI_COLOR_WHITE  << "  moddelay   " << COOL_COLOR_ORANGE << "- Modulated delay (chorus/flanger)\n";
  ss << ANSI_COLOR_WHITE  << "  compressor " << COOL_COLOR_ORANGE << "- Dynamics compression\n";
  ss << ANSI_COLOR_WHITE  << "  djeq       " << COOL_COLOR_ORANGE << "- DJ-style 2-band EQ (lo/hi cut)\n\n";

  ss << COOL_COLOR_GREEN << "Commands:\n";
  ss << ANSI_COLOR_WHITE  << "  bpm <tempo>        " << COOL_COLOR_ORANGE << "- Set tempo\n";
  ss << ANSI_COLOR_WHITE  << "  monitor(\"file.sb\")    " << COOL_COLOR_ORANGE << "- Import and monitor script file\n";
  ss << ANSI_COLOR_WHITE  << "  info(<instrument_name>)  " << COOL_COLOR_ORANGE << "- Show all parameters\n";
  ss << ANSI_COLOR_WHITE  << "  list_presets(<instrument_name>)          " << COOL_COLOR_ORANGE << "- List available presets\n";
  ss << ANSI_COLOR_WHITE  << "  list_presets(drums, \"part\")              " << COOL_COLOR_ORANGE << "- List presets for one drum voice (bd/sd/hh/oh/cp)\n";
  ss << ANSI_COLOR_WHITE  << "  p10 # comp_name;  " << COOL_COLOR_ORANGE << "- Assign computation to process\n\n";

  ss << COOL_COLOR_GREEN
            << "========================================================================\n"
            << "                    Built-In Functions Reference\n"
            << "========================================================================\n\n";

  ss << COOL_COLOR_GREEN << "ARRAY/COLLECTION FUNCTIONS\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  len(arr|str|map)      " << COOL_COLOR_ORANGE << "- Returns the length/size\n";
  ss << ANSI_COLOR_WHITE << "  head(arr)             " << COOL_COLOR_ORANGE << "- Returns the first element\n";
  ss << ANSI_COLOR_WHITE << "  tail(arr)             " << COOL_COLOR_ORANGE << "- Returns all elements except the first\n";
  ss << ANSI_COLOR_WHITE << "  last(arr)             " << COOL_COLOR_ORANGE << "- Returns the last element\n";
  ss << ANSI_COLOR_WHITE << "  push(arr, item)       " << COOL_COLOR_ORANGE << "- Appends item to array\n";
  ss << ANSI_COLOR_WHITE << "  take_n(arr, n)        " << COOL_COLOR_ORANGE << "- Returns first n elements\n";
  ss << ANSI_COLOR_WHITE << "  take_random_n(arr, n) " << COOL_COLOR_ORANGE << "- Returns n random unique elements\n";
  ss << ANSI_COLOR_WHITE << "  reverse(arr)          " << COOL_COLOR_ORANGE << "- Reverses the order\n";
  ss << ANSI_COLOR_WHITE << "  rotate(arr, n)        " << COOL_COLOR_ORANGE << "- Rotates elements by n positions\n";
  ss << ANSI_COLOR_WHITE << "  sort(arr)             " << COOL_COLOR_ORANGE << "- Sorts in ascending order\n";
  ss << ANSI_COLOR_WHITE << "  shuffle(arr)          " << COOL_COLOR_ORANGE << "- Randomly shuffles elements\n";
  ss << ANSI_COLOR_WHITE << "  scramble(arr)         " << COOL_COLOR_ORANGE << "- Randomly scramble a 16-step rhythm (keeps active beats)\n";
  ss << ANSI_COLOR_WHITE << "  stutter(arr)          " << COOL_COLOR_ORANGE << "- Create stutter effect on a step pattern\n";
  ss << ANSI_COLOR_WHITE << "  is_array(val)         " << COOL_COLOR_ORANGE << "- Returns true if value is an array\n";
  ss << ANSI_COLOR_WHITE << "  is_in(arr, item)      " << COOL_COLOR_ORANGE << "- Returns true if item is found\n";
  ss << ANSI_COLOR_WHITE << "  keys(map)             " << COOL_COLOR_ORANGE << "- Returns array of all keys\n";
  ss << ANSI_COLOR_WHITE << "  invert(arr)           " << COOL_COLOR_ORANGE << "- Inverts 0s and 1s (for rhythms)\n";
  ss << ANSI_COLOR_WHITE << "  bits(num)             " << COOL_COLOR_ORANGE << "- Convert integer to array of bits\n";
  ss << ANSI_COLOR_WHITE << "  hex(num)              " << COOL_COLOR_ORANGE << "- Convert integer to hex string\n\n";

  ss << COOL_COLOR_GREEN << "MATH FUNCTIONS\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  floor(num)            " << COOL_COLOR_ORANGE << "- Returns largest integer <= num\n";
  ss << ANSI_COLOR_WHITE << "  abs(num)              " << COOL_COLOR_ORANGE << "- Returns absolute value\n";
  ss << ANSI_COLOR_WHITE << "  log(num)              " << COOL_COLOR_ORANGE << "- Returns log2 of num\n";
  ss << ANSI_COLOR_WHITE << "  sin(num), cos(num)    " << COOL_COLOR_ORANGE << "- Trig functions (radians)\n";
  ss << ANSI_COLOR_WHITE << "  max(a, b), min(a, b)  " << COOL_COLOR_ORANGE << "- Returns larger/smaller value\n";
  ss << ANSI_COLOR_WHITE << "  incr(num, min, max)   " << COOL_COLOR_ORANGE << "- Increments, wrapping at max\n";
  ss << ANSI_COLOR_WHITE << "  rincr(num, min, max)  " << COOL_COLOR_ORANGE << "- Decrements, wrapping at min\n";
  ss << ANSI_COLOR_WHITE << "  dincr(num, min, max)  " << COOL_COLOR_ORANGE << "- Drunk walk (+1 or -1 randomly)\n";
  ss << ANSI_COLOR_WHITE << "  scale(val, in_min, in_max, out_min, out_max) " << COOL_COLOR_ORANGE << "- Map range\n\n";

  ss << COOL_COLOR_GREEN << "RANDOM/GENERATIVE FUNCTIONS\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  rand(n)                    " << COOL_COLOR_ORANGE << "- Random integer 0 to n-1  (rand(arr) picks random element)\n";
  ss << ANSI_COLOR_WHITE << "  rand_array(size, min, max) " << COOL_COLOR_ORANGE << "- Array of random numbers\n";
  ss << ANSI_COLOR_WHITE << "  rand_sixteenthz(n)         " << COOL_COLOR_ORANGE << "- Array of n random unique 16th-note positions\n";
  ss << ANSI_COLOR_WHITE << "  perlin(x)                  " << COOL_COLOR_ORANGE << "- Perlin noise value (smooth random)\n";
  ss << ANSI_COLOR_WHITE << "  bjork(pulses, steps)       " << COOL_COLOR_ORANGE << "- Euclidean rhythm pattern\n";
  ss << ANSI_COLOR_WHITE << "  gen_perc(len, density)     " << COOL_COLOR_ORANGE << "- Random percussion pattern\n\n";

  ss << COOL_COLOR_GREEN << "MUSIC THEORY FUNCTIONS\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  midi_ref()                       " << COOL_COLOR_ORANGE << "- Map of note names to MIDI numbers\n";
  ss << ANSI_COLOR_WHITE << "  notes_in_key(root, scale)        " << COOL_COLOR_ORANGE << "- MIDI notes in key/scale\n";
  ss << ANSI_COLOR_WHITE << "  notes_in_chord(root, key, mod, scale) " << COOL_COLOR_ORANGE << "- MIDI notes in chord (key-aware, quality from scale degree)\n";
  ss << ANSI_COLOR_WHITE << "    mod:   0=triad 1=seventh 2=seventh_inv 3=root_inv 4=power 5=ninth\n";
  ss << ANSI_COLOR_WHITE << "    scale: 0=major 1=nat_minor 2=harm_minor 3=mel_minor 4=phrygian\n";
  ss << ANSI_COLOR_WHITE << "    e.g. notes_in_chord(48, 36, 5, 1) " << COOL_COLOR_ORANGE << "- minor 9th from C3 in A minor\n";
  ss << ANSI_COLOR_WHITE << "  chord_notes(root, type, mod)          " << COOL_COLOR_ORANGE << "- MIDI notes in chord (key-independent)\n";
  ss << ANSI_COLOR_WHITE << "    type: 0=major 1=minor 2=dim 3=power 4=sus2 5=sus4\n";
  ss << ANSI_COLOR_WHITE << "    mod:  0=triad 1=min7 2=maj7 3=inv_min7 4=inv_maj7\n";
  ss << ANSI_COLOR_WHITE << "  scale_note(note, key, scale)     " << COOL_COLOR_ORANGE << "- Quantize note to scale\n";
  ss << ANSI_COLOR_WHITE << "  scale_melody(notes, root, scale) " << COOL_COLOR_ORANGE << "- Quantize melody array to scale\n";
  ss << ANSI_COLOR_WHITE << "  algoz()                          " << COOL_COLOR_ORANGE << "- Ascii drawing of the DX algorithms\n";
  ss << ANSI_COLOR_WHITE << "  ratioz()                         " << COOL_COLOR_ORANGE << "- these were the preset ratios used on a DX100\n\n";

  ss << COOL_COLOR_GREEN << "SOUND GENERATOR CONTROL\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  note_on(gen, note, vel)              " << COOL_COLOR_ORANGE << "- Trigger note\n";
  ss << ANSI_COLOR_WHITE << "  note_on_at(gen, note, time, vel=, dur=) " << COOL_COLOR_ORANGE << "- Schedule note\n";
  ss << ANSI_COLOR_WHITE << "  note_off(gen, note)                  " << COOL_COLOR_ORANGE << "- Stop note\n";
  ss << ANSI_COLOR_WHITE << "  note_off_at(gen, note, time)         " << COOL_COLOR_ORANGE << "- Schedule note off\n";
  ss << ANSI_COLOR_WHITE << "  set_pitch(gen, pitch)                " << COOL_COLOR_ORANGE << "- Set generator pitch\n";
  ss << ANSI_COLOR_WHITE << "  stop(gen)                            " << COOL_COLOR_ORANGE << "- Stop all notes\n";
  ss << ANSI_COLOR_WHITE << "  solo(gen), unsolo()                  " << COOL_COLOR_ORANGE << "- Solo/unsolo generator\n";
  ss << ANSI_COLOR_WHITE << "  stepn(seq)                           " << COOL_COLOR_ORANGE << "- Advance step sequencer and return next value\n";
  ss << ANSI_COLOR_WHITE << "  signal_from(phasor)                  " << COOL_COLOR_ORANGE << "- Get current signal value from a phasor\n";
  ss << ANSI_COLOR_WHITE << "  change_steps(phasor, steps)          " << COOL_COLOR_ORANGE << "- Retune phasor cycle length (in MIDI ticks)\n";
  ss << ANSI_COLOR_WHITE << "  reset(phasor)                        " << COOL_COLOR_ORANGE << "- Reset phasor phase to 0\n\n";

  ss << COOL_COLOR_GREEN << "PLAYBACK FUNCTIONS\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  play_array(gen, notes)           " << COOL_COLOR_ORANGE << "- Play array of notes (1 per beat)\n";
  ss << ANSI_COLOR_WHITE << "  play_array_over(gen, notes, spd) " << COOL_COLOR_ORANGE << "- Play notes stretched over 'spd' beats\n";
  ss << ANSI_COLOR_WHITE << "  fast(gen, notes, speed)          " << COOL_COLOR_ORANGE << "- Play notes at given speed\n";
  ss << ANSI_COLOR_WHITE << "  play_rhythm(rhythm_map)          " << COOL_COLOR_ORANGE << "- Play a rhythm map {gen -> notes}\n";
  ss << ANSI_COLOR_WHITE << "  play_map(gen, map)               " << COOL_COLOR_ORANGE << "- Play a note map on generator\n\n";

  ss << COOL_COLOR_GREEN << "SAMPLE/INSTRUMENT LOADING\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  load_dir(dirname)    " << COOL_COLOR_ORANGE << "- Load all samples from wavs/dirname as map\n";
  ss << ANSI_COLOR_WHITE << "  add_buf(gen, path)   " << COOL_COLOR_ORANGE << "- Add audio buffer/sample to generator\n";
  ss << ANSI_COLOR_WHITE << "  drum_kit()           " << COOL_COLOR_ORANGE << "- Show default drum kit sample mappings\n";
  ss << ANSI_COLOR_WHITE << "  kit()                " << COOL_COLOR_ORANGE << "- Quick-start: create drums + FM synth setup\n\n";

  ss << COOL_COLOR_GREEN << "PATTERN FUNCTIONS\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  eval_pattern(pattern)  " << COOL_COLOR_ORANGE << "- Evaluate a pattern to an array\n";
  ss << ANSI_COLOR_WHITE << "  print_pattern(pattern) " << COOL_COLOR_ORANGE << "- Print pattern and return step sequence\n\n";

  ss << COOL_COLOR_GREEN << "STRING FUNCTIONS\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  lowercase(str) " << COOL_COLOR_ORANGE << "- Convert string to lowercase\n";
  ss << ANSI_COLOR_WHITE << "  uppercase(str) " << COOL_COLOR_ORANGE << "- Convert string to uppercase\n\n";

  ss << COOL_COLOR_GREEN << "PRESET MANAGEMENT\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  list_presets(gen)           " << COOL_COLOR_ORANGE << "- List available presets\n";
  ss << ANSI_COLOR_WHITE << "  list_presets(drums, \"part\") " << COOL_COLOR_ORANGE << "- List presets for one drum voice (bd/sd/hh/oh/cp/fm1/fm2/fm3/lz)\n";
  ss << ANSI_COLOR_WHITE << "  load_preset(gen, name)      " << COOL_COLOR_ORANGE << "- Load named preset\n";
  ss << ANSI_COLOR_WHITE << "  rand_preset(gen)            " << COOL_COLOR_ORANGE << "- Load random preset\n";
  ss << ANSI_COLOR_WHITE << "  save_preset(gen, name)      " << COOL_COLOR_ORANGE << "- Save current settings\n";
  ss << ANSI_COLOR_WHITE << "  save_drum_part(drum, preset, part) " << COOL_COLOR_ORANGE << "- Save one drum voice (bd/sd/hh/oh/cp/fm1/fm2/fm3/lz)\n";
  ss << ANSI_COLOR_WHITE << "  load_drum_part(drum, preset, part) " << COOL_COLOR_ORANGE << "- Load one drum voice from preset\n";
  ss << COOL_COLOR_GREEN << "  Kick drum advanced:\n";
  ss << ANSI_COLOR_WHITE << "  set d:bd_noise_en 1                  " << COOL_COLOR_ORANGE << "- Enable noise transient layer\n";
  ss << ANSI_COLOR_WHITE << "  set d:bd_pitch_env2_range 38         " << COOL_COLOR_ORANGE << "- Fast pitch spike depth (semitones, 0=off)\n";
  ss << ANSI_COLOR_WHITE << "  set d:bd_pitch_env2_attack 1         " << COOL_COLOR_ORANGE << "- Fast pitch spike attack (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set d:bd_pitch_env2_decay 8          " << COOL_COLOR_ORANGE << "- Fast pitch spike decay (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set d:bd_chirp_en 1                  " << COOL_COLOR_ORANGE << "- Enable chirp oscillator (exp freq sweep)\n";
  ss << ANSI_COLOR_WHITE << "  set d:bd_chirp_start 3000            " << COOL_COLOR_ORANGE << "- Chirp start frequency (Hz)\n";
  ss << ANSI_COLOR_WHITE << "  set d:bd_chirp_end 200               " << COOL_COLOR_ORANGE << "- Chirp end frequency (Hz)\n";
  ss << ANSI_COLOR_WHITE << "  set d:bd_chirp_decay 10              " << COOL_COLOR_ORANGE << "- Chirp sweep duration (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set d:bd_chirp_amp 0.316             " << COOL_COLOR_ORANGE << "- Chirp amplitude (linear, -10dB=0.316)\n";
  ss << ANSI_COLOR_WHITE << "  set d:bd_mod_en 1                    " << COOL_COLOR_ORANGE << "- Enable FM modulator oscillator\n";
  ss << ANSI_COLOR_WHITE << "  set d:bd_mod_freq 140                " << COOL_COLOR_ORANGE << "- FM modulator frequency (Hz)\n";
  ss << ANSI_COLOR_WHITE << "  set d:bd_mod_index 200               " << COOL_COLOR_ORANGE << "- FM depth — peak Hz swing (try 50-2000)\n";
  ss << ANSI_COLOR_WHITE << "  set d:bd_mod_decay 200               " << COOL_COLOR_ORANGE << "- FM envelope decay (ms)\n";
  ss << COOL_COLOR_GREEN << "  Snare drum advanced:\n";
  ss << ANSI_COLOR_WHITE << "  set d:sd_pitch_eg_depth 8            " << COOL_COLOR_ORANGE << "- Upward pitch sweep on attack (semitones, 0=off)\n";
  ss << ANSI_COLOR_WHITE << "  set d:sd_pitch_eg_decay 25           " << COOL_COLOR_ORANGE << "- Pitch sweep decay time (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set d:sd_hi_ratio 2.3                " << COOL_COLOR_ORANGE << "- Hi osc ratio vs lo osc (2.0=default, 2.3=SC-style)\n";
  ss << ANSI_COLOR_WHITE << "  set d:sd_psat_en 1                   " << COOL_COLOR_ORANGE << "- Enable parallel tanh saturation\n";
  ss << ANSI_COLOR_WHITE << "  set d:sd_psat_drive 4.47             " << COOL_COLOR_ORANGE << "- Saturation pre-gain (linear, 13dB=4.47)\n";
  ss << ANSI_COLOR_WHITE << "  set d:sd_psat_blend 0.316            " << COOL_COLOR_ORANGE << "- Saturation wet blend (linear, -10dB=0.316)\n";
  ss << COOL_COLOR_GREEN << "  Hi-Hat / Open Hat (hh/oh) — shared params, replace hh_ with oh_ for open hat:\n";
  ss << ANSI_COLOR_WHITE << "  set d:hh_attack 1                    " << COOL_COLOR_ORANGE << "- Attack (ms, default 1)\n";
  ss << ANSI_COLOR_WHITE << "  set d:hh_decay 10                    " << COOL_COLOR_ORANGE << "- Decay (ms — short=closed, long=open)\n";
  ss << ANSI_COLOR_WHITE << "  set d:hh_sqamp 0.5                   " << COOL_COLOR_ORANGE << "- Square oscillator bank amplitude\n";
  ss << ANSI_COLOR_WHITE << "  set d:hh_midf 8000                   " << COOL_COLOR_ORANGE << "- BPF centre frequency (Hz, shapes metallic body)\n";
  ss << ANSI_COLOR_WHITE << "  set d:hh_midf_q 3                    " << COOL_COLOR_ORANGE << "- BPF resonance (higher=more metallic sizzle, 1-10)\n";
  ss << ANSI_COLOR_WHITE << "  set d:hh_hif 7000                    " << COOL_COLOR_ORANGE << "- HPF cutoff (Hz, cuts low body)\n";
  ss << ANSI_COLOR_WHITE << "  set d:hh_hif_q 1                     " << COOL_COLOR_ORANGE << "- HPF resonance\n";
  ss << COOL_COLOR_GREEN << "  Hand Clap — four-voice layered clap (TR-808 style, cp):\n";
  ss << ANSI_COLOR_WHITE << "  set d:cp_v2_delay 12                 " << COOL_COLOR_ORANGE << "- Voice 2 trigger delay (ms, default 12)\n";
  ss << ANSI_COLOR_WHITE << "  set d:cp_v2_vol 0.7                  " << COOL_COLOR_ORANGE << "- Voice 2 volume (0-1, default 0.7)\n";
  ss << ANSI_COLOR_WHITE << "  set d:cp_v2_attack 15                " << COOL_COLOR_ORANGE << "- Voice 2 noise envelope attack (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set d:cp_v2_decay 150                " << COOL_COLOR_ORANGE << "- Voice 2 noise envelope decay (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set d:cp_v3_delay 20                 " << COOL_COLOR_ORANGE << "- Voice 3 trigger delay (ms, default 20)\n";
  ss << ANSI_COLOR_WHITE << "  set d:cp_v3_vol 0.6                  " << COOL_COLOR_ORANGE << "- Voice 3 volume (0-1, default 0.6)\n";
  ss << ANSI_COLOR_WHITE << "  set d:cp_v3_attack 10                " << COOL_COLOR_ORANGE << "- Voice 3 noise envelope attack (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set d:cp_v3_decay 200                " << COOL_COLOR_ORANGE << "- Voice 3 noise envelope decay (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set d:cp_v4_delay 30                 " << COOL_COLOR_ORANGE << "- Voice 4 trigger delay (ms, default 30)\n";
  ss << ANSI_COLOR_WHITE << "  set d:cp_v4_vol 0.5                  " << COOL_COLOR_ORANGE << "- Voice 4 volume (0-1, default 0.5)\n";
  ss << ANSI_COLOR_WHITE << "  set d:cp_v4_attack 10                " << COOL_COLOR_ORANGE << "- Voice 4 noise envelope attack (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set d:cp_v4_decay 400                " << COOL_COLOR_ORANGE << "- Voice 4 noise decay (ms, longer = reverb-like tail)\n";
  ss << COOL_COLOR_GREEN << "  Sidechain from individual drum voice:\n";
  ss << ANSI_COLOR_WHITE << "  add_fx(inst, \"sidechain\", drums, \"bd\") " << COOL_COLOR_ORANGE << "- Sidechain from kick only (bd/sd/cp/hh/oh/fm1-3/lz)\n\n";

  ss << COOL_COLOR_GREEN << "WAVSYNTH — WAVETABLE / SAMPLE SYNTH\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  let s = wavsynth()            " << COOL_COLOR_ORANGE << "- Create synth (8-voice polyphonic)\n";
  ss << ANSI_COLOR_WHITE << "  add_buf(s, \"waves/sine.wav\") " << COOL_COLOR_ORANGE << "- Load waveform or sample into buffer\n\n";
  ss << COOL_COLOR_GREEN << "  Modes:\n";
  ss << ANSI_COLOR_WHITE << "  set s:mode 0  " << COOL_COLOR_ORANGE << "- Wavetable: cycles through buffer at note frequency (default)\n";
  ss << ANSI_COLOR_WHITE << "  set s:mode 1  " << COOL_COLOR_ORANGE << "- Sample: plays buffer at pitches relative to root note\n\n";
  ss << COOL_COLOR_GREEN << "  ADSR:\n";
  ss << ANSI_COLOR_WHITE << "  set s:attack  10   " << COOL_COLOR_ORANGE << "- Attack  (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set s:decay   200  " << COOL_COLOR_ORANGE << "- Decay   (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set s:sustain 0.7  " << COOL_COLOR_ORANGE << "- Sustain (0.0-1.0)\n";
  ss << ANSI_COLOR_WHITE << "  set s:release 300  " << COOL_COLOR_ORANGE << "- Release (ms)\n\n";
  ss << COOL_COLOR_GREEN << "  Filter (Moog ladder):\n";
  ss << ANSI_COLOR_WHITE << "  set s:cutoff 4000  " << COOL_COLOR_ORANGE << "- Filter cutoff (Hz, default 18000 = open)\n";
  ss << ANSI_COLOR_WHITE << "  set s:q      3.0   " << COOL_COLOR_ORANGE << "- Resonance (1.0-10.0, default 1.0)\n\n";
  ss << COOL_COLOR_GREEN << "  Sample mode:\n";
  ss << ANSI_COLOR_WHITE << "  set s:root 60  " << COOL_COLOR_ORANGE << "- MIDI note that plays at 1x speed (default 60 = C4)\n";
  ss << ANSI_COLOR_WHITE << "  set s:loop 0   " << COOL_COLOR_ORANGE << "- 0=one-shot, 1=loop (default: 1 for wavetable, 0 for sample)\n\n";
  ss << COOL_COLOR_GREEN << "  Wavetable morphing (multiple buffers):\n";
  ss << ANSI_COLOR_WHITE << "  add_buf(s, \"waves/saw.wav\")   " << COOL_COLOR_ORANGE << "- Add a second buffer\n";
  ss << ANSI_COLOR_WHITE << "  set s:morph 0.5              " << COOL_COLOR_ORANGE << "- Crossfade between buffers (0.0=first, 1.0=last)\n\n";
  ss << COOL_COLOR_GREEN << "  Example:\n";
  ss << COOL_COLOR_ORANGE << "    let s = wavsynth();\n";
  ss << COOL_COLOR_ORANGE << "    add_buf(s, \"perc/vocal.wav\");    " << ANSI_COLOR_WHITE << "// sample mode\n";
  ss << COOL_COLOR_ORANGE << "    set s:mode 1; set s:root 60;\n";
  ss << COOL_COLOR_ORANGE << "    note_on(s, 60); note_on(s, 64); note_on(s, 67);  " << ANSI_COLOR_WHITE << "// chord\n\n";

  ss << COOL_COLOR_GREEN << "PHASOR — CYCLIC RAMP SIGNAL\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  let p = phasor(steps)    " << COOL_COLOR_ORANGE << "- Create phasor cycling 0.0→1.0 over 'steps' MIDI ticks\n";
  ss << ANSI_COLOR_WHITE << "                           " << COOL_COLOR_ORANGE << "  (e.g. phasor(3840) = one bar at default ppq)\n";
  ss << ANSI_COLOR_WHITE << "  signal_from(p)           " << COOL_COLOR_ORANGE << "- Read current phase value (0.0–1.0)\n";
  ss << ANSI_COLOR_WHITE << "  change_steps(p, steps)   " << COOL_COLOR_ORANGE << "- Retune phasor cycle length live\n";
  ss << ANSI_COLOR_WHITE << "  reset(p)                 " << COOL_COLOR_ORANGE << "- Reset phase to 0\n\n";
  ss << COOL_COLOR_GREEN << "  Factory functions (user-defined, see phazor.sb):\n";
  ss << ANSI_COLOR_WHITE << "  ramp_div_factory()       " << COOL_COLOR_ORANGE << "- Returns fn(sig, ratio): sub-ramp at fractional speed\n";
  ss << ANSI_COLOR_WHITE << "  ramp2slope_factory()     " << COOL_COLOR_ORANGE << "- Returns fn(sig): instantaneous slope (derivative)\n";
  ss << ANSI_COLOR_WHITE << "  ramp2trigger_factory()   " << COOL_COLOR_ORANGE << "- Returns fn(sig): true on wrap-around (cycle complete)\n\n";
  ss << COOL_COLOR_GREEN << "  Example — automate morph from phasor:\n";
  ss << COOL_COLOR_ORANGE << "    comp my_synth {\n";
  ss << COOL_COLOR_ORANGE << "      setup() {\n";
  ss << COOL_COLOR_ORANGE << "        let ph = phasor(3840);           " << ANSI_COLOR_WHITE << "// one bar\n";
  ss << COOL_COLOR_ORANGE << "        let trig = ramp2trigger_factory();\n";
  ss << COOL_COLOR_ORANGE << "      }\n";
  ss << COOL_COLOR_ORANGE << "      run() {\n";
  ss << COOL_COLOR_ORANGE << "        for i in range(0, pplooplen) {\n";
  ss << COOL_COLOR_ORANGE << "          let sig = signal_from(ph);\n";
  ss << COOL_COLOR_ORANGE << "          set sbs:morph sig at=i;\n";
  ss << COOL_COLOR_ORANGE << "          if (trig(sig)) { note_on_at(sbs, 60, i); }\n";
  ss << COOL_COLOR_ORANGE << "        }\n";
  ss << COOL_COLOR_ORANGE << "      }\n";
  ss << COOL_COLOR_ORANGE << "    }\n\n";

  ss << COOL_COLOR_GREEN << "GRANULAR LOOPER\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  let x = loop(file)       " << COOL_COLOR_ORANGE << "- Create granular looper from sample\n";
  ss << ANSI_COLOR_WHITE << "  set x:len 2              " << COOL_COLOR_ORANGE << "- Loop length in bars (default 1)\n";
  ss << ANSI_COLOR_WHITE << "  set x:speed 0.5          " << COOL_COLOR_ORANGE << "- Playback speed multiplier\n";
  ss << ANSI_COLOR_WHITE << "  set x:pitch 0.5          " << COOL_COLOR_ORANGE << "- Pitch ratio\n";
  ss << ANSI_COLOR_WHITE << "  set x:mode 0             " << COOL_COLOR_ORANGE << "- Loop mode: 0=loop 1=static 2=smudge\n";
  ss << ANSI_COLOR_WHITE << "  set x:reverse 1          " << COOL_COLOR_ORANGE << "- Reverse playback\n\n";
  ss << COOL_COLOR_GREEN << "  Rhythmic FX (trigger on next bar):\n";
  ss << ANSI_COLOR_WHITE << "  set x:scramble 1         " << COOL_COLOR_ORANGE << "- Randomly reorder 16 slices (keeps active beats)\n";
  ss << ANSI_COLOR_WHITE << "  set x:stutter 1          " << COOL_COLOR_ORANGE << "- Random stutter repeats\n";
  ss << ANSI_COLOR_WHITE << "  set x:strobe 1           " << COOL_COLOR_ORANGE << "- Alternate between anchor slice and sequence\n";
  ss << ANSI_COLOR_WHITE << "  set x:gate 1             " << COOL_COLOR_ORANGE << "- Rhythmic gating with envelope\n";
  ss << ANSI_COLOR_WHITE << "  set x:speedulate 1       " << COOL_COLOR_ORANGE << "- Half/double speed slice remapping\n";
  ss << ANSI_COLOR_WHITE << "  set x:slowdown 1         " << COOL_COLOR_ORANGE << "- Decelerate into stutter at end of bar\n";
  ss << ANSI_COLOR_WHITE << "  set x:repeat 1           " << COOL_COLOR_ORANGE << "- Beat repeat: lock and loop a section\n\n";
  ss << COOL_COLOR_GREEN << "  Pitch FX (trigger on next bar):\n";
  ss << ANSI_COLOR_WHITE << "  set x:pitch_ramp 1       " << COOL_COLOR_ORANGE << "- Pitch ramps up/down across the bar\n";
  ss << ANSI_COLOR_WHITE << "  set x:octave_jump 1      " << COOL_COLOR_ORANGE << "- Random octave shifts per slice\n";
  ss << ANSI_COLOR_WHITE << "  set x:pitch_staircase 1  " << COOL_COLOR_ORANGE << "- Semitone steps across the bar\n\n";
  ss << COOL_COLOR_GREEN << "  Granular controls:\n";
  ss << ANSI_COLOR_WHITE << "  set x:grains_per_sec 15  " << COOL_COLOR_ORANGE << "- Grain density\n";
  ss << ANSI_COLOR_WHITE << "  set x:grain_dur_ms 80    " << COOL_COLOR_ORANGE << "- Grain duration in ms\n";
  ss << ANSI_COLOR_WHITE << "  set x:grain_overlap 0.2  " << COOL_COLOR_ORANGE << "- Overlap between grains (0.0-0.9, default 0.2)\n";
  ss << ANSI_COLOR_WHITE << "  set x:grain_env 0        " << COOL_COLOR_ORANGE << "- Envelope: 0=Tukey (loop-safe), 1=Hann (granular cloud)\n";
  ss << ANSI_COLOR_WHITE << "  set x:grain_spray_ms 10  " << COOL_COLOR_ORANGE << "- Random position spray in ms\n";
  ss << ANSI_COLOR_WHITE << "  set x:quasi_grain_fudge 0 " << COOL_COLOR_ORANGE << "- Duration randomisation\n\n";
  ss << COOL_COLOR_GREEN << "  Multi-buffer:\n";
  ss << ANSI_COLOR_WHITE << "  add_buf(x, \"file\")           " << COOL_COLOR_ORANGE << "- Add source buffer (buf[0] is primary)\n";
  ss << ANSI_COLOR_WHITE << "  set x:primary 1             " << COOL_COLOR_ORANGE << "- Swap buf[1] to primary (drives timing/FX)\n";
  ss << ANSI_COLOR_WHITE << "  set x:buf[N]:pitch 1.5      " << COOL_COLOR_ORANGE << "- Per-buffer pitch ratio\n";
  ss << ANSI_COLOR_WHITE << "  set x:buf[N]:speed 0.5      " << COOL_COLOR_ORANGE << "- Per-buffer playback speed multiplier\n";
  ss << ANSI_COLOR_WHITE << "  set x:buf[N]:gain 0.8       " << COOL_COLOR_ORANGE << "- Per-buffer amplitude (0.0+)\n";
  ss << ANSI_COLOR_WHITE << "  set x:buf[N]:distortion 0.5 " << COOL_COLOR_ORANGE << "- Per-buffer soft-clip distortion (0.0-1.0)\n";
  ss << ANSI_COLOR_WHITE << "  set x:buf[N]:len 2          " << COOL_COLOR_ORANGE << "- Per-buffer loop length in bars\n";
  ss << ANSI_COLOR_WHITE << "  set x:buf[N]:poffset 4      " << COOL_COLOR_ORANGE << "- Per-buffer pattern offset (0-15)\n";
  ss << ANSI_COLOR_WHITE << "  set x:buf[N]:plooplen 8     " << COOL_COLOR_ORANGE << "- Per-buffer pattern loop length (1-16)\n";
  ss << ANSI_COLOR_WHITE << "  set x:buf[N]:pinc 2         " << COOL_COLOR_ORANGE << "- Per-buffer pattern step increment\n\n";
  ss << COOL_COLOR_GREEN << "  shhh mode (per-grain loudness-based source selection):\n";
  ss << ANSI_COLOR_WHITE << "  set x:shhh 1             " << COOL_COLOR_ORANGE << "- 1=quietest 2=loudest 0=off\n";
  ss << ANSI_COLOR_WHITE << "  set x:shhh_window_ms 80  " << COOL_COLOR_ORANGE << "- RMS window in ms (0=grain_dur)\n\n";
  ss << COOL_COLOR_GREEN << "  Buffer xfader (constant-power, independent of shhh):\n";
  ss << ANSI_COLOR_WHITE << "  set x:xfl 0              " << COOL_COLOR_ORANGE << "- Assign buf index to xfader Left side\n";
  ss << ANSI_COLOR_WHITE << "  set x:xfr 1              " << COOL_COLOR_ORANGE << "- Assign buf index to xfader Right side\n";
  ss << ANSI_COLOR_WHITE << "  set x:xfpos -1           " << COOL_COLOR_ORANGE << "- Crossfader position (-1=L, 0=centre, 1=R)\n";
  ss << ANSI_COLOR_WHITE << "  set x:xfspeed 0.002      " << COOL_COLOR_ORANGE << "- Ramp rate (position units per sample)\n";
  ss << ANSI_COLOR_WHITE << "  set x:xfclear 1          " << COOL_COLOR_ORANGE << "- Clear xfader assignments, reset to centre\n\n";

  ss << COOL_COLOR_GREEN << "MIXER/ROUTING FUNCTIONS\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  send(effect, gen)    " << COOL_COLOR_ORANGE << "- Send generator to effect\n";
  ss << ANSI_COLOR_WHITE << "  add_fx(gen, fx_name) " << COOL_COLOR_ORANGE << "- Add effect to generator\n";
  ss << ANSI_COLOR_WHITE << "    granulate / gran    " << COOL_COLOR_ORANGE << "- Live granular FX (routes audio through grain engine)\n";
  ss << ANSI_COLOR_WHITE << "      set gen:fx0:wet 0.5          " << COOL_COLOR_ORANGE << "- Dry/wet mix\n";
  ss << ANSI_COLOR_WHITE << "      set gen:fx0:grain_overlap 0.5 " << COOL_COLOR_ORANGE << "- Grain overlap (0.0-0.9)\n";
  ss << ANSI_COLOR_WHITE << "      set gen:fx0:grain_env 1       " << COOL_COLOR_ORANGE << "- Envelope: 0=Tukey 1=Hann\n";
  ss << ANSI_COLOR_WHITE << "      set gen:fx0:grains_per_sec 15 " << COOL_COLOR_ORANGE << "- Grain density\n";
  ss << ANSI_COLOR_WHITE << "      set gen:fx0:grain_dur_ms 80   " << COOL_COLOR_ORANGE << "- Grain duration in ms\n";
  ss << ANSI_COLOR_WHITE << "      set gen:fx0:grain_spray_ms 10 " << COOL_COLOR_ORANGE << "- Position randomisation\n";
  ss << ANSI_COLOR_WHITE << "  mvol(volume)         " << COOL_COLOR_ORANGE << "- Set master volume (0.0-1.0)\n";
  ss << ANSI_COLOR_WHITE << "  monitor(filepath)    " << COOL_COLOR_ORANGE << "- Monitor file for live coding\n\n";

  ss << COOL_COLOR_GREEN << "CROSSFADER FUNCTIONS\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  xassign(ch, gen)   " << COOL_COLOR_ORANGE << "- Assign generator to xfader channel (0=L, 1=R)\n";
  ss << ANSI_COLOR_WHITE << "  xremove(ch, gen)   " << COOL_COLOR_ORANGE << "- Remove generator from xfader channel\n";
  ss << ANSI_COLOR_WHITE << "  xclear()           " << COOL_COLOR_ORANGE << "- Clear all xfader assignments\n";
  ss << ANSI_COLOR_WHITE << "  xfade(direction)   " << COOL_COLOR_ORANGE << "- Crossfade toward LEFT(0) or RIGHT(1)\n\n";

  ss << COOL_COLOR_GREEN << "SCHEDULING\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  sched(delay, start, end, time, action) " << COOL_COLOR_ORANGE << "- Parameter automation\n\n";

  ss << COOL_COLOR_GREEN << "MIDI FUNCTIONS\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  midi_init()           " << COOL_COLOR_ORANGE << "- Initialize MIDI system\n";
  ss << ANSI_COLOR_WHITE << "  midi_assign(gen)      " << COOL_COLOR_ORANGE << "- Assign generator to MIDI controller\n";
  ss << ANSI_COLOR_WHITE << "  midi_map(id, param)   " << COOL_COLOR_ORANGE << "- Map MIDI CC to a parameter\n";
  ss << ANSI_COLOR_WHITE << "  midi_rec()            " << COOL_COLOR_ORANGE << "- Toggle MIDI recording on/off\n";
  ss << ANSI_COLOR_WHITE << "  midi_print()          " << COOL_COLOR_ORANGE << "- Toggle MIDI debug printing\n";
  ss << ANSI_COLOR_WHITE << "  midi_reset()          " << COOL_COLOR_ORANGE << "- Clear MIDI recording buffer\n";
  ss << ANSI_COLOR_WHITE << "  midi_dump()           " << COOL_COLOR_ORANGE << "- Dump recording buffer to MidiArray\n";
  ss << ANSI_COLOR_WHITE << "  midi2array(midi)      " << COOL_COLOR_ORANGE << "- Convert MidiArray to 16-step note array\n";
  ss << ANSI_COLOR_WHITE << "  midi_at(midi, n)      " << COOL_COLOR_ORANGE << "- Get notes at beat n from MidiArray\n";
  ss << ANSI_COLOR_WHITE << "  midi_fix(midi)        " << COOL_COLOR_ORANGE << "- Quantize MidiArray timing to 8th-note grid\n\n";

  ss << COOL_COLOR_GREEN << "FILTERS / EQ\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  add_fx(inst, \"djeq\")           " << COOL_COLOR_ORANGE << "- Add DJ-style 2-band EQ\n";
  ss << ANSI_COLOR_WHITE << "  set inst:fx0:lo <hz>           " << COOL_COLOR_ORANGE << "- HPF cutoff (bass cut). Default 80hz (open). Raise to cut bass e.g. 500\n";
  ss << ANSI_COLOR_WHITE << "  set inst:fx0:hi <hz>           " << COOL_COLOR_ORANGE << "- LPF cutoff (treble cut). Default 18000hz (open). Lower to cut treble e.g. 4000\n\n";

  ss << COOL_COLOR_GREEN << "VISUALIZATION\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  draw_bar(val, width, label, at=, row=)   " << COOL_COLOR_ORANGE << "- Render a live bar graph (val: 0.0-1.0)\n";
  ss << ANSI_COLOR_WHITE << "  draw_plot(val, width, label, at=, row=)  " << COOL_COLOR_ORANGE << "- Render a rolling sparkline of a signal\n";
  ss << ANSI_COLOR_WHITE << "    at=<ticks>    " << COOL_COLOR_ORANGE << "- Schedule display at a future tick (e.g. at=i inside a for loop)\n";
  ss << ANSI_COLOR_WHITE << "    row=<n>       " << COOL_COLOR_ORANGE << "- Pin to screen row n (for multi-row display without scrolling)\n\n";
  ss << ANSI_COLOR_WHITE << "  Examples:\n";
  ss << COOL_COLOR_ORANGE << "    draw_bar(vol, 40, \"vol\")           " << ANSI_COLOR_WHITE << "// Immediate bar at current line\n";
  ss << COOL_COLOR_ORANGE << "    draw_plot(sig, 80, \"lfo\", at=i)    " << ANSI_COLOR_WHITE << "// Scheduled sparkline inside a for loop\n";
  ss << COOL_COLOR_ORANGE << "    draw_bar(v, 40, \"A\", row=1)        " << ANSI_COLOR_WHITE << "// Pinned to row 1 above cursor\n";
  ss << COOL_COLOR_ORANGE << "    draw_plot(v, 80, \"B\", row=2)       " << ANSI_COLOR_WHITE << "// Pinned to row 2 (stacked with row=1)\n\n";

  ss << COOL_COLOR_GREEN << "UTILITY\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  print(val)      " << COOL_COLOR_ORANGE << "- Print value to console\n";
  ss << ANSI_COLOR_WHITE << "  funcz()         " << COOL_COLOR_ORANGE << "- List environment variables\n";
  ss << ANSI_COLOR_WHITE << "  type(val)       " << COOL_COLOR_ORANGE << "- Returns the type of value\n";
  ss << ANSI_COLOR_WHITE << "  now()           " << COOL_COLOR_ORANGE << "- Returns current audio tick\n";
  ss << ANSI_COLOR_WHITE << "  timing_info()   " << COOL_COLOR_ORANGE << "- Print audio timing/sync info\n";
  ss << ANSI_COLOR_WHITE << "  synchz(n)       " << COOL_COLOR_ORANGE << "- Hz per timing unit at quantize n (2/4/8/16/32)\n";
  ss << ANSI_COLOR_WHITE << "  websock(bool)   " << COOL_COLOR_ORANGE << "- Enable/disable websocket interface\n\n";

  ss << COOL_COLOR_GREEN << "For more help:\n";
  ss << ANSI_COLOR_WHITE << "  Type " << COOL_COLOR_YELLOW << "'help'" << COOL_COLOR_ORANGE << " to see this again\n";
  ss << ANSI_COLOR_WHITE << "  Type " << COOL_COLOR_YELLOW << "'funcz()'" << COOL_COLOR_ORANGE << " to see current environment\n";
  ss << ANSI_COLOR_WHITE << "  Use "  << COOL_COLOR_YELLOW << "info(instrument)" << COOL_COLOR_ORANGE << " to see all parameters\n";
  ss << ANSI_COLOR_RESET << "\n";
  // clang-format on
  return ss.str();
}

std::string build_midi_ref() {
  // clang-format off
  std::stringstream ss;

  auto note = [&](const char* name, int num) {
    ss << ANSI_COLOR_WHITE << name << ":" << COOL_COLOR_PINK2 << num << " ";
  };

  ss << COOL_COLOR_BLUE << "Midi Note Reference\n"
     << COOL_COLOR_MAUVE << "------------------------------------------------------------------------\n";

  auto row = [&](const char* label, int base) {
    ss << COOL_COLOR_MAUVE << label << " ";
    note("C",  base);      note("C#", base+1);
    note("D",  base+2);    note("D#", base+3);
    note("E",  base+4);    note("F",  base+5);
    note("F#", base+6);    note("G",  base+7);
    note("G#", base+8);    note("A",  base+9);
    note("A#", base+10);   note("B",  base+11);
    ss << "\n";
  };

  row(" -", 0);
  row(" 0", 12);
  row(" 1", 24);
  row(" 2", 36);
  row(" 3", 48);
  row(" 4", 60);
  row(" 5", 72);

  ss << "\n";
  ss << COOL_COLOR_BLUE << "Chord Progressions\n"
     << COOL_COLOR_MAUVE << "------------------------------------------------------------------------\n"
     << COOL_COLOR_PINK2
     << "  I-IV-V   I-V-vi-IV   I-vi-IV-V   vi-ii-V-I   vi-IV-I-V\n\n";

  ss << COOL_COLOR_BLUE << "Chord Mods\n"
     << COOL_COLOR_MAUVE << "------------------------------------------------------------------------\n"
     << ANSI_COLOR_WHITE << "  None" << COOL_COLOR_PINK2 << "(0)  "
     << ANSI_COLOR_WHITE << "Seventh" << COOL_COLOR_PINK2 << "(1)  "
     << ANSI_COLOR_WHITE << "Seventh Inv" << COOL_COLOR_PINK2 << "(2)  "
     << ANSI_COLOR_WHITE << "Root Inv" << COOL_COLOR_PINK2 << "(3)  "
     << ANSI_COLOR_WHITE << "Power" << COOL_COLOR_PINK2 << "(4)\n\n";

  ss << COOL_COLOR_BLUE << "Key Mods\n"
     << COOL_COLOR_MAUVE << "------------------------------------------------------------------------\n"
     << ANSI_COLOR_WHITE << "  None" << COOL_COLOR_PINK2 << "(0)  "
     << ANSI_COLOR_WHITE << "Natural Minor" << COOL_COLOR_PINK2 << "(1)  "
     << ANSI_COLOR_WHITE << "Harmonic Minor" << COOL_COLOR_PINK2 << "(2)  "
     << ANSI_COLOR_WHITE << "Melodic Minor" << COOL_COLOR_PINK2 << "(3)  "
     << ANSI_COLOR_WHITE << "Phrygian" << COOL_COLOR_PINK2 << "(4)\n\n";

  ss << COOL_COLOR_BLUE << "Filters\n"
     << COOL_COLOR_MAUVE << "------------------------------------------------------------------------\n"
     << COOL_COLOR_PINK2
     << "  LPF1  HPF1  LPF2  HPF2  BPF2  BSF2  LPF4  HPF4  BPF4\n\n";

  ss << COOL_COLOR_BLUE << "Scales\n"
     << COOL_COLOR_MAUVE << "------------------------------------------------------------------------\n"
     << ANSI_COLOR_WHITE << "  Major: " << COOL_COLOR_PINK2 << "W W H W W W H"
     << ANSI_COLOR_WHITE << "   Minor: " << COOL_COLOR_PINK2 << "W H W W H W W"
     << "\n";

  ss << ANSI_COLOR_RESET;
  // clang-format on
  return ss.str();
}
