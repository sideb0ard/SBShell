#include "help.h"

#include <sstream>

#include "defjams.h"

// clang-format off

// ─── topic sections ──────────────────────────────────────────────────────────

static std::string help_functions() {
  std::stringstream ss;
  ss << COOL_COLOR_GREEN << "\nARRAY / COLLECTION\n"
     << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  len(arr|str|map)      " << COOL_COLOR_ORANGE << "- Returns length/size\n";
  ss << ANSI_COLOR_WHITE << "  head(arr)             " << COOL_COLOR_ORANGE << "- First element\n";
  ss << ANSI_COLOR_WHITE << "  tail(arr)             " << COOL_COLOR_ORANGE << "- All but first element\n";
  ss << ANSI_COLOR_WHITE << "  last(arr)             " << COOL_COLOR_ORANGE << "- Last element\n";
  ss << ANSI_COLOR_WHITE << "  push(arr, item)       " << COOL_COLOR_ORANGE << "- Append item\n";
  ss << ANSI_COLOR_WHITE << "  take_n(arr, n)        " << COOL_COLOR_ORANGE << "- First n elements\n";
  ss << ANSI_COLOR_WHITE << "  take_random_n(arr, n) " << COOL_COLOR_ORANGE << "- n random unique elements\n";
  ss << ANSI_COLOR_WHITE << "  reverse(arr)          " << COOL_COLOR_ORANGE << "- Reverse order\n";
  ss << ANSI_COLOR_WHITE << "  rotate(arr, n)        " << COOL_COLOR_ORANGE << "- Rotate by n positions\n";
  ss << ANSI_COLOR_WHITE << "  sort(arr)             " << COOL_COLOR_ORANGE << "- Sort ascending\n";
  ss << ANSI_COLOR_WHITE << "  shuffle(arr)          " << COOL_COLOR_ORANGE << "- Random shuffle\n";
  ss << ANSI_COLOR_WHITE << "  scramble(arr)         " << COOL_COLOR_ORANGE << "- Scramble 16-step rhythm (keeps active beats)\n";
  ss << ANSI_COLOR_WHITE << "  stutter(arr)          " << COOL_COLOR_ORANGE << "- Stutter effect on step pattern\n";
  ss << ANSI_COLOR_WHITE << "  invert(arr)           " << COOL_COLOR_ORANGE << "- Invert 0s and 1s (for rhythms)\n";
  ss << ANSI_COLOR_WHITE << "  is_array(val)         " << COOL_COLOR_ORANGE << "- True if value is an array\n";
  ss << ANSI_COLOR_WHITE << "  is_in(arr, item)      " << COOL_COLOR_ORANGE << "- True if item found\n";
  ss << ANSI_COLOR_WHITE << "  keys(map)             " << COOL_COLOR_ORANGE << "- Array of all map keys\n";
  ss << ANSI_COLOR_WHITE << "  bits(num)             " << COOL_COLOR_ORANGE << "- Integer to array of bits\n";
  ss << ANSI_COLOR_WHITE << "  hex(num)              " << COOL_COLOR_ORANGE << "- Integer to hex string\n\n";

  ss << COOL_COLOR_GREEN << "MATH\n"
     << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  floor(num)                           " << COOL_COLOR_ORANGE << "- Largest integer <= num\n";
  ss << ANSI_COLOR_WHITE << "  abs(num)                             " << COOL_COLOR_ORANGE << "- Absolute value\n";
  ss << ANSI_COLOR_WHITE << "  log(num)                             " << COOL_COLOR_ORANGE << "- log2\n";
  ss << ANSI_COLOR_WHITE << "  pow(base, exp)                       " << COOL_COLOR_ORANGE << "- base raised to exp\n";
  ss << ANSI_COLOR_WHITE << "  sin(num), cos(num)                   " << COOL_COLOR_ORANGE << "- Trig (radians)\n";
  ss << ANSI_COLOR_WHITE << "  max(a, b), min(a, b)                 " << COOL_COLOR_ORANGE << "- Larger/smaller value\n";
  ss << ANSI_COLOR_WHITE << "  incr(num, min, max)                  " << COOL_COLOR_ORANGE << "- Increment with wrap\n";
  ss << ANSI_COLOR_WHITE << "  rincr(num, min, max)                 " << COOL_COLOR_ORANGE << "- Decrement with wrap\n";
  ss << ANSI_COLOR_WHITE << "  dincr(num, min, max)                 " << COOL_COLOR_ORANGE << "- Drunk walk (+1 or -1)\n";
  ss << ANSI_COLOR_WHITE << "  scale(val, in_min, in_max, out_min, out_max) " << COOL_COLOR_ORANGE << "- Map range\n\n";

  ss << COOL_COLOR_GREEN << "RANDOM / GENERATIVE\n"
     << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  rand(n)                        " << COOL_COLOR_ORANGE << "- Random 0..n-1; rand(arr) picks random element\n";
  ss << ANSI_COLOR_WHITE << "  rand_array(size, min, max)     " << COOL_COLOR_ORANGE << "- Array of random numbers\n";
  ss << ANSI_COLOR_WHITE << "  rand_sixteenthz(n)             " << COOL_COLOR_ORANGE << "- n random unique 16th-note positions\n";
  ss << ANSI_COLOR_WHITE << "  perlin(x)                      " << COOL_COLOR_ORANGE << "- Perlin noise (smooth random)\n";
  ss << ANSI_COLOR_WHITE << "  bjork(pulses, steps)           " << COOL_COLOR_ORANGE << "- Euclidean rhythm pattern\n";
  ss << ANSI_COLOR_WHITE << "  gen_perc(len, density)         " << COOL_COLOR_ORANGE << "- Random percussion pattern\n";
  ss << ANSI_COLOR_WHITE << "  lsys_expand(axiom, rules, gen) " << COOL_COLOR_ORANGE << "- L-system sequence (gen 3-4 typical, max 6)\n";
  ss << ANSI_COLOR_WHITE << "    axiom=[0]  rules=[[0,2,4],[1,3,0],[2,4,1],[3,0,2],[4,1,3]]\n";
  ss << ANSI_COLOR_WHITE << "    let seq = lsys_expand([0], rules, 3)  " << COOL_COLOR_ORANGE << "-> 27-note index array\n";
  ss << ANSI_COLOR_WHITE << "    note_on_at(dx, notes[seq[i%len(seq)]], i*pp, dur=180) " << COOL_COLOR_ORANGE << "- use in run()\n\n";

  ss << COOL_COLOR_GREEN << "MUSIC THEORY\n"
     << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  midi_ref()                            " << COOL_COLOR_ORANGE << "- Note names to MIDI numbers\n";
  ss << ANSI_COLOR_WHITE << "  notes_in_key(root, scale)             " << COOL_COLOR_ORANGE << "- MIDI notes in key/scale\n";
  ss << ANSI_COLOR_WHITE << "  notes_in_chord(root, key, mod, scale) " << COOL_COLOR_ORANGE << "- Key-aware chord (quality from scale)\n";
  ss << ANSI_COLOR_WHITE << "    mod:   0=triad 1=seventh 2=seventh_inv 3=root_inv 4=power 5=ninth\n";
  ss << ANSI_COLOR_WHITE << "    scale: 0=major 1=nat_minor 2=harm_minor 3=mel_minor 4=phrygian\n";
  ss << ANSI_COLOR_WHITE << "  chord_notes(root, type, mod)          " << COOL_COLOR_ORANGE << "- Key-independent chord\n";
  ss << ANSI_COLOR_WHITE << "    type: 0=major 1=minor 2=dim 3=power 4=sus2 5=sus4\n";
  ss << ANSI_COLOR_WHITE << "    mod:  0=triad 1=min7 2=maj7 3=inv_min7 4=inv_maj7\n";
  ss << ANSI_COLOR_WHITE << "  scale_note(note, key, scale)          " << COOL_COLOR_ORANGE << "- Quantize note to scale\n";
  ss << ANSI_COLOR_WHITE << "  scale_melody(notes, root, scale)      " << COOL_COLOR_ORANGE << "- Quantize melody array\n";
  ss << ANSI_COLOR_WHITE << "  algoz()                               " << COOL_COLOR_ORANGE << "- ASCII diagram of DX algorithms\n\n";

  ss << COOL_COLOR_GREEN << "SOUND GENERATOR CONTROL\n"
     << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  note_on(gen, note, vel)                 " << COOL_COLOR_ORANGE << "- Trigger note\n";
  ss << ANSI_COLOR_WHITE << "  note_on_at(gen, note, time, vel=, dur=) " << COOL_COLOR_ORANGE << "- Schedule note\n";
  ss << ANSI_COLOR_WHITE << "  note_off(gen, note)                     " << COOL_COLOR_ORANGE << "- Stop note\n";
  ss << ANSI_COLOR_WHITE << "  note_off_at(gen, note, time)            " << COOL_COLOR_ORANGE << "- Schedule note off\n";
  ss << ANSI_COLOR_WHITE << "  stop(gen)                               " << COOL_COLOR_ORANGE << "- Stop all notes\n";
  ss << ANSI_COLOR_WHITE << "  solo(gen), unsolo()                     " << COOL_COLOR_ORANGE << "- Solo/unsolo\n";
  ss << ANSI_COLOR_WHITE << "  play_array(gen, notes)                  " << COOL_COLOR_ORANGE << "- Play array (1 per beat)\n";
  ss << ANSI_COLOR_WHITE << "  play_array_over(gen, notes, spd)        " << COOL_COLOR_ORANGE << "- Play notes over 'spd' beats\n";
  ss << ANSI_COLOR_WHITE << "  stepn(seq)                              " << COOL_COLOR_ORANGE << "- Advance step sequencer\n\n";

  ss << COOL_COLOR_GREEN << "STRINGS / UTILITY\n"
     << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  lowercase(str), uppercase(str)  " << COOL_COLOR_ORANGE << "- Case conversion\n";
  ss << ANSI_COLOR_WHITE << "  print(val)                      " << COOL_COLOR_ORANGE << "- Print to console\n";
  ss << ANSI_COLOR_WHITE << "  type(val)                       " << COOL_COLOR_ORANGE << "- Returns type of value\n";
  ss << ANSI_COLOR_WHITE << "  now()                           " << COOL_COLOR_ORANGE << "- Current audio tick\n";
  ss << ANSI_COLOR_WHITE << "  funcz()                         " << COOL_COLOR_ORANGE << "- List environment variables\n\n";

  ss << ANSI_COLOR_RESET;
  return ss.str();
}

