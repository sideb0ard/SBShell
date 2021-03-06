#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

#include <iostream>

#include <portaudio.h>

#include <ableton_link_wrapper.h>
#include <audio_action_queue.h>
#include <defjams.h>
#include <digisynth.h>
#include <drumsampler.h>
#include <drumsynth.h>
#include <dxsynth.h>
#include <event_queue.h>
#include <filereader.hpp>
#include <fx/envelope.h>
#include <fx/fx.h>
#include <interpreter/object.hpp>
#include <interpreter/sound_cmds.hpp>
#include <looper.h>
#include <minisynth.h>
#include <mixer.h>
#include <obliquestrategies.h>
#include <soundgenerator.h>
#include <tsqueue.hpp>
#include <utils.h>

mixer *mixr;
extern std::shared_ptr<object::Environment> global_env;
extern Tsqueue<event_queue_item> process_event_queue;
extern Tsqueue<std::string> repl_queue;
extern Tsqueue<audio_action_queue_item> audio_queue;
extern Tsqueue<int> audio_reply_queue;
extern Tsqueue<std::string> interpret_command_queue;

const char *key_names[] = {"C", "C_SHARP", "D", "D_SHARP", "E", "F", "F_SHARP",
                           "G", "G_SHARP", "A", "A_SHARP", "B"};

const char *chord_type_names[] = {"MAJOR", "MINOR", "DIMINISHED"};

static const char *s_progressions[NUM_PROGRESSIONS] = {
    "I-IV-V", "I-V-vi-IV", "I-vi-IV-V", "vi-ii-V-I"};

const wchar_t *s_status_colors[] = {
    WCOOL_COLOR_PINK,      // MINISYNTH_TYPE
    WCOOL_COLOR_ORANGE,    // DIGISYNTH_TYPE
    WCOOL_COLOR_MAUVE,     // LOOPER_TYPE
    WCOOL_COLOR_YELLOW,    // BITWIZE_TYPE
    WANSI_COLOR_DEEP_RED,  // LOOPER_TYPE
    WANSI_COLOR_GREEN_TOO, // DRUMSAMPLER_TYPE
    WANSI_COLOR_MAGENTA,   // DRUMSYNTH_TYPE
    WANSI_COLOR_CYAN,      // ALGORITHM_TYPE
    WANSI_COLOR_GREEN,     // CHAOSMONKEY_TYPE
    WANSI_COLOR_BLUE       //
};

const char *s_midi_control_type_name[] = {"NONE", "SYNTH", "DRUMSYNTH"};

const char *s_sg_names[] = {"MOOG", "DIGI", "DX", "LOOP", "STEP", "STEP"};

mixer *new_mixer(double output_latency)
{
    mixer *mixr = (mixer *)calloc(1, sizeof(mixer));
    if (mixr == NULL)
    {
        printf("Nae mixer, fucked up!\n");
        return NULL;
    }
    mixr->m_ableton_link = new_ableton_link(DEFAULT_BPM);
    if (!mixr->m_ableton_link)
    {
        printf("Something fucked up with yer Ableton link, mate."
               " ye wanna get that seen tae\n");
        return NULL;
    }
    link_set_latency(mixr->m_ableton_link, output_latency);

    mixr->volume = 0.7;
    mixer_update_bpm(mixr, DEFAULT_BPM);
    mixr->m_midi_controller_mode =
        KEY_MODE_ONE; // dunno whether this should be on mixer or synth
    mixr->midi_control_destination = NONE;

    for (int i = 0; i < MAX_NUM_PROC; i++)
        mixr->processes_[i] = std::make_shared<Process>();
    mixr->proc_initialized_ = true;

    // the lifetime of these booleans is a single sample

    mixr->timing_info.cur_sample = -1;
    mixr->timing_info.midi_tick = -1;
    mixr->timing_info.sixteenth_note_tick = -1;
    mixr->timing_info.loop_beat = 0;
    mixr->timing_info.time_of_next_midi_tick = 0;
    mixr->timing_info.has_started = false;
    mixr->timing_info.is_midi_tick = true;
    mixr->timing_info.is_start_of_loop = true;
    mixr->timing_info.is_thirtysecond = true;
    mixr->timing_info.is_twentyfourth = true;
    mixr->timing_info.is_sixteenth = true;
    mixr->timing_info.is_twelth = true;
    mixr->timing_info.is_eighth = true;
    mixr->timing_info.is_sixth = true;
    mixr->timing_info.is_quarter = true;
    mixr->timing_info.is_third = true;

    mixr->active_midi_soundgen_num = -99;

    mixr->timing_info.key = C;
    mixr->timing_info.octave = 3;
    mixer_set_notes(mixr);
    mixer_set_chord_progression(mixr, 1);
    mixr->bars_per_chord = 4;
    mixr->should_progress_chords = false;

    std::string contents = ReadFileContents(kStartupConfigFile);
    interpret_command_queue.push(contents);

    return mixr;
}

