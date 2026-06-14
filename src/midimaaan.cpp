#include <midimaaan.h>

#include <cstring>
#include <iostream>

midi_event new_midi_event(int event_type, int data1, int data2) {
  midi_event ev = {.event_type = event_type, .data1 = data1, .data2 = data2};
  return ev;
}

std::ostream &operator<<(std::ostream &out, const midi_event &ev) {
  out << "MIDIEVENT:"
      << "Type:" << ev.event_type << " D1:" << ev.data1 << " D2:" << ev.data2;
  return out;
}

int get_midi_note_from_string(char *string) {
  if (strlen(string) > 4) {
    printf("DINGIE!\n");
    return -1;
  }
  char note[3] = {0};
  int octave = 0;
  sscanf(string, "%2[a-z#]%d", note, &octave);
  note[2] = 0;

  octave = 12 + (octave * 12);

  int midinotenum = -1;
  if (!strcasecmp("c", note))
    midinotenum = 0 + octave;
  else if (!strcasecmp("c#", note) || !strcasecmp("db", note) ||
           !strcasecmp("dm", note))
    midinotenum = 1 + octave;
  else if (!strcasecmp("d", note))
    midinotenum = 2 + octave;
  else if (!strcasecmp("d#", note) || !strcasecmp("eb", note) ||
           !strcasecmp("em", note))
    midinotenum = 3 + octave;
  else if (!strcasecmp("e", note))
    midinotenum = 4 + octave;
  else if (!strcasecmp("f", note))
    midinotenum = 5 + octave;
  else if (!strcasecmp("f#", note) || !strcasecmp("gb", note) ||
           !strcasecmp("gm", note))
    midinotenum = 6 + octave;
  else if (!strcasecmp("g", note))
    midinotenum = 7 + octave;
  else if (!strcasecmp("g#", note) || !strcasecmp("ab", note) ||
           !strcasecmp("am", note))
    midinotenum = 8 + octave;
  else if (!strcasecmp("a", note))
    midinotenum = 9 + octave;
  else if (!strcasecmp("a#", note) || !strcasecmp("bb", note) ||
           !strcasecmp("bm", note))
    midinotenum = 10 + octave;
  else if (!strcasecmp("b", note))
    midinotenum = 11 + octave;

  return midinotenum;
}

int get_midi_note_from_mixer_key(unsigned int key, int octave) {
  int midi_octave = 12 + (octave * 12);
  return key + midi_octave;
}