static std::string help_generators() {
  std::stringstream ss;
  ss << COOL_COLOR_GREEN << "\nSOUND GENERATORS\n"
     << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  drumsynth()            " << COOL_COLOR_ORANGE << "- Drum synthesizer (9 voices: bd sd cp hh oh fm1-3 lz)\n";
  ss << ANSI_COLOR_WHITE << "  fmsynth()              " << COOL_COLOR_ORANGE << "- 4-operator FM synth\n";
  ss << ANSI_COLOR_WHITE << "  subsynth()             " << COOL_COLOR_ORANGE << "- Subtractive synth\n";
  ss << ANSI_COLOR_WHITE << "  wavsynth()             " << COOL_COLOR_ORANGE << "- Wavetable/sample synth (8-voice poly)\n";
  ss << ANSI_COLOR_WHITE << "  loop(file)             " << COOL_COLOR_ORANGE << "- Granular looper\n";
  ss << ANSI_COLOR_WHITE << "  sample(path)           " << COOL_COLOR_ORANGE << "- One-shot sample player\n\n";
  ss << COOL_COLOR_GREEN << "  Common to all generators:\n";
  ss << ANSI_COLOR_WHITE << "  vol <gen> <val>        " << COOL_COLOR_ORANGE << "- Volume (0.0-1.0+)\n";
  ss << ANSI_COLOR_WHITE << "  pan <gen> <val>        " << COOL_COLOR_ORANGE << "- Pan (-1.0=L, 0=centre, 1.0=R)\n";
  ss << ANSI_COLOR_WHITE << "  add_fx(gen, fx_name)   " << COOL_COLOR_ORANGE << "- Add effect\n";
  ss << ANSI_COLOR_WHITE << "  set gen:param val      " << COOL_COLOR_ORANGE << "- Set parameter\n";
  ss << ANSI_COLOR_WHITE << "  info gen               " << COOL_COLOR_ORANGE << "- Show all parameters\n";
  ss << ANSI_COLOR_WHITE << "  load_preset(gen, name) " << COOL_COLOR_ORANGE << "- Load preset\n";
  ss << ANSI_COLOR_WHITE << "  save_preset(gen, name) " << COOL_COLOR_ORANGE << "- Save preset\n\n";
  ss << COOL_COLOR_GREEN << "  For detailed params:\n";
  ss << ANSI_COLOR_WHITE << "  help fmsynth      " << COOL_COLOR_ORANGE << "- 4-op FM synth params in full\n";
  ss << ANSI_COLOR_WHITE << "  help subsynth     " << COOL_COLOR_ORANGE << "- subtractive synth params\n";
  ss << ANSI_COLOR_WHITE << "  help drumsynth    " << COOL_COLOR_ORANGE << "- drum voices in depth\n";
  ss << ANSI_COLOR_WHITE << "  help wavsynth     " << COOL_COLOR_ORANGE << "- wavetable/sample synth\n";
  ss << ANSI_COLOR_WHITE << "  help looper       " << COOL_COLOR_ORANGE << "- granular looper\n";
  ss << ANSI_COLOR_WHITE << "  help fx           " << COOL_COLOR_ORANGE << "- all effects\n\n";
  ss << ANSI_COLOR_RESET;
  return ss.str();
}

static std::string help_looper() {
  std::stringstream ss;
  ss << COOL_COLOR_GREEN << "\nGRANULAR LOOPER\n"
     << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  let x = loop(file)        " << COOL_COLOR_ORANGE << "- Create granular looper from sample\n";
  ss << ANSI_COLOR_WHITE << "  set x:len 2               " << COOL_COLOR_ORANGE << "- Loop length in bars (default 1)\n";
  ss << ANSI_COLOR_WHITE << "  set x:speed 0.5           " << COOL_COLOR_ORANGE << "- Playback speed multiplier\n";
  ss << ANSI_COLOR_WHITE << "  set x:pitch 0.5           " << COOL_COLOR_ORANGE << "- Pitch ratio\n";
  ss << ANSI_COLOR_WHITE << "  set x:mode 0              " << COOL_COLOR_ORANGE << "- Loop mode: 0=loop 1=static 2=smudge\n";
  ss << ANSI_COLOR_WHITE << "  set x:reverse 1           " << COOL_COLOR_ORANGE << "- Reverse playback\n\n";

  ss << COOL_COLOR_GREEN << "  Rhythmic FX (trigger on next bar):\n";
  ss << ANSI_COLOR_WHITE << "  set x:scramble 1          " << COOL_COLOR_ORANGE << "- Randomly reorder 16 slices\n";
  ss << ANSI_COLOR_WHITE << "  set x:stutter 1           " << COOL_COLOR_ORANGE << "- Random stutter repeats\n";
  ss << ANSI_COLOR_WHITE << "  set x:strobe 1            " << COOL_COLOR_ORANGE << "- Alternate anchor slice and sequence\n";
  ss << ANSI_COLOR_WHITE << "  set x:gate 1              " << COOL_COLOR_ORANGE << "- Rhythmic gating\n";
  ss << ANSI_COLOR_WHITE << "  set x:speedulate 1        " << COOL_COLOR_ORANGE << "- Half/double speed slice remapping\n";
  ss << ANSI_COLOR_WHITE << "  set x:slowdown 1          " << COOL_COLOR_ORANGE << "- Decelerate into stutter\n";
  ss << ANSI_COLOR_WHITE << "  set x:repeat 1            " << COOL_COLOR_ORANGE << "- Beat repeat: lock and loop a section\n\n";

  ss << COOL_COLOR_GREEN << "  Pitch FX (trigger on next bar):\n";
  ss << ANSI_COLOR_WHITE << "  set x:pitch_ramp 1        " << COOL_COLOR_ORANGE << "- Pitch ramps up/down across bar\n";
  ss << ANSI_COLOR_WHITE << "  set x:octave_jump 1       " << COOL_COLOR_ORANGE << "- Random octave shifts per slice\n";
  ss << ANSI_COLOR_WHITE << "  set x:pitch_staircase 1   " << COOL_COLOR_ORANGE << "- Semitone steps across bar\n\n";

  ss << COOL_COLOR_GREEN << "  Granular engine (global — applies to all buffers):\n";
  ss << ANSI_COLOR_WHITE << "  set x:grains_per_sec 15   " << COOL_COLOR_ORANGE << "- Grain density\n";
  ss << ANSI_COLOR_WHITE << "  set x:grain_dur_ms 80     " << COOL_COLOR_ORANGE << "- Grain duration in ms\n";
  ss << ANSI_COLOR_WHITE << "  set x:grain_overlap 0.2   " << COOL_COLOR_ORANGE << "- Overlap (0.0-0.9, default 0.2)\n";
  ss << ANSI_COLOR_WHITE << "  set x:grain_env 0         " << COOL_COLOR_ORANGE << "- Envelope: 0=Tukey 1=Hann\n";
  ss << ANSI_COLOR_WHITE << "  set x:grain_spray_ms 10   " << COOL_COLOR_ORANGE << "- Random position spray in ms\n";
  ss << ANSI_COLOR_WHITE << "  set x:quasi_grain_fudge 0 " << COOL_COLOR_ORANGE << "- Duration randomisation\n\n";

  ss << COOL_COLOR_GREEN << "  Multi-buffer:\n";
  ss << ANSI_COLOR_WHITE << "  add_buf(x, \"file\")           " << COOL_COLOR_ORANGE << "- Add source buffer\n";
  ss << ANSI_COLOR_WHITE << "  set x:primary 1             " << COOL_COLOR_ORANGE << "- Set buf[1] as primary (drives timing/FX)\n";
  ss << ANSI_COLOR_WHITE << "  set x:buf[N]:pitch 1.5      " << COOL_COLOR_ORANGE << "- Per-buffer pitch ratio\n";
  ss << ANSI_COLOR_WHITE << "  set x:buf[N]:speed 0.5      " << COOL_COLOR_ORANGE << "- Per-buffer speed multiplier\n";
  ss << ANSI_COLOR_WHITE << "  set x:buf[N]:gain 0.8       " << COOL_COLOR_ORANGE << "- Per-buffer amplitude (0.0+)\n";
  ss << ANSI_COLOR_WHITE << "  set x:buf[N]:distortion 0.5 " << COOL_COLOR_ORANGE << "- Per-buffer soft-clip distortion (0.0-1.0)\n";
  ss << ANSI_COLOR_WHITE << "  set x:buf[N]:len 2          " << COOL_COLOR_ORANGE << "- Per-buffer loop length in bars\n";
  ss << ANSI_COLOR_WHITE << "  set x:buf[N]:poffset 4      " << COOL_COLOR_ORANGE << "- Per-buffer pattern offset (0-15)\n";
  ss << ANSI_COLOR_WHITE << "  set x:buf[N]:plooplen 8     " << COOL_COLOR_ORANGE << "- Per-buffer pattern loop length (1-16)\n";
  ss << ANSI_COLOR_WHITE << "  set x:buf[N]:pinc 2         " << COOL_COLOR_ORANGE << "- Per-buffer pattern step increment\n\n";

  ss << COOL_COLOR_GREEN << "  shhh mode (per-grain loudness-based source selection):\n";
  ss << ANSI_COLOR_WHITE << "  set x:shhh 1               " << COOL_COLOR_ORANGE << "- 1=quietest 2=loudest 0=off\n";
  ss << ANSI_COLOR_WHITE << "  set x:shhh_window_ms 80    " << COOL_COLOR_ORANGE << "- RMS window in ms (0=grain_dur)\n\n";

  ss << COOL_COLOR_GREEN << "  Buffer xfader (constant-power, independent of shhh):\n";
  ss << ANSI_COLOR_WHITE << "  set x:xfl 0                " << COOL_COLOR_ORANGE << "- Assign buf to xfader Left side\n";
  ss << ANSI_COLOR_WHITE << "  set x:xfr 1                " << COOL_COLOR_ORANGE << "- Assign buf to xfader Right side\n";
  ss << ANSI_COLOR_WHITE << "  set x:xfpos -1             " << COOL_COLOR_ORANGE << "- Position (-1=L, 0=centre, 1=R)\n";
  ss << ANSI_COLOR_WHITE << "  set x:xfspeed 0.002        " << COOL_COLOR_ORANGE << "- Ramp rate (position units per sample)\n";
  ss << ANSI_COLOR_WHITE << "  set x:xfclear 1            " << COOL_COLOR_ORANGE << "- Clear assignments, reset to centre\n\n";

  ss << ANSI_COLOR_RESET;
  return ss.str();
}