std::string mixer_status_mixr(mixer *mixr)
{
    LinkData data = link_get_timing_data_for_display(mixr->m_ableton_link);
    // clang-format off
    std::stringstream ss;
    ss << COOL_COLOR_GREEN << "\n"
    << "::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n"
    << ":::::::::: vol:" << ANSI_COLOR_WHITE << mixr->volume << COOL_COLOR_GREEN
    << " bpm:" << ANSI_COLOR_WHITE <<data.tempo << COOL_COLOR_GREEN
    << " quantum:" << ANSI_COLOR_WHITE << data.quantum << COOL_COLOR_GREEN
    << " beat:" << ANSI_COLOR_WHITE << data.beat << COOL_COLOR_GREEN
    << " phase:" << ANSI_COLOR_WHITE << data.phase << COOL_COLOR_GREEN
    << " num_peers:" << ANSI_COLOR_WHITE << data.num_peers << COOL_COLOR_GREEN
    << "::::::::::\n"
    << ":::::::::: key:" << key_names[mixr->timing_info.key]
    << " chord:" << key_names[mixr->timing_info.chord] // TODO - fix - this is wrong - its not the chord its the chord progression index
    << " type:"<< chord_type_names[mixr->timing_info.chord_type]
    << " octave:" << mixr->timing_info.octave << " bars_per_chord:"<< mixr->bars_per_chord
    << " move:" << mixr->should_progress_chords << " prog:("<< mixr->progression_type << ")"
    << s_progressions[mixr->progression_type] << "\n";
    // clang-format on

    return ss.str();
}

std::string mixer_status_procz(mixer *mixr, bool all)
{
    std::stringstream ss;
    ss << COOL_COLOR_ORANGE << "\n[" << ANSI_COLOR_WHITE << "Procz"
       << COOL_COLOR_ORANGE << "]\n";

    for (int i = 0; i < MAX_NUM_PROC; i++)
    {
        auto &p = mixr->processes_[i];
        if (p->active_ || all)
        {
            ss << ANSI_COLOR_WHITE << "p" << i << ANSI_COLOR_RESET << " "
               << p->Status() << std::endl;
        }
    }

    return ss.str();
}

std::string mixer_status_env(mixer *mixr)
{
    std::stringstream ss;
    ss << COOL_COLOR_GREEN << "\n[" << ANSI_COLOR_WHITE << "Env"
       << COOL_COLOR_GREEN << "]" << std::endl;

    ss << global_env->Debug();

    std::unordered_map<std::string, int> soundgens =
        global_env->GetSoundGenerators();
    for (auto &[var_name, sg_idx] : soundgens)
    {
        if (mixer_is_valid_soundgen_num(mixr, sg_idx))
        {
            auto sg = mixr->sound_generators_[sg_idx];
            if (!sg->active)
                continue;
            ss << ANSI_COLOR_WHITE << var_name << ANSI_COLOR_RESET " = "
               << sg->Status() << ANSI_COLOR_RESET << std::endl;

            std::stringstream margin;
            for (auto c : var_name)
                margin << " ";
            margin << "   "; // for the ' = '

            for (int i = 0; i < sg->effects_num; i++)
            {
                ss << margin.str();
                Fx *f = sg->effects[i];
                if (f->enabled_)
                    ss << COOL_COLOR_YELLOW;
                else
                    ss << ANSI_COLOR_RESET;
                char fx_status[512];
                f->Status(fx_status);
                ss << "fx" << i << " " << fx_status << std::endl;
            }
            ss << ANSI_COLOR_RESET;
        }
    }
    return ss.str();
}

std::string mixer_status_sgz(mixer *mixr, bool all)
{
    std::stringstream ss;
    if (mixr->soundgen_num > 0)
    {
        ss << COOL_COLOR_GREEN << "\n[" << ANSI_COLOR_WHITE
           << "sound generators" << COOL_COLOR_GREEN << "]\n";
        for (int i = 0; i < mixr->soundgen_num; i++)
        {
            if (mixr->sound_generators_[i] != NULL)
            {
                if ((mixr->sound_generators_[i]->active &&
                     mixr->sound_generators_[i]->GetVolume() > 0.0) ||
                    all)
                {

                    ss << COOL_COLOR_GREEN << "[" << ANSI_COLOR_WHITE << "s"
                       << i << COOL_COLOR_GREEN << "] " << ANSI_COLOR_RESET;
                    ss << mixr->sound_generators_[i]->Info() << ANSI_COLOR_RESET
                       << "\n";

                    if (mixr->sound_generators_[i]->effects_num > 0)
                    {
                        ss << "      ";
                        for (int j = 0;
                             j < mixr->sound_generators_[i]->effects_num; j++)
                        {
                            Fx *f = mixr->sound_generators_[i]->effects[j];
                            if (f->enabled_)
                                ss << COOL_COLOR_YELLOW;
                            else
                                ss << ANSI_COLOR_RESET;
                            char fx_status[512];
                            f->Status(fx_status);
                            ss << "\n[fx " << i << ":" << j << fx_status << "]";
                        }
                        ss << ANSI_COLOR_RESET;
                    }
                    ss << "\n\n";
                }
            }
        }
    }
    return ss.str();
}

void mixer_ps(mixer *mixr, bool all)
{
    std::stringstream ss;
    ss << get_string_logo();
    ss << mixer_status_mixr(mixr);
    ss << mixer_status_env(mixr);
    ss << mixer_status_procz(mixr, all);
    ss << ANSI_COLOR_RESET;

    if (all)
        ss << mixer_status_sgz(mixr, all);

    repl_queue.push(ss.str());
}

