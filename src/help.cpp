#include "help.h"

#include <iostream>

#include "defjams.h"

void print_help() {
  // clang-format off
  std::cout << COOL_COLOR_GREEN << "\n=== SoundB0ard Help ===\n\n" << ANSI_COLOR_RESET;

  std::cout << COOL_COLOR_GREEN << "Documentation:\n";
  std::cout << ANSI_COLOR_WHITE  << "  USER_GUIDE.md    " << COOL_COLOR_ORANGE << "- Complete user guide with examples\n";
  std::cout << ANSI_COLOR_WHITE  << "  FX_REFERENCE.md  " << COOL_COLOR_ORANGE << "- Quick reference for all effects\n\n";

  std::cout << COOL_COLOR_GREEN << "Quick Start:\n";
  std::cout << ANSI_COLOR_WHITE  << "  let drums = drum();              " << COOL_COLOR_ORANGE << "// Create drum machine\n";
  std::cout << ANSI_COLOR_WHITE  << "  load_preset(drums, \"TR808\");     " << COOL_COLOR_ORANGE << "// Load preset\n";
  std::cout << ANSI_COLOR_WHITE  << "  note_on(drums, 0);               " << COOL_COLOR_ORANGE << "// Trigger kick\n";
  std::cout << ANSI_COLOR_WHITE  << "  add_fx(drums, \"distort\");              " << COOL_COLOR_ORANGE << "// Add distortion\n";
  std::cout << ANSI_COLOR_WHITE  << "  set drums:fx0:mode 2;" << COOL_COLOR_ORANGE << "// Tube saturation\n\n";

  std::cout << COOL_COLOR_GREEN << "Sound Generators:\n";
  std::cout << ANSI_COLOR_WHITE  << "  drum()       " << COOL_COLOR_ORANGE << "- Drum synthesizer (9 voices)\n";
  std::cout << ANSI_COLOR_WHITE  << "  minisynth()  " << COOL_COLOR_ORANGE << "- Subtractive synthesizer\n";
  std::cout << ANSI_COLOR_WHITE  << "  fmsynth()    " << COOL_COLOR_ORANGE << "- FM synthesizer (4 operators)\n";
  std::cout << ANSI_COLOR_WHITE  << "  loop(<sample_name>)   " << COOL_COLOR_ORANGE << "- Granular sampler\n";
  std::cout << ANSI_COLOR_WHITE  << "  sample(path) " << COOL_COLOR_ORANGE << "- Load audio sample\n\n";

  std::cout << COOL_COLOR_GREEN << "Effects:\n";
  std::cout << ANSI_COLOR_WHITE  << "  distort    " << COOL_COLOR_ORANGE << "- Multi-mode distortion (4 algorithms)\n";
  std::cout << ANSI_COLOR_WHITE  << "  lofi       " << COOL_COLOR_ORANGE << "- Lo-fi crusher (bit depth + sample rate)\n";
  std::cout << ANSI_COLOR_WHITE  << "  sculptor   " << COOL_COLOR_ORANGE << "- Waveform manipulation\n";
  std::cout << ANSI_COLOR_WHITE  << "  diffuser   " << COOL_COLOR_ORANGE << "- Multi-stage diffusion\n";
  std::cout << ANSI_COLOR_WHITE  << "  reverb     " << COOL_COLOR_ORANGE << "- Reverb\n";
  std::cout << ANSI_COLOR_WHITE  << "  delay      " << COOL_COLOR_ORANGE << "- Stereo delay\n";
  std::cout << ANSI_COLOR_WHITE  << "  moddelay   " << COOL_COLOR_ORANGE << "- Modulated delay (chorus/flanger)\n";
  std::cout << ANSI_COLOR_WHITE  << "  compressor " << COOL_COLOR_ORANGE << "- Dynamics compression\n\n";

  std::cout << COOL_COLOR_GREEN << "Commands:\n";
  std::cout << ANSI_COLOR_WHITE  << "  bpm <tempo>        " << COOL_COLOR_ORANGE << "- Set tempo\n";
  std::cout << ANSI_COLOR_WHITE  << "  monitor(\"file.sb\")    " << COOL_COLOR_ORANGE << "- Import and monitor script file\n";
  std::cout << ANSI_COLOR_WHITE  << "  info(<instrument_name>)  " << COOL_COLOR_ORANGE << "- Show all parameters\n";
  std::cout << ANSI_COLOR_WHITE  << "  list_presets(<instrument_name>)    " << COOL_COLOR_ORANGE << "- List available presets\n";
  std::cout << ANSI_COLOR_WHITE  << "  p10 # comp_name;  " << COOL_COLOR_ORANGE << "- Assign computation to process\n\n";

  std::cout << COOL_COLOR_GREEN
            << "========================================================================\n"
            << "                    Built-In Functions Reference\n"
            << "========================================================================\n\n";

  std::cout << COOL_COLOR_GREEN << "ARRAY/COLLECTION FUNCTIONS\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  std::cout << ANSI_COLOR_WHITE << "  len(arr|str|map)      " << COOL_COLOR_ORANGE << "- Returns the length/size\n";
  std::cout << ANSI_COLOR_WHITE << "  head(arr)             " << COOL_COLOR_ORANGE << "- Returns the first element\n";
  std::cout << ANSI_COLOR_WHITE << "  tail(arr)             " << COOL_COLOR_ORANGE << "- Returns all elements except the first\n";
  std::cout << ANSI_COLOR_WHITE << "  last(arr)             " << COOL_COLOR_ORANGE << "- Returns the last element\n";
  std::cout << ANSI_COLOR_WHITE << "  push(arr, item)       " << COOL_COLOR_ORANGE << "- Appends item to array\n";
  std::cout << ANSI_COLOR_WHITE << "  take_n(arr, n)        " << COOL_COLOR_ORANGE << "- Returns first n elements\n";
  std::cout << ANSI_COLOR_WHITE << "  take_random_n(arr, n) " << COOL_COLOR_ORANGE << "- Returns n random unique elements\n";
  std::cout << ANSI_COLOR_WHITE << "  reverse(arr)          " << COOL_COLOR_ORANGE << "- Reverses the order\n";
  std::cout << ANSI_COLOR_WHITE << "  rotate(arr, n)        " << COOL_COLOR_ORANGE << "- Rotates elements by n positions\n";
  std::cout << ANSI_COLOR_WHITE << "  sort(arr)             " << COOL_COLOR_ORANGE << "- Sorts in ascending order\n";
  std::cout << ANSI_COLOR_WHITE << "  shuffle(arr)          " << COOL_COLOR_ORANGE << "- Randomly shuffles elements\n";
  std::cout << ANSI_COLOR_WHITE << "  scramble(arr)         " << COOL_COLOR_ORANGE << "- Randomly scramble a 16-step rhythm (keeps active beats)\n";
  std::cout << ANSI_COLOR_WHITE << "  stutter(arr)          " << COOL_COLOR_ORANGE << "- Create stutter effect on a step pattern\n";
  std::cout << ANSI_COLOR_WHITE << "  is_array(val)         " << COOL_COLOR_ORANGE << "- Returns true if value is an array\n";
  std::cout << ANSI_COLOR_WHITE << "  is_in(arr, item)      " << COOL_COLOR_ORANGE << "- Returns true if item is found\n";
  std::cout << ANSI_COLOR_WHITE << "  keys(map)             " << COOL_COLOR_ORANGE << "- Returns array of all keys\n";
  std::cout << ANSI_COLOR_WHITE << "  invert(arr)           " << COOL_COLOR_ORANGE << "- Inverts 0s and 1s (for rhythms)\n";
  std::cout << ANSI_COLOR_WHITE << "  bits(num)             " << COOL_COLOR_ORANGE << "- Convert integer to array of bits\n";
  std::cout << ANSI_COLOR_WHITE << "  hex(num)              " << COOL_COLOR_ORANGE << "- Convert integer to hex string\n\n";

  std::cout << COOL_COLOR_GREEN << "MATH FUNCTIONS\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  std::cout << ANSI_COLOR_WHITE << "  floor(num)            " << COOL_COLOR_ORANGE << "- Returns largest integer <= num\n";
  std::cout << ANSI_COLOR_WHITE << "  abs(num)              " << COOL_COLOR_ORANGE << "- Returns absolute value\n";
  std::cout << ANSI_COLOR_WHITE << "  log(num)              " << COOL_COLOR_ORANGE << "- Returns log2 of num\n";
  std::cout << ANSI_COLOR_WHITE << "  sin(num), cos(num)    " << COOL_COLOR_ORANGE << "- Trig functions (radians)\n";
  std::cout << ANSI_COLOR_WHITE << "  max(a, b), min(a, b)  " << COOL_COLOR_ORANGE << "- Returns larger/smaller value\n";
  std::cout << ANSI_COLOR_WHITE << "  incr(num, min, max)   " << COOL_COLOR_ORANGE << "- Increments, wrapping at max\n";
  std::cout << ANSI_COLOR_WHITE << "  rincr(num, min, max)  " << COOL_COLOR_ORANGE << "- Decrements, wrapping at min\n";
  std::cout << ANSI_COLOR_WHITE << "  dincr(num, min, max)  " << COOL_COLOR_ORANGE << "- Drunk walk (+1 or -1 randomly)\n";
  std::cout << ANSI_COLOR_WHITE << "  scale(val, in_min, in_max, out_min, out_max) " << COOL_COLOR_ORANGE << "- Map range\n\n";

  std::cout << COOL_COLOR_GREEN << "RANDOM/GENERATIVE FUNCTIONS\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  std::cout << ANSI_COLOR_WHITE << "  rand(min, max)             " << COOL_COLOR_ORANGE << "- Random number in range\n";
  std::cout << ANSI_COLOR_WHITE << "  rand_array(size, min, max) " << COOL_COLOR_ORANGE << "- Array of random numbers\n";
  std::cout << ANSI_COLOR_WHITE << "  rand_sixteenthz(n)         " << COOL_COLOR_ORANGE << "- Array of n random unique 16th-note positions\n";
  std::cout << ANSI_COLOR_WHITE << "  perlin(x)                  " << COOL_COLOR_ORANGE << "- Perlin noise value (smooth random)\n";
  std::cout << ANSI_COLOR_WHITE << "  bjork(pulses, steps)       " << COOL_COLOR_ORANGE << "- Euclidean rhythm pattern\n";
  std::cout << ANSI_COLOR_WHITE << "  gen_perc(len, density)     " << COOL_COLOR_ORANGE << "- Random percussion pattern\n\n";

  std::cout << COOL_COLOR_GREEN << "MUSIC THEORY FUNCTIONS\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  std::cout << ANSI_COLOR_WHITE << "  midi_ref()                       " << COOL_COLOR_ORANGE << "- Map of note names to MIDI numbers\n";
  std::cout << ANSI_COLOR_WHITE << "  notes_in_key(root, scale)        " << COOL_COLOR_ORANGE << "- MIDI notes in key/scale\n";
  std::cout << ANSI_COLOR_WHITE << "  notes_in_chord(root, type)       " << COOL_COLOR_ORANGE << "- MIDI notes in chord\n";
  std::cout << ANSI_COLOR_WHITE << "  scale_note(note, key, scale)     " << COOL_COLOR_ORANGE << "- Quantize note to scale\n";
  std::cout << ANSI_COLOR_WHITE << "  scale_melody(notes, root, scale) " << COOL_COLOR_ORANGE << "- Quantize melody array to scale\n";
  std::cout << ANSI_COLOR_WHITE << "  algoz()                          " << COOL_COLOR_ORANGE << "- Available scale/mode names\n";
  std::cout << ANSI_COLOR_WHITE << "  ratioz()                         " << COOL_COLOR_ORANGE << "- Available chord types\n\n";

  std::cout << COOL_COLOR_GREEN << "SOUND GENERATOR CONTROL\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  std::cout << ANSI_COLOR_WHITE << "  note_on(gen, note, vel)              " << COOL_COLOR_ORANGE << "- Trigger note\n";
  std::cout << ANSI_COLOR_WHITE << "  note_on_at(gen, note, time, vel=, dur=) " << COOL_COLOR_ORANGE << "- Schedule note\n";
  std::cout << ANSI_COLOR_WHITE << "  note_off(gen, note)                  " << COOL_COLOR_ORANGE << "- Stop note\n";
  std::cout << ANSI_COLOR_WHITE << "  note_off_at(gen, note, time)         " << COOL_COLOR_ORANGE << "- Schedule note off\n";
  std::cout << ANSI_COLOR_WHITE << "  set_pitch(gen, pitch)                " << COOL_COLOR_ORANGE << "- Set generator pitch\n";
  std::cout << ANSI_COLOR_WHITE << "  stop(gen)                            " << COOL_COLOR_ORANGE << "- Stop all notes\n";
  std::cout << ANSI_COLOR_WHITE << "  solo(gen), unsolo()                  " << COOL_COLOR_ORANGE << "- Solo/unsolo generator\n";
  std::cout << ANSI_COLOR_WHITE << "  stepn(seq)                           " << COOL_COLOR_ORANGE << "- Advance step sequencer and return next value\n";
  std::cout << ANSI_COLOR_WHITE << "  signal_from(phasor)                  " << COOL_COLOR_ORANGE << "- Get current signal value from a phasor\n\n";

  std::cout << COOL_COLOR_GREEN << "PLAYBACK FUNCTIONS\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  std::cout << ANSI_COLOR_WHITE << "  play_array(gen, notes)           " << COOL_COLOR_ORANGE << "- Play array of notes (1 per beat)\n";
  std::cout << ANSI_COLOR_WHITE << "  play_array_over(gen, notes, spd) " << COOL_COLOR_ORANGE << "- Play notes stretched over 'spd' beats\n";
  std::cout << ANSI_COLOR_WHITE << "  fast(gen, notes, speed)          " << COOL_COLOR_ORANGE << "- Play notes at given speed\n";
  std::cout << ANSI_COLOR_WHITE << "  play_rhythm(rhythm_map)          " << COOL_COLOR_ORANGE << "- Play a rhythm map {gen -> notes}\n";
  std::cout << ANSI_COLOR_WHITE << "  play_map(gen, map)               " << COOL_COLOR_ORANGE << "- Play a note map on generator\n\n";

  std::cout << COOL_COLOR_GREEN << "SAMPLE/INSTRUMENT LOADING\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  std::cout << ANSI_COLOR_WHITE << "  load_dir(dirname)    " << COOL_COLOR_ORANGE << "- Load all samples from wavs/dirname as map\n";
  std::cout << ANSI_COLOR_WHITE << "  add_buf(gen, path)   " << COOL_COLOR_ORANGE << "- Add audio buffer/sample to generator\n";
  std::cout << ANSI_COLOR_WHITE << "  drum_kit()           " << COOL_COLOR_ORANGE << "- Show default drum kit sample mappings\n";
  std::cout << ANSI_COLOR_WHITE << "  kit()                " << COOL_COLOR_ORANGE << "- Quick-start: create drums + FM synth setup\n\n";

  std::cout << COOL_COLOR_GREEN << "PATTERN FUNCTIONS\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  std::cout << ANSI_COLOR_WHITE << "  eval_pattern(pattern)  " << COOL_COLOR_ORANGE << "- Evaluate a pattern to an array\n";
  std::cout << ANSI_COLOR_WHITE << "  print_pattern(pattern) " << COOL_COLOR_ORANGE << "- Print pattern and return step sequence\n\n";

  std::cout << COOL_COLOR_GREEN << "STRING FUNCTIONS\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  std::cout << ANSI_COLOR_WHITE << "  lowercase(str) " << COOL_COLOR_ORANGE << "- Convert string to lowercase\n";
  std::cout << ANSI_COLOR_WHITE << "  uppercase(str) " << COOL_COLOR_ORANGE << "- Convert string to uppercase\n\n";

  std::cout << COOL_COLOR_GREEN << "PRESET MANAGEMENT\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  std::cout << ANSI_COLOR_WHITE << "  list_presets(gen)      " << COOL_COLOR_ORANGE << "- List available presets\n";
  std::cout << ANSI_COLOR_WHITE << "  load_preset(gen, name) " << COOL_COLOR_ORANGE << "- Load named preset\n";
  std::cout << ANSI_COLOR_WHITE << "  rand_preset(gen)       " << COOL_COLOR_ORANGE << "- Load random preset\n";
  std::cout << ANSI_COLOR_WHITE << "  save_preset(gen, name) " << COOL_COLOR_ORANGE << "- Save current settings\n\n";

  std::cout << COOL_COLOR_GREEN << "MIXER/ROUTING FUNCTIONS\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  std::cout << ANSI_COLOR_WHITE << "  send(effect, gen)    " << COOL_COLOR_ORANGE << "- Send generator to effect\n";
  std::cout << ANSI_COLOR_WHITE << "  add_fx(gen, fx_name) " << COOL_COLOR_ORANGE << "- Add effect to generator\n";
  std::cout << ANSI_COLOR_WHITE << "  mvol(volume)         " << COOL_COLOR_ORANGE << "- Set master volume (0.0-1.0)\n";
  std::cout << ANSI_COLOR_WHITE << "  monitor(filepath)    " << COOL_COLOR_ORANGE << "- Monitor file for live coding\n\n";

  std::cout << COOL_COLOR_GREEN << "CROSSFADER FUNCTIONS\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  std::cout << ANSI_COLOR_WHITE << "  xassign(ch, gen)   " << COOL_COLOR_ORANGE << "- Assign generator to xfader channel (0=L, 1=R)\n";
  std::cout << ANSI_COLOR_WHITE << "  xremove(ch, gen)   " << COOL_COLOR_ORANGE << "- Remove generator from xfader channel\n";
  std::cout << ANSI_COLOR_WHITE << "  xclear()           " << COOL_COLOR_ORANGE << "- Clear all xfader assignments\n";
  std::cout << ANSI_COLOR_WHITE << "  xfade(direction)   " << COOL_COLOR_ORANGE << "- Crossfade toward LEFT(0) or RIGHT(1)\n\n";

  std::cout << COOL_COLOR_GREEN << "SCHEDULING\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  std::cout << ANSI_COLOR_WHITE << "  sched(delay, start, end, time, action) " << COOL_COLOR_ORANGE << "- Parameter automation\n\n";

  std::cout << COOL_COLOR_GREEN << "MIDI FUNCTIONS\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  std::cout << ANSI_COLOR_WHITE << "  midi_init()           " << COOL_COLOR_ORANGE << "- Initialize MIDI system\n";
  std::cout << ANSI_COLOR_WHITE << "  midi_assign(gen)      " << COOL_COLOR_ORANGE << "- Assign generator to MIDI controller\n";
  std::cout << ANSI_COLOR_WHITE << "  midi_map(id, param)   " << COOL_COLOR_ORANGE << "- Map MIDI CC to a parameter\n";
  std::cout << ANSI_COLOR_WHITE << "  midi_rec()            " << COOL_COLOR_ORANGE << "- Toggle MIDI recording on/off\n";
  std::cout << ANSI_COLOR_WHITE << "  midi_print()          " << COOL_COLOR_ORANGE << "- Toggle MIDI debug printing\n";
  std::cout << ANSI_COLOR_WHITE << "  midi_reset()          " << COOL_COLOR_ORANGE << "- Clear MIDI recording buffer\n";
  std::cout << ANSI_COLOR_WHITE << "  midi_dump()           " << COOL_COLOR_ORANGE << "- Dump recording buffer to MidiArray\n";
  std::cout << ANSI_COLOR_WHITE << "  midi2array(midi)      " << COOL_COLOR_ORANGE << "- Convert MidiArray to 16-step note array\n";
  std::cout << ANSI_COLOR_WHITE << "  midi_at(midi, n)      " << COOL_COLOR_ORANGE << "- Get notes at beat n from MidiArray\n";
  std::cout << ANSI_COLOR_WHITE << "  midi_fix(midi)        " << COOL_COLOR_ORANGE << "- Quantize MidiArray timing to 8th-note grid\n\n";

  std::cout << COOL_COLOR_GREEN << "UTILITY\n"
            << ANSI_COLOR_WHITE << "------------------------------------------------------------------------\n";
  std::cout << ANSI_COLOR_WHITE << "  print(val)      " << COOL_COLOR_ORANGE << "- Print value to console\n";
  std::cout << ANSI_COLOR_WHITE << "  funcz()         " << COOL_COLOR_ORANGE << "- List environment variables\n";
  std::cout << ANSI_COLOR_WHITE << "  type(val)       " << COOL_COLOR_ORANGE << "- Returns the type of value\n";
  std::cout << ANSI_COLOR_WHITE << "  now()           " << COOL_COLOR_ORANGE << "- Returns current audio tick\n";
  std::cout << ANSI_COLOR_WHITE << "  timing_info()   " << COOL_COLOR_ORANGE << "- Print audio timing/sync info\n";
  std::cout << ANSI_COLOR_WHITE << "  synchz(n)       " << COOL_COLOR_ORANGE << "- Hz per timing unit at quantize n (2/4/8/16/32)\n";
  std::cout << ANSI_COLOR_WHITE << "  websock(bool)   " << COOL_COLOR_ORANGE << "- Enable/disable websocket interface\n\n";

  std::cout << COOL_COLOR_GREEN << "For more help:\n";
  std::cout << ANSI_COLOR_WHITE << "  Type " << COOL_COLOR_YELLOW << "'help'" << COOL_COLOR_ORANGE << " to see this again\n";
  std::cout << ANSI_COLOR_WHITE << "  Type " << COOL_COLOR_YELLOW << "'funcz()'" << COOL_COLOR_ORANGE << " to see current environment\n";
  std::cout << ANSI_COLOR_WHITE << "  Use "  << COOL_COLOR_YELLOW << "info(instrument)" << COOL_COLOR_ORANGE << " to see all parameters\n";
  std::cout << ANSI_COLOR_RESET << "\n";
  // clang-format on
}