static std::string help_drums() {
  std::stringstream ss;
  ss << COOL_COLOR_GREEN << "\nDRUM SYNTH\n"
     << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  let d = drumsynth()              " << COOL_COLOR_ORANGE << "- Create drum machine\n";
  ss << ANSI_COLOR_WHITE << "  load_preset(d, \"TR808\")          " << COOL_COLOR_ORANGE << "- Load kit preset\n";
  ss << ANSI_COLOR_WHITE << "  list_presets(d)                  " << COOL_COLOR_ORANGE << "- List all kit presets\n";
  ss << ANSI_COLOR_WHITE << "  list_presets(d, \"bd\")            " << COOL_COLOR_ORANGE << "- List presets for one voice\n";
  ss << ANSI_COLOR_WHITE << "  save_drum_part(d, \"MY\", \"bd\")    " << COOL_COLOR_ORANGE << "- Save one voice to preset\n";
  ss << ANSI_COLOR_WHITE << "  load_drum_part(d, \"TR808\", \"bd\") " << COOL_COLOR_ORANGE << "- Load one voice from preset\n";
  ss << ANSI_COLOR_WHITE << "  add_fx(d, \"sidechain\", d, \"bd\")  " << COOL_COLOR_ORANGE << "- Sidechain from specific voice\n\n";
  ss << ANSI_COLOR_WHITE << "  Voices (MIDI note): bd=0 sd=1 cp=2 hh=3 oh=4 fm1=5 fm2=6 fm3=7 lz=8\n\n";

  ss << COOL_COLOR_GREEN << "  Kick drum:\n";
  ss << ANSI_COLOR_WHITE << "  set d:bd_noise_en 1           " << COOL_COLOR_ORANGE << "- Enable noise transient layer\n";
  ss << ANSI_COLOR_WHITE << "  set d:bd_pitch_env2_range 38  " << COOL_COLOR_ORANGE << "- Fast pitch spike depth (semitones)\n";
  ss << ANSI_COLOR_WHITE << "  set d:bd_pitch_env2_decay 8   " << COOL_COLOR_ORANGE << "- Fast pitch spike decay (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set d:bd_chirp_en 1           " << COOL_COLOR_ORANGE << "- Enable chirp (exp freq sweep)\n";
  ss << ANSI_COLOR_WHITE << "  set d:bd_chirp_start 3000     " << COOL_COLOR_ORANGE << "- Chirp start freq (Hz)\n";
  ss << ANSI_COLOR_WHITE << "  set d:bd_chirp_end 200        " << COOL_COLOR_ORANGE << "- Chirp end freq (Hz)\n";
  ss << ANSI_COLOR_WHITE << "  set d:bd_chirp_decay 10       " << COOL_COLOR_ORANGE << "- Chirp sweep duration (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set d:bd_mod_en 1             " << COOL_COLOR_ORANGE << "- Enable FM modulator\n";
  ss << ANSI_COLOR_WHITE << "  set d:bd_mod_freq 140         " << COOL_COLOR_ORANGE << "- FM modulator freq (Hz)\n";
  ss << ANSI_COLOR_WHITE << "  set d:bd_mod_index 200        " << COOL_COLOR_ORANGE << "- FM depth in Hz (try 50-2000)\n";
  ss << ANSI_COLOR_WHITE << "  set d:bd_mod_decay 200        " << COOL_COLOR_ORANGE << "- FM envelope decay (ms)\n\n";

  ss << COOL_COLOR_GREEN << "  Snare:\n";
  ss << ANSI_COLOR_WHITE << "  set d:sd_pitch_eg_depth 8     " << COOL_COLOR_ORANGE << "- Upward pitch sweep (semitones, 0=off)\n";
  ss << ANSI_COLOR_WHITE << "  set d:sd_pitch_eg_decay 25    " << COOL_COLOR_ORANGE << "- Pitch sweep decay (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set d:sd_hi_ratio 2.3         " << COOL_COLOR_ORANGE << "- Hi osc ratio (default 2.0, 2.3=SC-style)\n";
  ss << ANSI_COLOR_WHITE << "  set d:sd_psat_en 1            " << COOL_COLOR_ORANGE << "- Parallel tanh saturation\n";
  ss << ANSI_COLOR_WHITE << "  set d:sd_psat_drive 4.47      " << COOL_COLOR_ORANGE << "- Saturation pre-gain\n";
  ss << ANSI_COLOR_WHITE << "  set d:sd_psat_blend 0.316     " << COOL_COLOR_ORANGE << "- Saturation wet blend\n\n";

  ss << COOL_COLOR_GREEN << "  Hi-Hat / Open Hat (replace hh_ with oh_ for open hat):\n";
  ss << ANSI_COLOR_WHITE << "  set d:hh_decay 10             " << COOL_COLOR_ORANGE << "- Decay (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set d:hh_sqamp 0.5            " << COOL_COLOR_ORANGE << "- Square bank amplitude\n";
  ss << ANSI_COLOR_WHITE << "  set d:hh_midf 8000            " << COOL_COLOR_ORANGE << "- BPF centre (Hz)\n";
  ss << ANSI_COLOR_WHITE << "  set d:hh_midf_q 3             " << COOL_COLOR_ORANGE << "- BPF resonance (higher=more sizzle)\n";
  ss << ANSI_COLOR_WHITE << "  set d:hh_hif 7000             " << COOL_COLOR_ORANGE << "- HPF cutoff (Hz)\n\n";

  ss << COOL_COLOR_GREEN << "  Clap (four-voice, TR-808 style):\n";
  ss << ANSI_COLOR_WHITE << "  set d:cp_v2_delay 12          " << COOL_COLOR_ORANGE << "- Voice 2 delay (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set d:cp_v2_vol 0.7           " << COOL_COLOR_ORANGE << "- Voice 2 volume\n";
  ss << ANSI_COLOR_WHITE << "  set d:cp_v2_attack 15         " << COOL_COLOR_ORANGE << "- Voice 2 attack (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set d:cp_v2_decay 150         " << COOL_COLOR_ORANGE << "- Voice 2 decay (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set d:cp_v4_delay 30          " << COOL_COLOR_ORANGE << "- Voice 4 delay (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set d:cp_v4_decay 400         " << COOL_COLOR_ORANGE << "- Voice 4 decay (longer = reverb tail)\n\n";

  ss << ANSI_COLOR_RESET;
  return ss.str();
}