void mixer_help(mixer *mixr)
{
    std::string strategy = oblique_strategy();
    repl_queue.push(strategy);
}

// static void mixer_print_notes(mixer *mixr)
//{
//    printf("Current KEY is %s. Compat NOTEs are:",
//           key_names[mixr->timing_info.key]);
//    // for (int i = 0; i < 6; ++i)
//    //{
//    //    printf("%s ", key_names[compat_keys[mixr->key][i]]);
//    //}
//    printf("\n");
//}

void mixer_emit_event(mixer *mixr, broadcast_event event)
{
    event_queue_item ev;
    ev.type = Event::TIMING_EVENT;
    ev.timing_info = mixr->timing_info;
    process_event_queue.push(ev);

    for (int i = 0; i < mixr->soundgen_num; ++i)
    {
        std::shared_ptr<SoundGenerator> sg = mixr->sound_generators_[i];
        if (sg != NULL)
        {
            sg->eventNotify(event, mixr->timing_info);
            if (sg->effects_num > 0)
            {
                for (int j = 0; j < sg->effects_num; j++)
                {
                    if (sg->effects[j])
                    {
                        Fx *f = sg->effects[j];
                        f->EventNotify(event);
                    }
                }
            }
        }
    }
}

void mixer_update_bpm(mixer *mixr, int bpm)
{
    mixr->bpm = bpm;
    mixr->timing_info.frames_per_midi_tick = (60.0 / bpm * SAMPLE_RATE) / PPQN;
    mixr->timing_info.loop_len_in_frames =
        mixr->timing_info.frames_per_midi_tick * PPBAR;
    mixr->timing_info.loop_len_in_ticks = PPBAR;

    mixr->timing_info.ms_per_midi_tick = 60000.0 / (bpm * PPQN);
    mixr->timing_info.midi_ticks_per_ms = PPQN / (60000.0 / bpm);

    mixr->timing_info.size_of_thirtysecond_note =
        (PPSIXTEENTH / 2) * mixr->timing_info.frames_per_midi_tick;
    mixr->timing_info.size_of_sixteenth_note =
        mixr->timing_info.size_of_thirtysecond_note * 2;
    mixr->timing_info.size_of_eighth_note =
        mixr->timing_info.size_of_sixteenth_note * 2;
    mixr->timing_info.size_of_quarter_note =
        mixr->timing_info.size_of_eighth_note * 2;

    mixer_emit_event(mixr, (broadcast_event){.type = TIME_BPM_CHANGE});
    link_set_bpm(mixr->m_ableton_link, bpm);
}

void mixer_vol_change(mixer *mixr, float vol)
{
    if (vol >= 0.0 && vol <= 1.0)
    {
        mixr->volume = vol;
    }
}

void vol_change(mixer *mixr, int sg, float vol)
{
    if (!mixer_is_valid_soundgen_num(mixr, sg))
    {
        printf("Nah mate, returning\n");
        return;
    }
    mixr->sound_generators_[sg]->SetVolume(vol);
}

void pan_change(mixer *mixr, int sg, float val)
{
    if (!mixer_is_valid_soundgen_num(mixr, sg))
    {
        printf("Nah mate, returning\n");
        return;
    }
    mixr->sound_generators_[sg]->SetPan(val);
}

void add_sound_generator(mixer *mixr, std::shared_ptr<SoundGenerator> sg)
{
    if (mixr->soundgen_num == MAX_NUM_SOUND_GENERATORS)
        audio_reply_queue.push(-1);

    mixr->sound_generators_[mixr->soundgen_num] = sg;
    audio_reply_queue.push(mixr->soundgen_num++);
}

void add_drumsynth(mixer *mixr)
{
    repl_queue.push("Adding a DRUM SYNTH!\n");
    auto ds = std::make_shared<DrumSynth>();
    add_sound_generator(mixr, ds);
}

void add_minisynth(mixer *mixr)
{
    repl_queue.push("Adding a MINISYNTH!\n");
    auto ms = std::make_shared<MiniSynth>();
    add_sound_generator(mixr, ms);
}

void add_sample(mixer *mixr, std::string sample_path)
{
    repl_queue.push("Adding a SAMPLE!\n");
    auto ds = std::make_shared<DrumSampler>(sample_path.data());
    add_sound_generator(mixr, ds);
}

void add_digisynth(mixer *mixr, std::string sample_path)
{
    repl_queue.push("Adding a DIGISYNTH!\n");
    auto ds = std::make_shared<DigiSynth>(sample_path);
    add_sound_generator(mixr, ds);
}

void add_dxsynth(mixer *mixr)
{
    repl_queue.push("Adding a DXSYNTH!\n");
    auto dx = std::make_shared<dxsynth>();
    add_sound_generator(mixr, dx);
}

void add_looper(mixer *mixr, std::string filename, bool loop_mode)
{
    repl_queue.push("Adding a Granular Looper!\n");
    auto loopr = std::make_shared<looper>(filename.data(), loop_mode);
    add_sound_generator(mixr, loopr);
}

