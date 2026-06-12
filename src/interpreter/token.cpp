#include <interpreter/token.hpp>
#include <iostream>
#include <string>
#include <unordered_map>

namespace token {
const std::unordered_map<std::string, TokenType> keywords{
    {"at", SLANG_AT},
    {"delay", SLANG_FX_DELAY},
    {"reverb", SLANG_FX_REVERB},
    {"distort", SLANG_FX_DISTORT},
    {"lofi", SLANG_FX_LOFI},
    {"sculptor", SLANG_FX_SCULPTOR},
    {"diffuser", SLANG_FX_DIFFUSER},
    {"moddelay", SLANG_FX_MODDELAY},
    {"compressor", SLANG_FX_COMPRESSOR},
    {"bitop", SLANG_BITOP},
    {"break", SLANG_BREAK},
    {"bpm", SLANG_BPM},
    {"digi", SLANG_DIGI_SYNTH},
    {"drumsynth", SLANG_DRUM_SYNTH},
    {"dur", SLANG_DURATION},
    {"else", SLANG_ELSE},
    {"every", SLANG_EVERY},
    {"false", SLANG_FALSE},
    {"fm", SLANG_FM_SYNTH},
    {"fmsynth", SLANG_FM_SYNTH},
    {"sbsynth", SLANG_SB_SYNTH},
    {"fn", SLANG_FUNCTION},
    {"for", SLANG_FOR},
    {"comp", SLANG_COMPUTATION},
    {"grain", SLANG_GRAIN},
    {"granular", SLANG_GRANULAR},
    {"gran", SLANG_GRANULAR},
    {"help", SLANG_HELP},
    {"if", SLANG_IF},
    {"info", SLANG_INFO},
    {"let", SLANG_LET},
    {"loop", SLANG_LOOP},
    {"ls", SLANG_LS},
    {"midi_array", SLANG_MIDI_ARRAY},
    {"minisynth", SLANG_MOOG_SYNTH},
    {"moog", SLANG_MOOG_SYNTH},
    {"NULL", SLANG_NULL},
    {"osc", SLANG_OSC},
    {"over", SLANG_OVER},
    {"pattern", SLANG_PATTERN},
    {"pan", SLANG_PAN},
    {"phasor", SLANG_PHASOR},
    {"play", SLANG_PLAY},
    {"proc", SLANG_PROC},
    {"ps", SLANG_PS},
    {"ramp", SLANG_RAMP},
    {"return", SLANG_RETURN},
    {"run", SLANG_COMPUTATION_RUN},
    {"sample", SLANG_SAMPLE},
    {"set", SLANG_SET},
    {"setup", SLANG_COMPUTATION_SETUP},
    {"signal_generator", SLANG_SIGNAL_GENERATOR},
    {"sseq", SLANG_STEP_SEQUENCER},
    {"strategy", SLANG_STRATEGY},
    {"true", SLANG_TRUE},
    {"vel", SLANG_VELOCITY},
    {"row", SLANG_ROW},
    {"vol", SLANG_VOLUME},
};

TokenType LookupIdent(std::string ident) {
  std::unordered_map<std::string, TokenType>::const_iterator got =
      keywords.find(ident);
  if (got != keywords.end()) {
    return got->second;
  }

  return SLANG_IDENT;
}

bool IsKeyword(const std::string &ident) {
  return keywords.find(ident) != keywords.end();
}

std::ostream &operator<<(std::ostream &out, const Token &tok) {
  out << "{type:" << tok.type_ << " literal:" << tok.literal_ << "}";
  return out;
}

}  // namespace token