static std::string help_wavsynth() {
  std::stringstream ss;
  ss << COOL_COLOR_GREEN << "\nWAVSYNTH — WAVETABLE / SAMPLE SYNTH\n"
     << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  let s = wavsynth()           " << COOL_COLOR_ORANGE << "- Create synth (8-voice polyphonic)\n";
  ss << ANSI_COLOR_WHITE << "  add_buf(s, \"waves/sine.wav\") " << COOL_COLOR_ORANGE << "- Load waveform or sample\n\n";
  ss << COOL_COLOR_GREEN << "  Modes:\n";
  ss << ANSI_COLOR_WHITE << "  set s:mode 0  " << COOL_COLOR_ORANGE << "- Wavetable: cycles buffer at note frequency (default)\n";
  ss << ANSI_COLOR_WHITE << "  set s:mode 1  " << COOL_COLOR_ORANGE << "- Sample: plays at pitches relative to root note\n\n";
  ss << COOL_COLOR_GREEN << "  ADSR:\n";
  ss << ANSI_COLOR_WHITE << "  set s:attack  10   " << COOL_COLOR_ORANGE << "- Attack  (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set s:decay   200  " << COOL_COLOR_ORANGE << "- Decay   (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set s:sustain 0.7  " << COOL_COLOR_ORANGE << "- Sustain (0.0-1.0)\n";
  ss << ANSI_COLOR_WHITE << "  set s:release 300  " << COOL_COLOR_ORANGE << "- Release (ms)\n\n";
  ss << COOL_COLOR_GREEN << "  Filter (Moog ladder):\n";
  ss << ANSI_COLOR_WHITE << "  set s:cutoff 4000  " << COOL_COLOR_ORANGE << "- Cutoff (Hz, default 18000=open)\n";
  ss << ANSI_COLOR_WHITE << "  set s:q      3.0   " << COOL_COLOR_ORANGE << "- Resonance (1.0-10.0)\n\n";
  ss << COOL_COLOR_GREEN << "  Sample mode:\n";
  ss << ANSI_COLOR_WHITE << "  set s:root 60  " << COOL_COLOR_ORANGE << "- MIDI note that plays at 1x speed (default C4)\n";
  ss << ANSI_COLOR_WHITE << "  set s:loop 0   " << COOL_COLOR_ORANGE << "- 0=one-shot, 1=loop\n\n";
  ss << COOL_COLOR_GREEN << "  Wavetable morphing (multiple buffers):\n";
  ss << ANSI_COLOR_WHITE << "  add_buf(s, \"waves/saw.wav\")   " << COOL_COLOR_ORANGE << "- Add second waveform\n";
  ss << ANSI_COLOR_WHITE << "  set s:morph 0.5              " << COOL_COLOR_ORANGE << "- Crossfade between buffers (0.0-1.0)\n\n";
  ss << COOL_COLOR_ORANGE << "  Example:\n"
     << "    let s = wavsynth();\n"
     << "    add_buf(s, \"perc/vocal.wav\");\n"
     << "    set s:mode 1; set s:root 60;\n"
     << "    note_on(s, 60); note_on(s, 64); note_on(s, 67);\n\n";
  ss << ANSI_COLOR_RESET;
  return ss.str();
}