void mixer_midi_tick(mixer *mixr)
{
    mixr->timing_info.is_thirtysecond = false;
    mixr->timing_info.is_twentyfourth = false;
    mixr->timing_info.is_sixteenth = false;
    mixr->timing_info.is_twelth = false;
    mixr->timing_info.is_eighth = false;
    mixr->timing_info.is_sixth = false;
    mixr->timing_info.is_quarter = false;
    mixr->timing_info.is_third = false;
    mixr->timing_info.is_start_of_loop = false;

    mixer_check_for_audio_action_queue_messages(mixr);
    mixer_check_for_midi_messages(mixr); // from external

    if (mixr->timing_info.midi_tick % PPBAR == 0)
        mixr->timing_info.is_start_of_loop = true;

    if (mixr->timing_info.midi_tick % 120 == 0)
    {
        mixr->timing_info.is_thirtysecond = true;

        if (mixr->timing_info.midi_tick % 240 == 0)
        {
            mixr->timing_info.is_sixteenth = true;
            mixr->timing_info.sixteenth_note_tick++;

            if (mixr->timing_info.midi_tick % 480 == 0)
            {
                mixr->timing_info.is_eighth = true;

                if (mixr->timing_info.midi_tick % PPQN == 0)
                    mixr->timing_info.is_quarter = true;
            }
        }
    }

    // so far only used for ARP engines
    if (mixr->timing_info.midi_tick % 160 == 0)
    {
        mixr->timing_info.is_twentyfourth = true;

        if (mixr->timing_info.midi_tick % 320 == 0)
        {
            mixr->timing_info.is_twelth = true;

            if (mixr->timing_info.midi_tick % 640 == 0)
            {
                mixr->timing_info.is_sixth = true;

                if (mixr->timing_info.midi_tick % 1280 == 0)
                    mixr->timing_info.is_third = true;
            }
        }
    }

    // std::cout << "Mixer -- midi_tick:" << mixr->timing_info.midi_tick
    //          << " 16th:" << mixr->timing_info.sixteenth_note_tick
    //          << " Start of Loop:" <<
    //          mixr->timing_info.is_start_of_loop
    //          << std::endl;

    repl_queue.push("tick");
    mixer_emit_event(mixr, (broadcast_event){.type = TIME_MIDI_TICK});
    // lo_send(mixr->processing_addr, "/bpm", NULL);
}

bool should_progress_chords(mixer *mixr, int tick)
{
    int chance = rand() % 100;
    if (mixr->should_progress_chords == false)
        return false;

    if (tick == 0)
    {
        if (chance > 75)
            return true;
    }
    else if (tick == PPQN * 2)
    {
        if (chance > 97)
            return true;
    }
    else if (tick == PPQN * 3)
    {
        if (chance > 99)
            return true;
    }

    return false;
}

int mixer_gennext(mixer *mixr, float *out, int frames_per_buffer)
{

    link_update_from_main_callback(mixr->m_ableton_link, frames_per_buffer);

    for (int i = 0, j = 0; i < frames_per_buffer; i++, j += 2)
    {
        double output_left = 0.0;
        double output_right = 0.0;

        mixr->timing_info.cur_sample++;

        if (mixr->preview.enabled)
        {
            stereo_val preview_audio = preview_buffer_generate(&mixr->preview);
            output_left += preview_audio.left * 0.6;
            output_right += preview_audio.right * 0.6;
        }

        if (link_is_midi_tick(mixr->m_ableton_link, &mixr->timing_info, i))
        {
            int current_tick_within_bar = mixr->timing_info.midi_tick % PPBAR;
            if (should_progress_chords(mixr, current_tick_within_bar))
                mixer_next_chord(mixr);

            mixer_midi_tick(mixr);
        }

        if (mixr->soundgen_num > 0)
        {
            for (int i = 0; i < mixr->soundgen_num; i++)
            {
                if (mixr->sound_generators_[i] != NULL)
                {
                    mixr->soundgen_cur_val[i] =
                        mixr->sound_generators_[i]->genNext();
                    output_left += mixr->soundgen_cur_val[i].left;
                    output_right += mixr->soundgen_cur_val[i].right;
                }
            }
        }

        out[j] = mixr->volume * (output_left / 1.53);
        out[j + 1] = mixr->volume * (output_right / 1.53);
    }

    return 0;
}

bool mixer_del_soundgen(mixer *mixr, int soundgen_num)
{
    if (mixer_is_valid_soundgen_num(mixr, soundgen_num))
    {
        printf("MIXR!! Deleting SOUND GEN %d\n", soundgen_num);
        std::shared_ptr<SoundGenerator> sg =
            mixr->sound_generators_[soundgen_num];

        if (mixr->active_midi_soundgen_num == soundgen_num)
            mixr->active_midi_soundgen_num = -99;

        mixr->sound_generators_[soundgen_num] = nullptr;
    }
    return true;
}

bool mixer_is_valid_soundgen_num(mixer *mixr, int soundgen_num)
{
    if (soundgen_num >= 0 && soundgen_num < mixr->soundgen_num &&
        mixr->sound_generators_[soundgen_num] != NULL)
        return true;
    return false;
}

