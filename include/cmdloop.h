#pragma once

#include <stdbool.h>

#include <interpreter/object.hpp>
#include <memory>

#include <lo/lo.h>

#include "audioutils.h"
#include "digisynth.h"
#include "dxsynth.h"
#include "minisynth.h"

void *loopy(void *arg);
// void interpret(char *line);

void Interpret(char *line, std::shared_ptr<object::Environment> env);

int stacksize(void);

int exxit(void);
int parse_wurds_from_cmd(char wurds[][SIZE_OF_WURD], char *line);
bool parse_minisynth_settings_change(MiniSynth *ms, char wurds[][SIZE_OF_WURD]);
bool parse_dxsynth_settings_change(dxsynth *ms, char wurds[][SIZE_OF_WURD]);
bool parse_digisynth_settings_change(digisynth *ms, char wurds[][SIZE_OF_WURD]);

bool is_valid_file(char *filename);

int generic_osc_handler(const char *path, const char *types, lo_arg **argv,
                        int argc, void *data, void *user_data);
int trigger_osc_handler(const char *path, const char *types, lo_arg **argv,
                        int argc, void *data, void *user_data);
int osc_note_on_handler(const char *path, const char *types, lo_arg **argv,
                        int argc, void *data, void *user_data);