static std::string help_mixer() {
  std::stringstream ss;
  ss << COOL_COLOR_GREEN << "\nMIXER / ROUTING\n"
     << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  send(effect, gen)         " << COOL_COLOR_ORANGE << "- Send generator to effect (default level 0.4)\n";
  ss << ANSI_COLOR_WHITE << "  send(effect, gen, level)  " << COOL_COLOR_ORANGE << "- Send with explicit intensity (0.0-1.0)\n";
  ss << ANSI_COLOR_WHITE << "  set_send(gen, fx, level)  " << COOL_COLOR_ORANGE << "- Update send intensity (fx: \"delay\"/\"reverb\"/\"distort\")\n";
  ss << COOL_COLOR_ORANGE << "    // ps shows each generator's active sends (e.g. sends: dly:0.40)\n";
  ss << ANSI_COLOR_WHITE << "  add_fx(gen, fx_name)   " << COOL_COLOR_ORANGE << "- Add effect to generator\n";
  ss << ANSI_COLOR_WHITE << "  mvol(volume)           " << COOL_COLOR_ORANGE << "- Master volume (0.0-1.0)\n";
  ss << ANSI_COLOR_WHITE << "  monitor(filepath)      " << COOL_COLOR_ORANGE << "- Monitor file for live coding\n\n";
  ss << COOL_COLOR_GREEN << "  Granulate FX:\n";
  ss << ANSI_COLOR_WHITE << "  add_fx(gen, \"gran\")              " << COOL_COLOR_ORANGE << "- Live granular FX\n";
  ss << ANSI_COLOR_WHITE << "  set gen:fx0:wet 0.5             " << COOL_COLOR_ORANGE << "- Dry/wet mix\n";
  ss << ANSI_COLOR_WHITE << "  set gen:fx0:grain_overlap 0.5   " << COOL_COLOR_ORANGE << "- Grain overlap\n";
  ss << ANSI_COLOR_WHITE << "  set gen:fx0:grain_env 1         " << COOL_COLOR_ORANGE << "- 0=Tukey 1=Hann\n";
  ss << ANSI_COLOR_WHITE << "  set gen:fx0:grains_per_sec 15   " << COOL_COLOR_ORANGE << "- Grain density\n";
  ss << ANSI_COLOR_WHITE << "  set gen:fx0:grain_dur_ms 80     " << COOL_COLOR_ORANGE << "- Grain duration\n";
  ss << ANSI_COLOR_WHITE << "  set gen:fx0:grain_spray_ms 10   " << COOL_COLOR_ORANGE << "- Position randomisation\n\n";

  ss << COOL_COLOR_GREEN << "CROSSFADER\n"
     << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  xassign(ch, gen)   " << COOL_COLOR_ORANGE << "- Assign generator to xfader channel (0=L, 1=R)\n";
  ss << ANSI_COLOR_WHITE << "  xremove(ch, gen)   " << COOL_COLOR_ORANGE << "- Remove generator from xfader channel\n";
  ss << ANSI_COLOR_WHITE << "  xclear()           " << COOL_COLOR_ORANGE << "- Clear all xfader assignments\n";
  ss << ANSI_COLOR_WHITE << "  xfade(direction)   " << COOL_COLOR_ORANGE << "- Crossfade toward LEFT(0) or RIGHT(1)\n\n";

  ss << COOL_COLOR_GREEN << "SCHEDULING\n"
     << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  sched(delay, start, end, time, action) " << COOL_COLOR_ORANGE << "- Parameter automation\n";
  ss << ANSI_COLOR_WHITE << "    e.g. sched(0, 0.8, 0.2, pp*16, \"vol dx %\") " << COOL_COLOR_ORANGE << "- fade dx out over one bar\n\n";

  ss << COOL_COLOR_GREEN << "MIDI\n"
     << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  See: help midi        " << COOL_COLOR_ORANGE << "- Full MIDI reference (init, record, CC map, conversion)\n\n";

  ss << COOL_COLOR_GREEN << "GLOBAL SENDS\n"
     << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  global_reverb(amt)      " << COOL_COLOR_ORANGE << "- Route full dry mix into global reverb (0.0-1.0)\n";
  ss << ANSI_COLOR_WHITE << "  global_delay(amt)       " << COOL_COLOR_ORANGE << "- Route full dry mix into global delay (0.0-1.0)\n";
  ss << ANSI_COLOR_WHITE << "  global_distort(amt)     " << COOL_COLOR_ORANGE << "- Route full dry mix into global distortion (0.0-1.0)\n";
  ss << ANSI_COLOR_WHITE << "  global_reverb_fb(amt)   " << COOL_COLOR_ORANGE << "- Feed reverb output back into reverb (0.0-0.98)\n";
  ss << ANSI_COLOR_WHITE << "  global_delay_fb(amt)    " << COOL_COLOR_ORANGE << "- Feed delay output back into delay (0.0-0.98)\n";
  ss << ANSI_COLOR_WHITE << "  global_distort_fb(amt)  " << COOL_COLOR_ORANGE << "- Feed distort output back into distort (0.0-0.98)\n";
  ss << COOL_COLOR_ORANGE << "  // Additive — per-generator sends still apply\n";
  ss << COOL_COLOR_ORANGE << "  // ps shows global sends and per-FX routing in the header\n";
  ss << COOL_COLOR_ORANGE << "  // Example: slowly suck everything into a reverb swoosh:\n";
  ss << COOL_COLOR_ORANGE << "  sched(0, 0.0, 1.0, pp*32, \"global_reverb %\") // fade in\n";
  ss << COOL_COLOR_ORANGE << "  global_reverb_fb(0.6)                        // bloom\n";
  ss << COOL_COLOR_ORANGE << "  sched(pp*32, 1.0, 0.0, pp*32, \"global_reverb %\") // fade out\n\n";

  ss << COOL_COLOR_GREEN << "FILTERS / EQ\n"
     << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  add_fx(inst, \"djeq\")       " << COOL_COLOR_ORANGE << "- DJ-style 2-band EQ\n";
  ss << ANSI_COLOR_WHITE << "  set inst:fx0:lo <hz>       " << COOL_COLOR_ORANGE << "- HPF cutoff (bass cut, default 80=open)\n";
  ss << ANSI_COLOR_WHITE << "  set inst:fx0:hi <hz>       " << COOL_COLOR_ORANGE << "- LPF cutoff (treble cut, default 18000=open)\n\n";

  ss << ANSI_COLOR_RESET;
  return ss.str();
}

static std::string help_phasor() {
  std::stringstream ss;
  ss << COOL_COLOR_GREEN << "\nPHASOR — CYCLIC RAMP SIGNAL\n"
     << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  let p = phasor(steps)    " << COOL_COLOR_ORANGE << "- Create ramp: 0.0→1.0 over 'steps' MIDI ticks\n";
  ss << ANSI_COLOR_WHITE << "  signal_from(p)           " << COOL_COLOR_ORANGE << "- Advance and read current phase (0.0-1.0)\n";
  ss << ANSI_COLOR_WHITE << "  change_steps(p, steps)   " << COOL_COLOR_ORANGE << "- Retune cycle length live (phase preserved)\n";
  ss << ANSI_COLOR_WHITE << "  reset(p)                 " << COOL_COLOR_ORANGE << "- Snap phase back to 0\n";
  ss << ANSI_COLOR_WHITE << "  steps: 3840=1bar  1920=halfbar  960=beat  7680=2bars\n\n";

  ss << COOL_COLOR_GREEN << "FACTORY FUNCTIONS  (in startup.sb — always available)\n"
     << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";

  ss << COOL_COLOR_GREEN << "  ramp2slope_factory()  — instantaneous slope / derivative\n";
  ss << ANSI_COLOR_WHITE
     << "  Returns fn(sig): per-tick rate-of-change of the ramp.\n"
     << "  Closure keeps history_val so it can diff consecutive samples.\n"
     << "  Handles the 1→0 wrap correctly (returns small +ve, not a spike).\n"
     << "  Mostly ~1/steps per tick; used internally by ramp_div_factory.\n\n";
  ss << COOL_COLOR_ORANGE
     << "    let slope = ramp2slope_factory();\n"
     << "    let ds = slope(sig);   // ~0.00026/tick for steps=3840\n\n";

  ss << COOL_COLOR_GREEN << "  ramp_div_factory()  — fractional-speed sub-ramp\n";
  ss << ANSI_COLOR_WHITE
     << "  Returns fn(sig, ratio): runs a new 0→1 ramp at 'ratio' of\n"
     << "  the master ramp speed. Internally: slope = ramp2slope(sig),\n"
     << "  accum += slope/ratio, wraps at 1. Each factory() call is\n"
     << "  independent — closures hold their own accum.\n"
     << "  Phase carries across bars (accum lives in setup()).\n\n"
     << "  ratio = 1/N  → N complete cycles per bar, triggers evenly spaced\n"
     << "  ratio = P/Q  → Q/P cycles/bar; phase drifts, pattern repeats\n"
     << "                 after lcm(P,Q) bars (irrational euclidean feel)\n\n";
  ss << COOL_COLOR_ORANGE
     << "    let div = ramp_div_factory();\n"
     << "    let s = div(ramp, 1/3);   // 3 even triggers per bar\n"
     << "    let s = div(ramp, 3/8);   // ~2.67/bar, drifts across 3 bars\n\n";

  ss << COOL_COLOR_GREEN << "  ramp2trigger_factory()  — wrap-around trigger\n";
  ss << ANSI_COLOR_WHITE
     << "  Returns fn(sig): true exactly once per cycle, when the ramp\n"
     << "  wraps 1→0 (detected as abs(delta) > 0.5). Closure holds\n"
     << "  history_val. Combine with ramp_div to trigger notes at any\n"
     << "  rhythmic subdivision.\n\n";
  ss << COOL_COLOR_ORANGE
     << "    let trig = ramp2trigger_factory();\n"
     << "    if (trig(sub)) { note_on_at(sg, 60, i); }\n\n";

  ss << COOL_COLOR_GREEN << "FULL EXAMPLE — phazor_draw\n"
     << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << COOL_COLOR_ORANGE
     << "  let phazor_draw = comp()\n"
     << "  {\n"
     << "    setup()\n"
     << "    {\n"
     << "      let fullramp = phasor(3840);        // master 1-bar ramp\n"
     << "      let ramp_div1 = ramp_div_factory(); // each holds own accum\n"
     << "      let ramp_div2 = ramp_div_factory();\n"
     << "      let ramp_div3 = ramp_div_factory();\n"
     << "      let ramp2tri1 = ramp2trigger_factory();\n"
     << "      let ramp2tri2 = ramp2trigger_factory();\n"
     << "      let ramp2tri3 = ramp2trigger_factory();\n"
     << "      let plot_width = 80;\n"
     << "      let stride = 3840 / plot_width;     // 48 ticks per column\n"
     << "    }\n"
     << "    run()\n"
     << "    {\n"
     << "      for (let i = 0; i < 3840; i++) {\n"
     << "        let rampval  = signal_from(fullramp);\n"
     << "        let out_sig1 = ramp_div1(rampval, 3/8);     // ~2.67/bar\n"
     << "        let out_sig2 = ramp_div2(rampval, 3/15);    // 5/bar\n"
     << "        let mod_ratio = max(out_sig2, 0.001);        // avoid /0\n"
     << "        let out_sig3 = ramp_div3(out_sig1, mod_ratio); // FM!\n"
     << "        if (i % stride == 0) {\n"
     << "          draw_plot(rampval,  plot_width, \"r  \", at=i, row=1);\n"
     << "          draw_plot(out_sig1, plot_width, \"s1 \", at=i, row=2);\n"
     << "          draw_plot(out_sig2, plot_width, \"s2 \", at=i, row=3);\n"
     << "          draw_plot(out_sig3, plot_width, \"s3 \", at=i, row=4);\n"
     << "        }\n"
     << "        if (ramp2tri1(out_sig1)) { print(\"s1 NOTEON:\", i); }\n"
     << "        if (ramp2tri2(out_sig2)) { print(\"s2 NOTEON:\", i); }\n"
     << "        if (ramp2tri3(out_sig3)) { print(\"s3 NOTEON:\", i); }\n"
     << "      }\n"
     << "    }\n"
     << "  }\n\n";
  ss << COOL_COLOR_GREEN << "  out_sig3 — modulated divisor (FM ramp)\n";
  ss << ANSI_COLOR_WHITE
     << "  ramp_div3 takes out_sig1 as its *input* and out_sig2 as its\n"
     << "  *ratio*. out_sig2 is itself a 0→1 ramp cycling 5x/bar, so the\n"
     << "  ratio passed to ramp_div3 sweeps from ~0 to ~1 continuously.\n\n"
     << "  Effect: when out_sig2 is near 0 (ratio tiny), accum advances\n"
     << "  very fast — out_sig3 races through many cycles. When out_sig2\n"
     << "  is near 1 (ratio ~1), out_sig3 tracks out_sig1 1:1. The result\n"
     << "  is a ramp that accelerates and decelerates 5 times per bar,\n"
     << "  producing a burst of closely-spaced triggers at the start of\n"
     << "  each out_sig2 cycle that spread out as out_sig2 rises.\n"
     << "  This is the ramp equivalent of FM synthesis — using one signal\n"
     << "  to modulate the frequency of another.\n\n";

  ss << COOL_COLOR_GREEN << "VISUALIZATION\n"
     << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  draw_bar(val, width, label, at=, row=)  " << COOL_COLOR_ORANGE << "- Bar graph (val: 0.0-1.0)\n";
  ss << ANSI_COLOR_WHITE << "  draw_plot(val, width, label, at=, row=) " << COOL_COLOR_ORANGE << "- Waveform sparkline (pin with row=)\n";
  ss << ANSI_COLOR_WHITE << "    at=<ticks>  " << COOL_COLOR_ORANGE << "- Exact tick column — use at=i inside for loop\n";
  ss << ANSI_COLOR_WHITE << "    row=<n>     " << COOL_COLOR_ORANGE << "- Fixed terminal row; Ctrl-L parks cursor below all rows\n";
  ss << ANSI_COLOR_WHITE << "    width=80    " << COOL_COLOR_ORANGE << "- Match terminal width (3840/80=48 ticks per column)\n\n";

  ss << ANSI_COLOR_RESET;
  return ss.str();
}

static std::string help_fmsynth() {
  std::stringstream ss;
  ss << COOL_COLOR_GREEN << "\nFM SYNTH — 4-OPERATOR FM\n"
     << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  let dx = fmsynth()            " << COOL_COLOR_ORANGE << "- Create 4-op FM synth\n";
  ss << ANSI_COLOR_WHITE << "  set dx:algo 0                 " << COOL_COLOR_ORANGE << "- Algorithm 0-7 (see algoz() for diagram)\n";
  ss << ANSI_COLOR_WHITE << "  set dx:porta 0.01             " << COOL_COLOR_ORANGE << "- Portamento rate (0=off)\n";
  ss << ANSI_COLOR_WHITE << "  set dx:pitchrange 2           " << COOL_COLOR_ORANGE << "- Pitch bend range (semitones)\n";
  ss << ANSI_COLOR_WHITE << "  set dx:legato 1               " << COOL_COLOR_ORANGE << "- Legato mode\n";
  ss << ANSI_COLOR_WHITE << "  set dx:vel2att 1              " << COOL_COLOR_ORANGE << "- Velocity controls attack\n";
  ss << ANSI_COLOR_WHITE << "  set dx:note2dec 1             " << COOL_COLOR_ORANGE << "- Note pitch controls decay\n";
  ss << COOL_COLOR_GREEN << "  Per-operator (N = 1-4):\n";
  ss << ANSI_COLOR_WHITE << "  set dx:oNwav 0                " << COOL_COLOR_ORANGE << "- Waveform: 0=sine 1=tri 2=saw 3=sq 4=half-sine\n";
  ss << ANSI_COLOR_WHITE << "  set dx:oNrat 1.0              " << COOL_COLOR_ORANGE << "- Frequency ratio (e.g. 0.5=sub, 4.7=bells)\n";
  ss << ANSI_COLOR_WHITE << "  set dx:oNdet 0.0              " << COOL_COLOR_ORANGE << "- Fine detune (cents)\n";
  ss << ANSI_COLOR_WHITE << "  set dx:oNsus 0.7              " << COOL_COLOR_ORANGE << "- Sustain level (0.0-1.0)\n";
  ss << ANSI_COLOR_WHITE << "  set dx:opNfreq 440            " << COOL_COLOR_ORANGE << "- Override: fixed freq instead of ratio\n";
  ss << ANSI_COLOR_WHITE << "  set dx:opNfb 0.0              " << COOL_COLOR_ORANGE << "- Self-feedback (0.0-1.0)\n";
  ss << ANSI_COLOR_WHITE << "  set dx:opNout 1.0             " << COOL_COLOR_ORANGE << "- Output level (0.0-1.0)\n";
  ss << ANSI_COLOR_WHITE << "  set dx:eNatt 5                " << COOL_COLOR_ORANGE << "- Attack  (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set dx:eNdec 200              " << COOL_COLOR_ORANGE << "- Decay   (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set dx:eNsus 0.5              " << COOL_COLOR_ORANGE << "- Sustain (0.0-1.0)\n";
  ss << ANSI_COLOR_WHITE << "  set dx:eNrel 300              " << COOL_COLOR_ORANGE << "- Release (ms)\n";
  ss << COOL_COLOR_GREEN << "  LFO (l1_*):\n";
  ss << ANSI_COLOR_WHITE << "  set dx:l1_wav 0               " << COOL_COLOR_ORANGE << "- Waveform: 0=sine 1=tri 2=saw 3=sq\n";
  ss << ANSI_COLOR_WHITE << "  set dx:l1_rate 2.0            " << COOL_COLOR_ORANGE << "- Rate (Hz)\n";
  ss << ANSI_COLOR_WHITE << "  set dx:l1_int 0.5             " << COOL_COLOR_ORANGE << "- Depth (0.0-1.0)\n";
  ss << ANSI_COLOR_WHITE << "  set dx:l1_dest1 1             " << COOL_COLOR_ORANGE << "- Route to op1 (1=on); dest2-4 for other ops\n\n";
  ss << ANSI_COLOR_RESET;
  return ss.str();
}

static std::string help_subsynth() {
  std::stringstream ss;
  ss << COOL_COLOR_GREEN << "\nSUBSYNTH — SUBTRACTIVE SYNTH\n"
     << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  ss << ANSI_COLOR_WHITE << "  let sb = subsynth()           " << COOL_COLOR_ORANGE << "- Create subtractive synth\n";
  ss << COOL_COLOR_GREEN << "  Oscillators (N = 1-4):\n";
  ss << ANSI_COLOR_WHITE << "  set sb:oscN 0                 " << COOL_COLOR_ORANGE << "- Waveform: 0=sine 1=saw 2=sq 3=tri 4=noise\n";
  ss << ANSI_COLOR_WHITE << "  set sb:oNamp 0.8              " << COOL_COLOR_ORANGE << "- Oscillator level (0.0-1.0)\n";
  ss << ANSI_COLOR_WHITE << "  set sb:oNoct 0                " << COOL_COLOR_ORANGE << "- Octave offset (-3 to 3)\n";
  ss << ANSI_COLOR_WHITE << "  set sb:oNsemi 0               " << COOL_COLOR_ORANGE << "- Semitone offset\n";
  ss << ANSI_COLOR_WHITE << "  set sb:o1cents 0              " << COOL_COLOR_ORANGE << "- Fine detune (cents, osc1 only)\n";
  ss << ANSI_COLOR_WHITE << "  set sb:pw 0.5                 " << COOL_COLOR_ORANGE << "- Pulse width for sq oscillators (0.0-1.0)\n";
  ss << ANSI_COLOR_WHITE << "  set sb:subosc 0.3             " << COOL_COLOR_ORANGE << "- Sub oscillator level (one octave below osc1)\n";
  ss << ANSI_COLOR_WHITE << "  set sb:noisedb -60            " << COOL_COLOR_ORANGE << "- Noise floor mix (dB, -96=off)\n";
  ss << ANSI_COLOR_WHITE << "  set sb:detune 0.02            " << COOL_COLOR_ORANGE << "- Global oscillator spread (0=none)\n";
  ss << COOL_COLOR_GREEN << "  Global:\n";
  ss << ANSI_COLOR_WHITE << "  set sb:voice 0                " << COOL_COLOR_ORANGE << "- 0=poly 1=mono\n";
  ss << ANSI_COLOR_WHITE << "  set sb:hard_sync 1            " << COOL_COLOR_ORANGE << "- Hard sync osc2/3/4 to osc1\n";
  ss << ANSI_COLOR_WHITE << "  set sb:octave 0               " << COOL_COLOR_ORANGE << "- Master octave shift\n";
  ss << ANSI_COLOR_WHITE << "  set sb:porta 0.01             " << COOL_COLOR_ORANGE << "- Portamento rate\n";
  ss << ANSI_COLOR_WHITE << "  set sb:legato 1               " << COOL_COLOR_ORANGE << "- Legato mode\n";
  ss << COOL_COLOR_GREEN << "  Filter:\n";
  ss << ANSI_COLOR_WHITE << "  set sb:filter 0               " << COOL_COLOR_ORANGE << "- Type: 0=LPF 1=HPF 2=BPF 3=BSF\n";
  ss << ANSI_COLOR_WHITE << "  set sb:fc 2000                " << COOL_COLOR_ORANGE << "- Cutoff freq (Hz)\n";
  ss << ANSI_COLOR_WHITE << "  set sb:fq 1.0                 " << COOL_COLOR_ORANGE << "- Resonance (1.0-10.0)\n";
  ss << ANSI_COLOR_WHITE << "  set sb:sat 0                  " << COOL_COLOR_ORANGE << "- Filter saturation\n";
  ss << ANSI_COLOR_WHITE << "  set sb:kt 1                   " << COOL_COLOR_ORANGE << "- Key tracking (1=on)\n";
  ss << COOL_COLOR_GREEN << "  Envelopes:\n";
  ss << ANSI_COLOR_WHITE << "  set sb:eg1_attack 10          " << COOL_COLOR_ORANGE << "- EG1 attack  (ms) — usually routed to amp\n";
  ss << ANSI_COLOR_WHITE << "  set sb:eg1_decay 200          " << COOL_COLOR_ORANGE << "- EG1 decay   (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set sb:eg1_sus 0.7            " << COOL_COLOR_ORANGE << "- EG1 sustain (0.0-1.0)\n";
  ss << ANSI_COLOR_WHITE << "  set sb:eg1_release 300        " << COOL_COLOR_ORANGE << "- EG1 release (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set sb:eg1_dca_en 1           " << COOL_COLOR_ORANGE << "- Route EG1 to amp\n";
  ss << ANSI_COLOR_WHITE << "  set sb:eg1_filter_en 1        " << COOL_COLOR_ORANGE << "- Route EG1 to filter\n";
  ss << ANSI_COLOR_WHITE << "  set sb:eg1_filter_int 0.5     " << COOL_COLOR_ORANGE << "- EG1 → filter intensity\n";
  ss << ANSI_COLOR_WHITE << "  set sb:eg2_attack 5           " << COOL_COLOR_ORANGE << "- EG2 (same params, eg2_*)\n";
  ss << COOL_COLOR_GREEN << "  LFOs (l1_* / l2_*):\n";
  ss << ANSI_COLOR_WHITE << "  set sb:l1wave 0               " << COOL_COLOR_ORANGE << "- Waveform: 0=sine 1=tri 2=saw 3=sq\n";
  ss << ANSI_COLOR_WHITE << "  set sb:l1rate 2.0             " << COOL_COLOR_ORANGE << "- Rate (Hz)\n";
  ss << ANSI_COLOR_WHITE << "  set sb:l1amp 0.5              " << COOL_COLOR_ORANGE << "- Depth\n";
  ss << ANSI_COLOR_WHITE << "  set sb:l1_filter_en 1         " << COOL_COLOR_ORANGE << "- Route LFO1 to filter\n";
  ss << ANSI_COLOR_WHITE << "  set sb:l1_osc_en 1            " << COOL_COLOR_ORANGE << "- Route LFO1 to pitch\n";
  ss << ANSI_COLOR_WHITE << "  set sb:l1_amp_en 1            " << COOL_COLOR_ORANGE << "- Route LFO1 to amp (tremolo)\n";
  ss << ANSI_COLOR_WHITE << "  set sb:l1_pan_en 1            " << COOL_COLOR_ORANGE << "- Route LFO1 to pan (auto-pan)\n";
  ss << ANSI_COLOR_WHITE << "  set sb:l1_filter_int 0.5      " << COOL_COLOR_ORANGE << "- LFO1 → filter intensity (l2_* for LFO2)\n\n";
  ss << ANSI_COLOR_RESET;
  return ss.str();
}

static std::string help_fx() {
  std::stringstream ss;
  ss << COOL_COLOR_GREEN << "\nEFFECTS — add_fx(gen, \"name\")  then  set gen:fx0:param val\n"
     << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n\n";

  ss << COOL_COLOR_GREEN << "  distort — soft-clip distortion\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:threshold 0.5    " << COOL_COLOR_ORANGE << "- Clip threshold (0.0-1.0)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:drive 2.0        " << COOL_COLOR_ORANGE << "- Pre-gain drive\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:mode 0           " << COOL_COLOR_ORANGE << "- 0=soft-clip 1=hard-clip 2=fold\n\n";

  ss << COOL_COLOR_GREEN << "  lofi — bit crusher / sample-rate reducer\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:bitdepth 8          " << COOL_COLOR_ORANGE << "- Bit depth (1-16)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:sample_hold_freq 8000 " << COOL_COLOR_ORANGE << "- Sample-and-hold rate (Hz)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:destruct 0.5        " << COOL_COLOR_ORANGE << "- Destruction amount\n\n";

  ss << COOL_COLOR_GREEN << "  reverb — Schroeder reverb\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:predelayms 20    " << COOL_COLOR_ORANGE << "- Pre-delay (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:reverbtime 1.5   " << COOL_COLOR_ORANGE << "- Decay time (s)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:wetmx 0.5        " << COOL_COLOR_ORANGE << "- Wet mix (0.0-1.0)\n\n";

  ss << COOL_COLOR_GREEN << "  diffuser — all-pass diffusion reverb\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:wet 0.5          " << COOL_COLOR_ORANGE << "- Wet mix\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:wet_gain 0.8     " << COOL_COLOR_ORANGE << "- Wet gain\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:sz 0.9           " << COOL_COLOR_ORANGE << "- Room size (0.0-1.0)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:fb 0.6           " << COOL_COLOR_ORANGE << "- Feedback (0.0-1.0)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:uni 0            " << COOL_COLOR_ORANGE << "- 0=stereo 1=mono\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:dif1 0.5         " << COOL_COLOR_ORANGE << "- Diffusion stage 1-4 (dif1-dif4)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:inert1 0.5       " << COOL_COLOR_ORANGE << "- Inertia 1-4 (inert1-inert4)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:reset 1          " << COOL_COLOR_ORANGE << "- Reset diffuser state\n\n";

  ss << COOL_COLOR_GREEN << "  delay — stereo tape delay\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:ms 375           " << COOL_COLOR_ORANGE << "- Delay time (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:fb 0.4           " << COOL_COLOR_ORANGE << "- Feedback (0.0-1.0)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:rat 0.5          " << COOL_COLOR_ORANGE << "- L/R ratio for stereo spread\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:mx 0.5           " << COOL_COLOR_ORANGE << "- Wet mix\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:sync 1           " << COOL_COLOR_ORANGE << "- Sync to tempo (1=on)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:sync_len 0.5     " << COOL_COLOR_ORANGE << "- Sync note value (0.5=8th, 0.25=16th)\n\n";

  ss << COOL_COLOR_GREEN << "  moddelay — chorus/flanger/phaser\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:type 0           " << COOL_COLOR_ORANGE << "- 0=chorus 1=flanger 2=phaser\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:rate 0.5         " << COOL_COLOR_ORANGE << "- LFO rate (Hz)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:depth 0.5        " << COOL_COLOR_ORANGE << "- Modulation depth\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:fb 0.3           " << COOL_COLOR_ORANGE << "- Feedback\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:mix 0.5          " << COOL_COLOR_ORANGE << "- Wet mix\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:offset 0.0       " << COOL_COLOR_ORANGE << "- L/R phase offset\n\n";

  ss << COOL_COLOR_GREEN << "  transverb — pitch-shifting delay (two heads)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:speed1 1.0       " << COOL_COLOR_ORANGE << "- Head 1 read speed (1=unity, 0.5=octave down)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:speed2 1.5       " << COOL_COLOR_ORANGE << "- Head 2 read speed\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:mix1 0.5         " << COOL_COLOR_ORANGE << "- Head 1 wet mix\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:mix2 0.3         " << COOL_COLOR_ORANGE << "- Head 2 wet mix\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:dry 0.7          " << COOL_COLOR_ORANGE << "- Dry level\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:fb1 0.2          " << COOL_COLOR_ORANGE << "- Head 1 feedback\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:fb2 0.2          " << COOL_COLOR_ORANGE << "- Head 2 feedback\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:dist1 0.0        " << COOL_COLOR_ORANGE << "- Head 1 distortion\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:buffer_size 4096 " << COOL_COLOR_ORANGE << "- Buffer size (samples)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:freeze 1         " << COOL_COLOR_ORANGE << "- Freeze buffer\n\n";

  ss << COOL_COLOR_GREEN << "  compressor — dynamics processor\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:threshold -12    " << COOL_COLOR_ORANGE << "- Threshold (dB)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:ratio 4          " << COOL_COLOR_ORANGE << "- Compression ratio (e.g. 4=4:1)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:attackms 5       " << COOL_COLOR_ORANGE << "- Attack (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:releasems 100    " << COOL_COLOR_ORANGE << "- Release (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:inputgain 0      " << COOL_COLOR_ORANGE << "- Input gain (dB)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:outputgain 0     " << COOL_COLOR_ORANGE << "- Output makeup gain (dB)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:kneewidth 3      " << COOL_COLOR_ORANGE << "- Knee width (dB)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:lookahead 0      " << COOL_COLOR_ORANGE << "- Lookahead (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:stereolink 1     " << COOL_COLOR_ORANGE << "- Stereo link\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:type 0           " << COOL_COLOR_ORANGE << "- 0=compressor 1=limiter 2=expander 3=gate\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:extsource gen    " << COOL_COLOR_ORANGE << "- Sidechain source (use add_fx(g,\"sidechain\",src) syntax)\n\n";

  ss << COOL_COLOR_GREEN << "  sculptor — waveform sculptor (spectral reshaping)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:wsize 2048       " << COOL_COLOR_ORANGE << "- Window size (samples, power of 2)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:landmarks 8      " << COOL_COLOR_ORANGE << "- Number of spectral landmarks\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:lparam 0.5       " << COOL_COLOR_ORANGE << "- Landmark parameter\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:interp 0         " << COOL_COLOR_ORANGE << "- Interpolation type\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:wet 0.8          " << COOL_COLOR_ORANGE << "- Wet mix\n\n";

  ss << COOL_COLOR_GREEN << "  gran — live granular processor\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:wet 0.5          " << COOL_COLOR_ORANGE << "- Wet mix\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:grains_per_sec 15 " << COOL_COLOR_ORANGE << "- Grain density\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:grain_dur_ms 80  " << COOL_COLOR_ORANGE << "- Grain duration (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:grain_overlap 0.2 " << COOL_COLOR_ORANGE << "- Overlap (0.0-0.9)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:grain_spray_ms 10 " << COOL_COLOR_ORANGE << "- Position spray (ms)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:grain_env 1      " << COOL_COLOR_ORANGE << "- Envelope: 0=Tukey 1=Hann\n\n";

  ss << COOL_COLOR_GREEN << "  djeq — DJ-style 2-band EQ\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:lo 80            " << COOL_COLOR_ORANGE << "- HPF cutoff (Hz) — bass cut (80=open)\n";
  ss << ANSI_COLOR_WHITE << "  set g:fx0:hi 18000         " << COOL_COLOR_ORANGE << "- LPF cutoff (Hz) — treble cut (18000=open)\n\n";

  ss << ANSI_COLOR_RESET;
  return ss.str();
}

// ─── midi ─────────────────────────────────────────────────────────────────────

static std::string help_midi() {
  std::stringstream ss;
  ss << COOL_COLOR_GREEN << "\nMIDI\n"
     << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";

  ss << COOL_COLOR_GREEN << "SETUP\n";
  ss << ANSI_COLOR_WHITE << "  midi_init()           " << COOL_COLOR_ORANGE << "- Open MIDI device (call once at startup)\n";
  ss << ANSI_COLOR_WHITE << "  midi_assign(gen)      " << COOL_COLOR_ORANGE << "- Route incoming MIDI note-on/off to generator\n";
  ss << ANSI_COLOR_WHITE << "  midi_print()          " << COOL_COLOR_ORANGE << "- Toggle printing of all incoming MIDI events\n\n";

  ss << COOL_COLOR_GREEN << "CC MAPPING\n";
  ss << ANSI_COLOR_WHITE << "  midi_map(cc, param)   " << COOL_COLOR_ORANGE << "- Bind MIDI CC number to a parameter name\n";
  ss << ANSI_COLOR_WHITE << "    e.g. midi_map(74, \"cutoff\")  " << COOL_COLOR_ORANGE << "- CC74 controls cutoff\n";
  ss << ANSI_COLOR_WHITE << "  midi_map()            " << COOL_COLOR_ORANGE << "- (no args) dump current CC→param mappings\n\n";

  ss << COOL_COLOR_GREEN << "RECORDING\n";
  ss << ANSI_COLOR_WHITE << "  midi_bars(n)          " << COOL_COLOR_ORANGE << "- Set recording length in bars (default 2)\n";
  ss << ANSI_COLOR_WHITE << "  midi_rec()            " << COOL_COLOR_ORANGE << "- Toggle recording; also auto-enables loop\n";
  ss << ANSI_COLOR_WHITE << "  midi_stop()           " << COOL_COLOR_ORANGE << "- Stop MIDI playback\n";
  ss << ANSI_COLOR_WHITE << "  midi_loop()           " << COOL_COLOR_ORANGE << "- Toggle looped playback of recording\n";
  ss << ANSI_COLOR_WHITE << "  midi_reset()          " << COOL_COLOR_ORANGE << "- Clear the recording buffer\n";
  ss << COOL_COLOR_GREEN << "  Keyboard shortcuts (in REC mode):\n";
  ss << ANSI_COLOR_WHITE << "  SPACE                 " << COOL_COLOR_ORANGE << "- Toggle recording on/off (⏺ REC> when active)\n";
  ss << ANSI_COLOR_WHITE << "  ` or ESC              " << COOL_COLOR_ORANGE << "- Exit record/loop mode, return to normal prompt\n\n";

  ss << COOL_COLOR_GREEN << "QUANTIZE\n";
  ss << ANSI_COLOR_WHITE << "  midi_quantize(n)      " << COOL_COLOR_ORANGE << "- Snap recording to n subdivisions/bar (default 16)\n";
  ss << ANSI_COLOR_WHITE << "  midi_fix(midi)        " << COOL_COLOR_ORANGE << "- Hard-snap MidiArray events to 8th-note grid\n\n";

  ss << COOL_COLOR_GREEN << "DATA / EXPORT\n";
  ss << ANSI_COLOR_WHITE << "  midi_dump()           " << COOL_COLOR_ORANGE << "- Return full recording buffer (all bars) as MidiArray\n";
  ss << ANSI_COLOR_WHITE << "  midi2array(midi)      " << COOL_COLOR_ORANGE << "- Convert to [notes_arrs, durs_arrs], one array per bar\n";
  ss << COOL_COLOR_ORANGE << "    // returns [[notes_bar0, notes_bar1, ...], [durs_bar0, durs_bar1, ...]]\n";
  ss << COOL_COLOR_ORANGE << "    // notes_barN = 16-element array of MIDI note numbers (0=rest)\n";
  ss << COOL_COLOR_ORANGE << "    // durs_barN  = 16-element array of durations in MIDI ticks\n";
  ss << ANSI_COLOR_WHITE << "  midi_at(midi, bar)    " << COOL_COLOR_ORANGE << "- Get MIDI note number at bar index (0-based)\n\n";

  ss << COOL_COLOR_GREEN << "CONVERSION UTILS\n";
  ss << ANSI_COLOR_WHITE << "  midi2note(num)        " << COOL_COLOR_ORANGE << "- MIDI number → note name string (e.g. 60 → \"C4\")\n";
  ss << ANSI_COLOR_WHITE << "  midi2freq(num)        " << COOL_COLOR_ORANGE << "- MIDI number → frequency in Hz\n";
  ss << ANSI_COLOR_WHITE << "  freq2midi(freq)       " << COOL_COLOR_ORANGE << "- Frequency → nearest MIDI number\n";
  ss << ANSI_COLOR_WHITE << "  midi_ref()            " << COOL_COLOR_ORANGE << "- Print full note-name to MIDI number table\n\n";

  ss << COOL_COLOR_GREEN << "TYPICAL WORKFLOW\n";
  ss << COOL_COLOR_ORANGE
     << "  midi_init()                      // connect device\n"
     << "  let dx = fmsynth()\n"
     << "  midi_assign(dx)                  // play dx from keyboard\n"
     << "  midi_bars(2)                     // record 2 bars (default)\n"
     << "  midi_rec()                       // start recording (loop auto-enables)\n"
     << "  // play something...             // SPACE toggles rec, ` / ESC to exit\n"
     << "  midi_quantize(16)                // snap to 16th notes\n"
     << "  let m = midi_dump()              // grab all bars as MidiArray\n"
     << "  let arr = midi2array(m)          // [notes_arrays, durs_arrays]\n"
     << "  let bar0_notes = arr[0][0]       // 16-step notes for bar 0\n"
     << "  let bar0_durs  = arr[1][0]       // 16-step durations for bar 0\n\n";

  ss << ANSI_COLOR_RESET;
  return ss.str();
}

// ─── overview ────────────────────────────────────────────────────────────────

static std::string help_overview() {
  std::stringstream ss;
  ss << COOL_COLOR_GREEN << "\n=== SoundB0ard Help ===\n\n" << ANSI_COLOR_RESET;

  ss << COOL_COLOR_GREEN << "Topics:\n";
  ss << ANSI_COLOR_WHITE << "  help functions    " << COOL_COLOR_ORANGE << "- arrays, math, random/generative, music theory, built-ins\n";
  ss << ANSI_COLOR_WHITE << "  help generators   " << COOL_COLOR_ORANGE << "- all sound generators + links to detailed help\n";
  ss << ANSI_COLOR_WHITE << "  help fmsynth      " << COOL_COLOR_ORANGE << "- 4-op FM synth params in full\n";
  ss << ANSI_COLOR_WHITE << "  help subsynth     " << COOL_COLOR_ORANGE << "- subtractive synth params in full\n";
  ss << ANSI_COLOR_WHITE << "  help drumsynth    " << COOL_COLOR_ORANGE << "- drumsynth: kick, snare, hi-hat, clap advanced params\n";
  ss << ANSI_COLOR_WHITE << "  help wavsynth     " << COOL_COLOR_ORANGE << "- wavetable/sample synth params\n";
  ss << ANSI_COLOR_WHITE << "  help looper       " << COOL_COLOR_ORANGE << "- granular looper: grains, FX, multi-buffer, shhh, xfader\n";
  ss << ANSI_COLOR_WHITE << "  help fx           " << COOL_COLOR_ORANGE << "- distort lofi reverb diffuser delay moddelay\n";
  ss << ANSI_COLOR_WHITE << "                      " << COOL_COLOR_ORANGE << "  transverb compressor sculptor gran djeq\n";
  ss << ANSI_COLOR_WHITE << "  help midi         " << COOL_COLOR_ORANGE << "- MIDI: init, assign, record, quantize, CC map, conversion\n";
  ss << ANSI_COLOR_WHITE << "  help mixer        " << COOL_COLOR_ORANGE << "- routing, xfader, scheduling, global sends\n";
  ss << ANSI_COLOR_WHITE << "  help phasor       " << COOL_COLOR_ORANGE << "- phasor signals, factory functions, visualization\n\n";

  ss << COOL_COLOR_GREEN << "Quick Start:\n";
  ss << ANSI_COLOR_WHITE << "  let d = drumsynth();          " << COOL_COLOR_ORANGE << "// drum machine\n";
  ss << ANSI_COLOR_WHITE << "  load_preset(d, \"TR808\");      " << COOL_COLOR_ORANGE << "// load preset\n";
  ss << ANSI_COLOR_WHITE << "  note_on(d, 0);                " << COOL_COLOR_ORANGE << "// trigger kick\n";
  ss << ANSI_COLOR_WHITE << "  let lp = loop(myfile.wav);    " << COOL_COLOR_ORANGE << "// granular looper\n";
  ss << ANSI_COLOR_WHITE << "  let dx = fmsynth();           " << COOL_COLOR_ORANGE << "// FM synth\n";
  ss << ANSI_COLOR_WHITE << "  add_fx(dx, \"reverb\");         " << COOL_COLOR_ORANGE << "// add effect\n";
  ss << ANSI_COLOR_WHITE << "  set dx:o2rat 4.7;             " << COOL_COLOR_ORANGE << "// set parameter\n";
  ss << ANSI_COLOR_WHITE << "  info dx;                      " << COOL_COLOR_ORANGE << "// show all params\n\n";

  ss << COOL_COLOR_GREEN << "Effects (add_fx):\n";
  ss << ANSI_COLOR_WHITE << "  distort  lofi  sculptor  diffuser  reverb  delay  moddelay  compressor  djeq\n\n";

  ss << COOL_COLOR_GREEN << "Commands:\n";
  ss << ANSI_COLOR_WHITE << "  bpm <tempo>    ps    ls    funcz()\n";
  ss << ANSI_COLOR_WHITE << "  vol <gen> <v>  pan <gen> <v>  solo(gen)  unsolo()\n";
  ss << ANSI_COLOR_WHITE << "  p10 # comp_name;   " << COOL_COLOR_ORANGE << "- Assign computation to process\n";
  ss << ANSI_COLOR_WHITE << "  monitor(\"file.sb\")  " << COOL_COLOR_ORANGE << "- Live-reload script\n\n";

  ss << ANSI_COLOR_RESET;
  return ss.str();
}

// ─── dispatch ────────────────────────────────────────────────────────────────

std::string build_help(const std::string& topic) {
  if (topic.empty())              return help_overview();
  if (topic == "functions"
   || topic == "func"
   || topic == "builtins")        return help_functions();
  if (topic == "generators"
   || topic == "gen")             return help_generators();
  if (topic == "synths")          return help_generators();
  if (topic == "fmsynth"
   || topic == "fm")              return help_fmsynth();
  if (topic == "subsynth"
   || topic == "sub")             return help_subsynth();
  if (topic == "looper"
   || topic == "loop"
   || topic == "granular")        return help_looper();
  if (topic == "drums"
   || topic == "drum"
   || topic == "drumsynth")       return help_drums();
  if (topic == "wavsynth"
   || topic == "wav")             return help_wavsynth();
  if (topic == "fx"
   || topic == "effects"
   || topic == "distort"
   || topic == "reverb"
   || topic == "delay")           return help_fx();
  if (topic == "midi")             return help_midi();
  if (topic == "mixer"
   || topic == "mix"
   || topic == "routing")         return help_mixer();
  if (topic == "phasor"
   || topic == "viz"
   || topic == "visual")          return help_phasor();

  // Unknown topic — show overview with a note
  std::stringstream ss;
  ss << COOL_COLOR_ORANGE << "\nUnknown help topic: '" << topic << "'\n" << ANSI_COLOR_RESET;
  return ss.str() + help_overview();
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