bool mixer_is_valid_fx(mixer *mixr, int soundgen_num, int fx_num)
{
    if (mixer_is_valid_soundgen_num(mixr, soundgen_num))
    {
        std::shared_ptr<SoundGenerator> sg =
            mixr->sound_generators_[soundgen_num];
        if (fx_num >= 0 && fx_num < sg->effects_num && sg->effects[fx_num])
            return true;
    }
    return false;
}

void mixer_set_notes(mixer *mixr)
{
    mixr->timing_info.notes[0] = mixr->timing_info.key;
    mixr->timing_info.notes[1] =
        (mixr->timing_info.key + 2) % NUM_KEYS; // W step
    mixr->timing_info.notes[2] =
        (mixr->timing_info.key + 4) % NUM_KEYS; // W step
    mixr->timing_info.notes[3] =
        (mixr->timing_info.key + 5) % NUM_KEYS; // H step
    mixr->timing_info.notes[4] =
        (mixr->timing_info.key + 7) % NUM_KEYS; // W step
    mixr->timing_info.notes[5] =
        (mixr->timing_info.key + 9) % NUM_KEYS; // W step
    mixr->timing_info.notes[6] =
        (mixr->timing_info.key + 11) % NUM_KEYS; // W step
    mixr->timing_info.notes[7] =
        (mixr->timing_info.key + 12) % NUM_KEYS; // H step
}

int mixer_print_timing_info(mixer *mixr)
{
    mixer_timing_info *info = &mixr->timing_info;
    printf("TIMING INFO!\n");
    printf("============\n");
    printf("FRAMES per midi tick:%d\n", info->frames_per_midi_tick);
    printf("MS per MIDI tick:%f\n", info->ms_per_midi_tick);
    printf("TIME of next MIDI tick:%f\n", info->time_of_next_midi_tick);
    printf("SIXTEENTH NOTE tick:%d\n", info->sixteenth_note_tick);
    printf("MIDI tick:%d\n", info->midi_tick);
    printf("LOOP beat:%d\n", info->loop_beat);
    printf("LOOP Started:%d\n", info->loop_started);
    printf("CUR SAMPLE:%d\n", info->cur_sample);
    printf("Loop_len_in_frames:%d\n", info->loop_len_in_frames);
    printf("Loop_len_in_ticks:%d\n", info->loop_len_in_ticks);
    printf("Size of 1/32 note:%d\n", info->size_of_thirtysecond_note);
    printf("Size of 1/16 note:%d\n", info->size_of_sixteenth_note);
    printf("Size of 1/8 note:%d\n", info->size_of_eighth_note);
    printf("Size of 1/4 note:%d\n", info->size_of_quarter_note);

    printf("Has_started:%d\n", info->has_started);
    printf("Start of loop:%d\n", info->is_start_of_loop);
    printf("Is 1/32:%d\n", info->is_thirtysecond);
    printf("Is 1/16:%d\n", info->is_sixteenth);
    printf("Is 1/8:%d\n", info->is_eighth);
    printf("Is 1/4:%d\n", info->is_quarter);
    printf("Is midi_tick:%d\n", info->is_midi_tick);
    return 0;
}

double mixer_get_hz_per_bar(mixer *mixr)
{

    double hz_per_beat = (60. / mixr->bpm);
    return hz_per_beat / 4;
}

double mixer_get_hz_per_timing_unit(mixer *mixr, unsigned int timing_unit)
{
    double return_val = 0;
    double hz_per_beat = (60. / mixr->bpm);
    if (timing_unit == Q2)
        return_val = hz_per_beat / 2.;
    if (timing_unit == Q4)
        return_val = hz_per_beat;
    else if (timing_unit == Q8)
        return_val = hz_per_beat * 2;
    else if (timing_unit == Q16)
        return_val = hz_per_beat * 4;
    else if (timing_unit == Q32)
        return_val = hz_per_beat * 8;

    return return_val;
}

int mixer_get_ticks_per_cycle_unit(mixer *mixr, unsigned int event_type)
{
    int ticks = 0;
    switch (event_type)
    {
    case (TIME_START_OF_LOOP_TICK):
        ticks = mixr->timing_info.loop_len_in_ticks;
        break;
    case (TIME_MIDI_TICK):
        ticks = 1;
        break;
    case (TIME_QUARTER_TICK):
        ticks = PPQN;
        break;
    case (TIME_EIGHTH_TICK):
        ticks = PPQN / 2;
        break;
    case (TIME_SIXTEENTH_TICK):
        ticks = PPQN / 4;
        break;
    case (TIME_THIRTYSECOND_TICK):
        ticks = PPQN / 8;
        break;
    }
    return ticks;
}
void mixer_set_octave(mixer *mixr, int octave)
{
    if (octave > -10 && octave < 10)
        mixr->timing_info.octave = octave;
}
void mixer_set_bars_per_chord(mixer *mixr, int bars)
{
    if (bars > 0 && bars < 32)
        mixr->bars_per_chord = bars;
}

void mixer_set_key(mixer *mixr, unsigned int key)
{
    if (key < NUM_KEYS)
        mixr->timing_info.key = key;
}

void mixer_set_key(mixer *mixr, std::string str_key)
{
    str_lower(str_key);

    int key = -1;
    if (str_key == "c#" || str_key == "dm")
        key = 1;
    else if (str_key == "d#" || str_key == "em")
        key = 3;
    else if (str_key == "f#" || str_key == "gm")
        key = 6;
    else if (str_key == "g#" || str_key == "am")
        key = 8;
    else if (str_key == "a#" || str_key == "bm")
        key = 10;
    else if (str_key == "c")
        key = 0;
    else if (str_key == "d")
        key = 2;
    else if (str_key == "e")
        key = 4;
    else if (str_key == "f")
        key = 5;
    else if (str_key == "g")
        key = 7;
    else if (str_key == "a")
        key = 9;
    else if (str_key == "b")
        key = 11;

    if (key != -1)
        mixer_set_key(mixr, key);
}

void mixer_set_chord_progression(mixer *mixr, unsigned int prog_num)
{
    if (prog_num < NUM_PROGRESSIONS)
    {
        switch (prog_num)
        {
        case (0):
            mixr->progression_type = 0;
            mixr->prog_len = 3;
            mixr->prog_degrees[0] = 0; // I
            mixr->prog_degrees[1] = 3; // IV
            mixr->prog_degrees[2] = 4; // V
            break;
        case (1):
            mixr->progression_type = 1;
            mixr->prog_len = 4;
            mixr->prog_degrees[0] = 0; // I
            mixr->prog_degrees[1] = 4; // V
            mixr->prog_degrees[2] = 5; // vi
            mixr->prog_degrees[3] = 3; // IV
            break;
        case (2):
            mixr->progression_type = 2;
            mixr->prog_len = 4;
            mixr->prog_degrees[0] = 0; // I
            mixr->prog_degrees[1] = 5; // vi
            mixr->prog_degrees[2] = 3; // IV
            mixr->prog_degrees[3] = 4; // V
            break;
        case (3):
            mixr->progression_type = 3;
            mixr->prog_len = 4;
            mixr->prog_degrees[0] = 5; // vi
            mixr->prog_degrees[1] = 1; // ii
            mixr->prog_degrees[2] = 4; // V
            mixr->prog_degrees[3] = 0; // I
            break;
        }
        mixr->prog_degrees_idx = 0;
        mixr->timing_info.chord_progression_index = mixr->prog_degrees_idx;
        mixr->timing_info.chord = mixr->prog_degrees[mixr->prog_degrees_idx];
    }
}
void mixer_change_chord(mixer *mixr, unsigned int root, unsigned int chord_type)
{
    if (root < NUM_KEYS && chord_type < NUM_CHORD_TYPES)
    {
        mixr->timing_info.chord = root;
        mixr->timing_info.chord_type = chord_type;
        mixer_emit_event(mixr, (broadcast_event){.type = TIME_CHORD_CHANGE});
    }
}

int mixer_get_key_from_degree(mixer *mixr, unsigned int scale_degree)
{
    return mixr->timing_info.key + scale_degree;
}

void mixer_preview_audio(mixer *mixr, char *filename)
{
    if (is_valid_file(filename))
    {
        mixr->preview.enabled = false;
        preview_buffer_import_file(&mixr->preview, filename);
    }
}

void preview_buffer_import_file(preview_buffer *buffy, char *filename)
{
    strncpy(buffy->filename, filename, 512);
    audio_buffer_details deetz =
        import_file_contents(&buffy->audio_buffer, filename);
    buffy->audio_buffer_len = deetz.buffer_length;
    buffy->num_channels = deetz.num_channels;
    buffy->audio_buffer_read_idx = 0;
    buffy->enabled = true;
}

stereo_val preview_buffer_generate(preview_buffer *buffy)
{
    stereo_val ret = {.0, .0};
    if (!buffy->enabled || !buffy->audio_buffer)
        return ret;

    ret.left = buffy->audio_buffer[buffy->audio_buffer_read_idx];
    if (buffy->num_channels == 1)
        ret.right = ret.left;
    else
        ret.right = buffy->audio_buffer[buffy->audio_buffer_read_idx + 1];

    buffy->audio_buffer_read_idx += buffy->num_channels;
    if (buffy->audio_buffer_read_idx >= buffy->audio_buffer_len)
    {
        buffy->audio_buffer_read_idx = 0;
        buffy->enabled = false;
    }

    return ret;
}

void mixer_enable_print_midi(mixer *mixr, bool b)
{
    mixr->midi_print_notes = b;
}

void mixer_check_for_audio_action_queue_messages(mixer *mixr)
{
    while (auto action = audio_queue.try_pop())
    {
        if (action)
        {
            if (action->type == AudioAction::STATUS)
                mixer_ps(mixr, action->status_all);
            else if (action->type == AudioAction::HELP)
                mixer_help(mixr);
            else if (action->type == AudioAction::ADD)
            {
                std::cout << "ADD SOUNDGEN YO!\n";
                switch (action->soundgenerator_type)
                {
                case (MINISYNTH_TYPE):
                    add_minisynth(mixr);
                    break;
                case (DXSYNTH_TYPE):
                    add_dxsynth(mixr);
                    break;
                case (DIGISYNTH_TYPE):
                    add_digisynth(mixr, action->filepath);
                    break;
                case (LOOPER_TYPE):
                    add_looper(mixr, action->filepath, action->loop_mode);
                    break;
                case (DRUMSAMPLER_TYPE):
                    add_sample(mixr, action->filepath);
                    break;
                case (DRUMSYNTH_TYPE):
                    add_drumsynth(mixr);
                    break;
                }
            }
            else if (action->type == AudioAction::ADD_FX)
                interpreter_sound_cmds::ParseFXCmd(action->args);
            else if (action->type == AudioAction::BPM)
                mixer_update_bpm(mixr, action->new_bpm);
            else if (action->type == AudioAction::MIXER_UPDATE)
            {
                if (action->param_name == "key")
                    mixer_set_key(mixr, action->param_val);
                else if (action->param_name == "prog")
                    mixer_set_chord_progression(
                        mixr, std::stoi(action->param_val, nullptr, 0));
            }
            else if (action->type == AudioAction::MIDI_EVENT_ADD ||
                     action->type == AudioAction::MIDI_EVENT_ADD_DELAYED)
            {
                // args[0] is sound generator
                // args[1] is midi_note
                // args[2] is delay in midi ticks if present and type ==
                // MIDI_EVENT_ADD_DELAYED
                // args[3] if present is velocity
                auto args = action->args;
                int args_size = args.size();
                if (args_size >= 2)
                {
                    auto soundgen =
                        std::dynamic_pointer_cast<object::SoundGenerator>(
                            args[0]);
                    if (soundgen)
                    {
                        if (mixer_is_valid_soundgen_num(mixr,
                                                        soundgen->soundgen_id_))
                        {
                            auto sg =
                                mixr->sound_generators_[soundgen->soundgen_id_];
                            // sg->parseMidiEvent(event_on, mixr->timing_info);

                            auto int_object =
                                std::dynamic_pointer_cast<object::Number>(
                                    args[1]);

                            if (!int_object)
                                return;

                            auto midinum = int_object->value_;

                            // optional value
                            int velocity = 127;
                            if ((action->type ==
                                     AudioAction::MIDI_EVENT_ADD_DELAYED &&
                                 args_size == 4) ||
                                (action->type !=
                                     AudioAction::MIDI_EVENT_ADD_DELAYED &&
                                 args_size == 3))
                            {
                                int pos =
                                    action->type ==
                                            AudioAction::MIDI_EVENT_ADD_DELAYED
                                        ? 3
                                        : 2;

                                auto velocity_obj =
                                    std::dynamic_pointer_cast<object::Number>(
                                        args[pos]);

                                if (velocity_obj)
                                    velocity = velocity_obj->value_;
                            }

                            midi_event event_on =
                                new_midi_event(MIDI_ON, midinum, velocity);
                            event_on.source = EXTERNAL_OSC;

                            if (action->type ==
                                AudioAction::MIDI_EVENT_ADD_DELAYED)
                            {
                                if (args_size >= 3)
                                {
                                    auto delay_time_obj =
                                        std::dynamic_pointer_cast<
                                            object::Number>(args[2]);

                                    if (!delay_time_obj)
                                        return;
                                    int delay_time = delay_time_obj->value_;
                                    int delay_tick =
                                        (mixr->timing_info.midi_tick +
                                         delay_time) %
                                        PPBAR;
                                    midi_event event = new_midi_event(
                                        MIDI_ON, midinum, velocity);
                                    event.delete_after_use = true;

                                    sg->addDelayedEvent(event, delay_tick);
                                }
                                else
                                {
                                    std::cerr << "NEED A DELAY SIZE IN "
                                                 "MIDI TICKS"
                                              << std::endl;
                                }
                            }
                            else
                            {

                                sg->noteOn(event_on);

                                int note_duration_ms = sg->note_duration_ms_;
                                int duration_in_midi_ticks =
                                    note_duration_ms /
                                    mixr->timing_info.ms_per_midi_tick;
                                int midi_off_tick =
                                    (mixr->timing_info.midi_tick +
                                     duration_in_midi_ticks) %
                                    PPBAR;

                                midi_event event_off =
                                    new_midi_event(MIDI_OFF, midinum, velocity);
                                event_off.delete_after_use = true;
                                sg->addDelayedEvent(event_off, midi_off_tick);
                            }
                        }
                    }
                }
            }
            else if (action->type == AudioAction::MIDI_EVENT_CLEAR)
            {
                // args[0] is sound generator
                auto args = action->args;
                int args_size = args.size();
                if (args_size == 2)
                {
                    auto soundgen =
                        std::dynamic_pointer_cast<object::SoundGenerator>(
                            args[0]);
                    if (soundgen)
                    {
                        if (mixer_is_valid_soundgen_num(mixr,
                                                        soundgen->soundgen_id_))
                        {
                            auto sg =
                                mixr->sound_generators_[soundgen->soundgen_id_];
                            auto int_object =
                                std::dynamic_pointer_cast<object::Number>(
                                    args[1]);

                            if (!int_object)
                                return;

                            auto start_idx = int_object->value_;
                            sg->clearQueue(start_idx);
                        }
                    }
                }
            }
            else if (action->type == AudioAction::UPDATE)
            {
                if (mixer_is_valid_soundgen_num(mixr,
                                                action->mixer_soundgen_idx))
                {
                    double param_val = 0.;

                    if (action->param_val == "sync32")
                        param_val = mixer_get_hz_per_timing_unit(mixr, Q32);
                    if (action->param_val == "sync16")
                        param_val = mixer_get_hz_per_timing_unit(mixr, Q16);
                    else if (action->param_val == "sync8")
                        param_val = mixer_get_hz_per_timing_unit(mixr, Q8);
                    else if (action->param_val == "sync4")
                        param_val = mixer_get_hz_per_timing_unit(mixr, Q4);
                    else if (action->param_val == "sync2")
                        param_val = mixer_get_hz_per_timing_unit(mixr, Q2);
                    else
                        param_val = std::stod(action->param_val);

                    auto sg =
                        mixr->sound_generators_[action->mixer_soundgen_idx];
                    if (!sg)
                    {
                        std::cerr << "WHOE NELLY! Naw SG! bailing out!\n";
                        return;
                    }
                    if (action->delayed_by > 0)
                    {
                        // TODO - this is hardcoded for pitch - should add an
                        // ENUM to be able to do others
                        midi_event event =
                            new_midi_event(MIDI_PITCHBEND, param_val * 10, 0);
                        event.delete_after_use = true;
                        sg->addDelayedEvent(event, action->delayed_by);
                        return;
                    }
                    if (action->param_name == "volume")
                        sg->SetVolume(param_val);
                    else if (action->param_name == "pan")
                        sg->SetPan(param_val);
                    else
                    {
                        // first check if we're setting an FX param
                        if (action->fx_id != -1)
                        {
                            int fx_num = action->fx_id;
                            if (mixer_is_valid_fx(
                                    mixr, action->mixer_soundgen_idx, fx_num))
                            {
                                Fx *f = sg->effects[fx_num];
                                if (action->param_name == "active")
                                    f->enabled_ = param_val;
                                else
                                    f->SetParam(action->param_name, param_val);
                            }
                        }
                        else // must be a SoundGenerator param
                        {
                            sg->SetParam(action->param_name, param_val);
                        }
                    }
                }
            }
            else if (action->type == AudioAction ::INFO)
            {
                if (mixer_is_valid_soundgen_num(mixr,
                                                action->mixer_soundgen_idx))
                {
                    auto sg =
                        mixr->sound_generators_[action->mixer_soundgen_idx];
                    repl_queue.push(sg->Info());
                }
            }
            else if (action->type == AudioAction ::SAVE_PRESET ||
                     action->type == AudioAction::LOAD_PRESET)
            {
                interpreter_sound_cmds::ParseSynthCmd(action->args);
            }
            else if (action->type == AudioAction::RAND)
            {
                mixr->sound_generators_[action->mixer_soundgen_idx]
                    ->randomize();
            }
            else if (action->type == AudioAction::PREVIEW)
            {
                char *fname = action->preview_filename.data();
                mixer_preview_audio(mixr, fname);
            }
        }
    }
}

void mixer_check_for_midi_messages(mixer *mixr)
{
    PmEvent msg[32];
    if (Pm_Poll(mixr->midi_stream))
    {
        int cnt = Pm_Read(mixr->midi_stream, msg, 32);
        for (int i = 0; i < cnt; i++)
        {
            int status = Pm_MessageStatus(msg[i].message);
            int data1 = Pm_MessageData1(msg[i].message);
            int data2 = Pm_MessageData2(msg[i].message);

            if (status == 176)
            {
                if (data1 == 9)
                    mixer_set_midi_bank(mixr, 0);
                if (data1 == 10)
                    mixer_set_midi_bank(mixr, 1);
                if (data1 == 11)
                    mixer_set_midi_bank(mixr, 2);
                if (data1 == 12)
                    mixer_set_midi_bank(mixr, 3);
            }

            if (mixr->midi_print_notes)
                printf("[MIDI message] status:%d data1:%d "
                       "data2:%d\n",
                       status, data1, data2);

            if (mixr->midi_control_destination != NONE &&
                mixer_is_valid_soundgen_num(mixr,
                                            mixr->active_midi_soundgen_num))
            {

                std::shared_ptr<SoundGenerator> sg =
                    mixr->sound_generators_[mixr->active_midi_soundgen_num];

                midi_event ev = new_midi_event(status, data1, data2);

                ev.source = EXTERNAL_DEVICE;
                ev.delete_after_use = false;

                sg->parseMidiEvent(ev, mixr->timing_info);
            }
            else
            {
                printf("Got midi but not connected to "
                       "synth\n");
            }
        }
    }
}

void mixer_set_midi_bank(mixer *mixr, int num)
{
    if (num >= 0 && num < 4)
        mixr->midi_bank_num = num;
}

void mixer_set_should_progress_chords(mixer *mixr, bool b)
{
    mixr->should_progress_chords = b;
}

void mixer_next_chord(mixer *mixr)
{
    mixr->prog_degrees_idx = (mixr->prog_degrees_idx + 1) % mixr->prog_len;
    mixr->timing_info.chord_progression_index = mixr->prog_degrees_idx;
    mixr->timing_info.chord = mixr->prog_degrees[mixr->prog_degrees_idx];
}

void mixr_add_file_to_monitor(mixer *mixr, std::string filepath)
{
    mixr->file_monitors.push_back(
        file_monitor{.function_file_filepath = filepath});
}
