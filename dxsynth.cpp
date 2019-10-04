#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "dxsynth.h"
#include "midi_freq_table.h"
#include "mixer.h"
#include "utils.h"

#include <iostream>

extern mixer *mixr;
extern const wchar_t *sparkchars;
extern const char *s_source_enum_to_name[];
extern const char *s_dest_enum_to_name[];

static const char *s_dx_dest_names[] = {"dx_dest_none", "dx_dest_amp_mod",
                                        "dx_dest_vibrato"};

dxsynth::dxsynth()
{
    std::cout << "NEW DX\n";
    type = DXSYNTH_TYPE;
    active_midi_osc = 1;

    dxsynth_reset(this);

    for (int i = 0; i < MAX_DX_VOICES; i++)
    {
        m_voices[i] = new_dxsynth_voice();
        if (!m_voices[i])
            return; // would be bad

        dxsynth_voice_init_global_parameters(m_voices[i],
                                             &m_global_synth_params);
    }

    std::cout << "DX PREP \n";
    dxsynth_prepare_for_play(this);

    std::cout << "DX INIT MODMATRIX\n";
    // use first voice to setup global
    dxsynth_voice_initialize_modmatrix(m_voices[0], &m_global_modmatrix);

    std::cout << "VOICE SET CORE DX\n";
    for (int i = 0; i < MAX_DX_VOICES; i++)
    {
        voice_set_modmatrix_core(&m_voices[i]->m_voice,
                                 get_matrix_core(&m_global_modmatrix));
    }
    std::cout << "UPDATE DX\n";
    dxsynth_update(this);

    m_last_note_frequency = -1.0;

    active = true;
    printf("BOOM!\n");
}

dxsynth::~dxsynth()
{
    for (int i = 0; i < MAX_DX_VOICES; i++)
    {
        dxsynth_voice_free_self(m_voices[i]);
    }
    printf("Deleting dxsynth self\n");
}

void dxsynth::start()
{
    active = true;
    engine.cur_step = mixr->timing_info.sixteenth_note_tick % 16;
}

void dxsynth::stop()
{
    active = false;
    allNotesOff();
}

void dxsynth::status(wchar_t *status_string)
{
    char *INSTRUMENT_COLOR = (char *)ANSI_COLOR_RESET;
    if (active)
        INSTRUMENT_COLOR = (char *)ANSI_COLOR_CYAN;

    // clang-format off
    swprintf(
        status_string, MAX_STATIC_STRING_SZ,
        WANSI_COLOR_WHITE
        "%s " "%s" "algo:%d vol:%.1f pan:%.1f midi_osc:%d porta:%.1f pitchrange:%d op4fb:%.2f\n"
        "vel2att:%d note2dec:%d reset2zero:%d legato:%d l1_wav:%d l1_int:%.2f l1_rate:%0.2f\n"
        "l1_dest1:%s l1_dest2:%s\nl1_dest3:%s l1_dest4:%s\n"
        "o1wav:%d o1rat:%.2f o1det:%.2f e1att:%.2f e1dec:%.2f e1sus:%.2f e1rel:%.2f\n"
        "o2wav:%d o2rat:%.2f o2det:%.2f e2att:%.2f e2dec:%.2f e2sus:%.2f e2rel:%.2f\n"
        "o3wav:%d o3rat:%.2f o3det:%.2f e3att:%.2f e3dec:%.2f e3sus:%.2f e3rel:%.2f\n"
        "o4wav:%d o4rat:%.2f o4det:%.2f e4att:%.2f e4dec:%.2f e4sus:%.2f e4rel:%.2f\n"
        "op1out:%.2f op2out:%.2f op3out:%.2f op4out:%.2f",

        m_settings.m_settings_name,
        INSTRUMENT_COLOR,
        m_settings.m_voice_mode,
        volume, pan,
        active_midi_osc,
        m_settings.m_portamento_time_ms,
        m_settings.m_pitchbend_range,
        m_settings.m_op4_feedback,
        m_settings.m_velocity_to_attack_scaling,
        m_settings.m_note_number_to_decay_scaling,
        m_settings.m_reset_to_zero,
        m_settings.m_legato_mode,

        m_settings.m_lfo1_waveform,
        m_settings.m_lfo1_intensity,
        m_settings.m_lfo1_rate,
        s_dx_dest_names[m_settings.m_lfo1_mod_dest1],
        s_dx_dest_names[m_settings.m_lfo1_mod_dest2],
        s_dx_dest_names[m_settings.m_lfo1_mod_dest3],
        s_dx_dest_names[m_settings.m_lfo1_mod_dest4],

        m_settings.m_op1_waveform, m_settings.m_op1_ratio,
        m_settings.m_op1_detune_cents, m_settings.m_eg1_attack_ms,
        m_settings.m_eg1_decay_ms, m_settings.m_eg1_sustain_lvl,
        m_settings.m_eg1_release_ms,

        m_settings.m_op2_waveform, m_settings.m_op2_ratio,
        m_settings.m_op2_detune_cents, m_settings.m_eg2_attack_ms,
        m_settings.m_eg2_decay_ms, m_settings.m_eg2_sustain_lvl,
        m_settings.m_eg2_release_ms,

        m_settings.m_op3_waveform, m_settings.m_op3_ratio,
        m_settings.m_op3_detune_cents, m_settings.m_eg3_attack_ms,
        m_settings.m_eg3_decay_ms, m_settings.m_eg3_sustain_lvl,
        m_settings.m_eg3_release_ms,

        m_settings.m_op4_waveform, m_settings.m_op4_ratio,
        m_settings.m_op4_detune_cents, m_settings.m_eg4_attack_ms,
        m_settings.m_eg4_decay_ms, m_settings.m_eg4_sustain_lvl,
        m_settings.m_eg4_release_ms,
        m_settings.m_op1_output_lvl,
        m_settings.m_op2_output_lvl,
        m_settings.m_op3_output_lvl,
        m_settings.m_op4_output_lvl
        );
    // clang-format on
    wchar_t scratch[1024] = {};
    sequence_engine_status(&engine, scratch);
    wcscat(status_string, scratch);
}

stereo_val dxsynth::genNext()
{
    if (!active)
        return (stereo_val){0, 0};

    double accum_out_left = 0.0;
    double accum_out_right = 0.0;

    // float mix = 1.0 / MAX_DX_VOICES;
    float mix = 0.25;

    double out_left = 0.0;
    double out_right = 0.0;

    for (int i = 0; i < MAX_DX_VOICES; i++)
    {
        if (m_voices[i])
            dxsynth_voice_gennext(m_voices[i], &out_left, &out_right);

        accum_out_left += mix * out_left;
        accum_out_right += mix * out_right;
    }

    pan = fmin(pan, 1.0);
    pan = fmax(pan, -1.0);
    double pan_left = 0.707;
    double pan_right = 0.707;
    calculate_pan_values(pan, &pan_left, &pan_right);

    stereo_val out = {.left = accum_out_left * volume * pan_left,
                      .right = accum_out_right * volume * pan_right};
    out = effector(this, out);
    return out;
}

void dxsynth::noteOn(midi_event ev)
{

    bool steal_note = true;
    for (int i = 0; i < MAX_DX_VOICES; i++)
    {
        dxsynth_voice *msv = m_voices[i];
        if (!msv)
            return; // should never happen
        if (!msv->m_voice.m_note_on)
        {
            dxsynth_increment_voice_timestamps(this);
            voice_note_on(&msv->m_voice, ev.data1, ev.data2,
                          get_midi_freq(ev.data1), m_last_note_frequency);

            m_last_note_frequency = get_midi_freq(ev.data1);
            steal_note = false;
            break;
        }
    }

    if (steal_note)
    {
        if (mixr->debug_mode)
            printf("STEAL NOTE\n");
        dxsynth_voice *msv = dxsynth_get_oldest_voice(this);
        if (msv)
        {
            dxsynth_increment_voice_timestamps(this);
            voice_note_on(&msv->m_voice, ev.data1, ev.data2,
                          get_midi_freq(ev.data1), m_last_note_frequency);
        }
        m_last_note_frequency = get_midi_freq(ev.data1);
    }
}

void dxsynth::allNotesOff()
{
    for (int i = 0; i < MAX_DX_VOICES; i++)
    {
        dxsynth_voice *dxv = m_voices[i];
        if (dxv)
            voice_note_off(&dxv->m_voice, -1);
    }
}

void dxsynth::noteOff(midi_event ev)
{

    for (int i = 0; i < MAX_DX_VOICES; i++)
    {
        dxsynth_voice *dxv = dxsynth_get_oldest_voice_with_note(this, ev.data1);
        if (dxv)
        {
            voice_note_off(&dxv->m_voice, ev.data1);
        }
    }
}

void dxsynth::control(midi_event ev)
{
    double val = 0;
    switch (ev.data1)
    {
    case (1):
        if (mixr->midi_bank_num == 0)
        {
            printf("Algo\n");
            val = scaleybum(0, 127, 1, 7, ev.data2);
            dxsynth_set_voice_mode(this, val);
        }
        else if (mixr->midi_bank_num == 1)
        {
            int osc_num = scaleybum(0, 127, 1, 4, ev.data2);
            printf("OSC%d!\n", osc_num);
            dxsynth_set_active_midi_osc(this, osc_num);
        }
        else if (mixr->midi_bank_num == 2)
        {
        }
        break;
    case (2):
        if (mixr->midi_bank_num == 0)
        {
            printf("Op4Feedback\n");
            val = scaleybum(0, 127, 0, 70, ev.data2);
            dxsynth_set_op4_feedback(this, val);
        }
        else if (mixr->midi_bank_num == 1)
        {
            // wav
            printf("wav %d\n", active_midi_osc);
            int osc_type = scaleybum(0, 127, 0, MAX_OSC - 1, ev.data2);
            dxsynth_set_op_waveform(this, active_midi_osc, osc_type);
        }
        else if (mixr->midi_bank_num == 2)
        {
        }
        break;
    case (3):
        if (mixr->midi_bank_num == 0)
        {
            printf("LFO Rate\n");
            val = scaleybum(0, 128, MIN_LFO_RATE, MAX_LFO_RATE, ev.data2);
            dxsynth_set_lfo1_rate(this, val);
        }
        else if (mixr->midi_bank_num == 1)
        {
            printf("opratio %d\n", active_midi_osc);
            val = scaleybum(0, 127, 0.01, 10, ev.data2);
            dxsynth_set_op_ratio(this, active_midi_osc, val);
        }
        else if (mixr->midi_bank_num == 2)
        {
        }
        break;
    case (4):
        if (mixr->midi_bank_num == 0)
        {
            printf("LFO Intensity\n");
            val = scaleybum(0, 128, 0.0, 1.0, ev.data2);
            dxsynth_set_lfo1_intensity(this, val);
        }
        else if (mixr->midi_bank_num == 1)
        {
            printf("detune %d\n", active_midi_osc);
            val = scaleybum(0, 127, -100, 100, ev.data2);
            dxsynth_set_op_detune(this, active_midi_osc, val);
        }
        else if (mixr->midi_bank_num == 2)
        {
        }
        break;
    case (5):
        if (mixr->midi_bank_num == 0)
        {
            val = scaleybum(0, 127, 0, 99, ev.data2);
            printf("OP1OUT! %f\n", val);
            dxsynth_set_op_output_lvl(this, 1, val);
        }
        else if (mixr->midi_bank_num == 1)
        {
            printf("attack %d\n", active_midi_osc);
            val = scaleybum(0, 127, EG_MINTIME_MS, EG_MINTIME_MS, ev.data2);
            dxsynth_set_eg_attack_ms(this, active_midi_osc, val);
        }
        else if (mixr->midi_bank_num == 2)
        {
        }
        break;
    case (6):
        if (mixr->midi_bank_num == 0)
        {
            val = scaleybum(0, 127, 0, 99, ev.data2);
            printf("OP2OUT! %f\n", val);
            dxsynth_set_op_output_lvl(this, 2, val);
        }
        else if (mixr->midi_bank_num == 1)
        {
            printf("decay %d\n", active_midi_osc);
            val = scaleybum(0, 127, EG_MINTIME_MS, EG_MINTIME_MS, ev.data2);
            dxsynth_set_eg_decay_ms(this, active_midi_osc, val);
        }
        else if (mixr->midi_bank_num == 2)
        {
        }
        break;
    case (7):
        if (mixr->midi_bank_num == 0)
        {
            val = scaleybum(0, 127, 0, 99, ev.data2);
            printf("OP3OUT! %f\n", val);
            dxsynth_set_op_output_lvl(this, 3, val);
        }
        else if (mixr->midi_bank_num == 1)
        {
            printf("sustain %d\n", active_midi_osc);
            val = scaleybum(0, 127, 0, 1, ev.data2);
            dxsynth_set_eg_sustain_lvl(this, active_midi_osc, val);
        }
        else if (mixr->midi_bank_num == 2)
        {
        }
        break;
    case (8):
        if (mixr->midi_bank_num == 0)
        {
            val = scaleybum(0, 127, 0, 99, ev.data2);
            printf("OP4OUT! %f\n", val);
            dxsynth_set_op_output_lvl(this, 4, val);
        }
        else if (mixr->midi_bank_num == 1)
        {
            printf("release %d\n", active_midi_osc);
            val = scaleybum(0, 127, EG_MINTIME_MS, EG_MINTIME_MS, ev.data2);
            dxsynth_set_eg_release_ms(this, active_midi_osc, val);
        }
        else if (mixr->midi_bank_num == 2)
        {
        }
        break;
    default:
        printf("nah\n");
    }
    dxsynth_update(this);
}

void dxsynth::pitchBend(midi_event ev)
{
    unsigned int data1 = ev.data1;
    unsigned int data2 = ev.data2;
    // printf("Pitch bend, babee: %d %d\n", data1, data2);
    int actual_pitch_bent_val = (int)((data1 & 0x7F) | ((data2 & 0x7F) << 7));

    if (actual_pitch_bent_val != 8192)
    {
        double normalized_pitch_bent_val =
            (float)(actual_pitch_bent_val - 0x2000) / (float)(0x2000);
        double scaley_val =
            // scaleybum(0, 16383, -100, 100, normalized_pitch_bent_val);
            scaleybum(0, 16383, -600, 600, actual_pitch_bent_val);
        // printf("Cents to bend - %f\n", scaley_val);
        for (int i = 0; i < MAX_DX_VOICES; i++)
        {
            m_voices[i]->m_voice.m_osc1->m_cents = scaley_val;
            m_voices[i]->m_voice.m_osc2->m_cents = scaley_val + 2.5;
            m_voices[i]->m_voice.m_osc3->m_cents = scaley_val;
            m_voices[i]->m_voice.m_osc4->m_cents = scaley_val + 2.5;
            m_voices[i]->m_voice.m_v_modmatrix.m_sources[SOURCE_PITCHBEND] =
                normalized_pitch_bent_val;
        }
    }
    else
    {
        for (int i = 0; i < MAX_DX_VOICES; i++)
        {
            m_voices[i]->m_voice.m_osc1->m_cents = 0;
            m_voices[i]->m_voice.m_osc2->m_cents = 2.5;
            m_voices[i]->m_voice.m_osc3->m_cents = 0;
            m_voices[i]->m_voice.m_osc4->m_cents = 2.5;
        }
    }
}

void dxsynth_reset(dxsynth *dx)
{
    strncpy(dx->m_settings.m_settings_name, "default", 7);
    dx->m_settings.m_volume_db = 0;
    dx->m_settings.m_voice_mode = 0;
    dx->m_settings.m_portamento_time_ms = 0;
    dx->m_settings.m_pitchbend_range = 1; // 0 -12
    dx->m_settings.m_velocity_to_attack_scaling = 0;
    dx->m_settings.m_note_number_to_decay_scaling = 0;
    dx->m_settings.m_reset_to_zero = 0;
    dx->m_settings.m_legato_mode = 0;

    dx->m_settings.m_lfo1_intensity = 1;
    dx->m_settings.m_lfo1_rate = 0.5;
    dx->m_settings.m_lfo1_waveform = 0;
    dx->m_settings.m_lfo1_mod_dest1 = DX_LFO_DEST_NONE;
    dx->m_settings.m_lfo1_mod_dest2 = DX_LFO_DEST_NONE;
    dx->m_settings.m_lfo1_mod_dest3 = DX_LFO_DEST_NONE;
    dx->m_settings.m_lfo1_mod_dest4 = DX_LFO_DEST_NONE;

    dx->m_settings.m_op1_waveform = SINE;
    dx->m_settings.m_op1_ratio = 1; // 0.01-10
    dx->m_settings.m_op1_detune_cents = 0;
    dx->m_settings.m_eg1_attack_ms = 100;
    dx->m_settings.m_eg1_decay_ms = 100;
    dx->m_settings.m_eg1_sustain_lvl = 0.707;
    dx->m_settings.m_eg1_release_ms = 2000;
    dx->m_settings.m_op1_output_lvl = 90;

    dx->m_settings.m_op2_waveform = SINE;
    dx->m_settings.m_op2_ratio = 1; // 0.01-10
    dx->m_settings.m_op2_detune_cents = 0;
    dx->m_settings.m_eg2_attack_ms = 100;
    dx->m_settings.m_eg2_decay_ms = 100;
    dx->m_settings.m_eg2_sustain_lvl = 0.707;
    dx->m_settings.m_eg2_release_ms = 2000;
    dx->m_settings.m_op2_output_lvl = 75;

    dx->m_settings.m_op3_waveform = SINE;
    dx->m_settings.m_op3_ratio = 1; // 0.01-10
    dx->m_settings.m_op3_detune_cents = 0;
    dx->m_settings.m_eg3_attack_ms = 100;
    dx->m_settings.m_eg3_decay_ms = 100;
    dx->m_settings.m_eg3_sustain_lvl = 0.707;
    dx->m_settings.m_eg3_release_ms = 2000;
    dx->m_settings.m_op3_output_lvl = 75;

    dx->m_settings.m_op4_waveform = SINE;
    dx->m_settings.m_op4_ratio = 1; // 0.01-10
    dx->m_settings.m_op4_detune_cents = 0;
    dx->m_settings.m_eg4_attack_ms = 100;
    dx->m_settings.m_eg4_decay_ms = 100;
    dx->m_settings.m_eg4_sustain_lvl = 0.707;
    dx->m_settings.m_eg4_release_ms = 2000;
    dx->m_settings.m_op4_output_lvl = 75;
    dx->m_settings.m_op4_feedback = 0; // 0-70
}

bool dxsynth_prepare_for_play(dxsynth *dx)
{
    for (int i = 0; i < MAX_DX_VOICES; i++)
    {
        if (dx->m_voices[i])
        {
            dxsynth_voice_prepare_for_play(dx->m_voices[i]);
        }
    }

    dxsynth_update(dx);

    return true;
}

void dxsynth_update(dxsynth *dx)
{
    dx->m_global_synth_params.voice_params.voice_mode =
        dx->m_settings.m_voice_mode;
    dx->m_global_synth_params.voice_params.op4_feedback =
        dx->m_settings.m_op4_feedback / 100.0;
    dx->m_global_synth_params.voice_params.portamento_time_msec =
        dx->m_settings.m_portamento_time_ms;

    dx->m_global_synth_params.voice_params.osc_fo_pitchbend_mod_range =
        dx->m_settings.m_pitchbend_range;

    dx->m_global_synth_params.voice_params.lfo1_osc_mod_intensity =
        dx->m_settings.m_lfo1_intensity;

    dx->m_global_synth_params.osc1_params.amplitude =
        calculate_dx_amp(dx->m_settings.m_op1_output_lvl);
    dx->m_global_synth_params.osc2_params.amplitude =
        calculate_dx_amp(dx->m_settings.m_op2_output_lvl);
    dx->m_global_synth_params.osc3_params.amplitude =
        calculate_dx_amp(dx->m_settings.m_op3_output_lvl);
    dx->m_global_synth_params.osc4_params.amplitude =
        calculate_dx_amp(dx->m_settings.m_op4_output_lvl);

    dx->m_global_synth_params.osc1_params.fo_ratio = dx->m_settings.m_op1_ratio;
    dx->m_global_synth_params.osc2_params.fo_ratio = dx->m_settings.m_op2_ratio;
    dx->m_global_synth_params.osc3_params.fo_ratio = dx->m_settings.m_op3_ratio;
    dx->m_global_synth_params.osc4_params.fo_ratio = dx->m_settings.m_op4_ratio;

    dx->m_global_synth_params.osc1_params.waveform =
        dx->m_settings.m_op1_waveform;
    dx->m_global_synth_params.osc2_params.waveform =
        dx->m_settings.m_op2_waveform;
    dx->m_global_synth_params.osc3_params.waveform =
        dx->m_settings.m_op3_waveform;
    dx->m_global_synth_params.osc4_params.waveform =
        dx->m_settings.m_op4_waveform;

    dx->m_global_synth_params.osc1_params.cents =
        dx->m_settings.m_op1_detune_cents;
    dx->m_global_synth_params.osc2_params.cents =
        dx->m_settings.m_op2_detune_cents;
    dx->m_global_synth_params.osc3_params.cents =
        dx->m_settings.m_op3_detune_cents;
    dx->m_global_synth_params.osc4_params.cents =
        dx->m_settings.m_op4_detune_cents;

    // EG1
    dx->m_global_synth_params.eg1_params.attack_time_msec =
        dx->m_settings.m_eg1_attack_ms;
    dx->m_global_synth_params.eg1_params.decay_time_msec =
        dx->m_settings.m_eg1_decay_ms;
    dx->m_global_synth_params.eg1_params.sustain_level =
        dx->m_settings.m_eg1_sustain_lvl;
    dx->m_global_synth_params.eg1_params.release_time_msec =
        dx->m_settings.m_eg1_release_ms;
    dx->m_global_synth_params.eg1_params.reset_to_zero =
        (bool)dx->m_settings.m_reset_to_zero;
    dx->m_global_synth_params.eg1_params.legato_mode =
        (bool)dx->m_settings.m_legato_mode;

    // EG2
    dx->m_global_synth_params.eg2_params.attack_time_msec =
        dx->m_settings.m_eg2_attack_ms;
    dx->m_global_synth_params.eg2_params.decay_time_msec =
        dx->m_settings.m_eg2_decay_ms;
    dx->m_global_synth_params.eg2_params.sustain_level =
        dx->m_settings.m_eg2_sustain_lvl;
    dx->m_global_synth_params.eg2_params.release_time_msec =
        dx->m_settings.m_eg2_release_ms;
    dx->m_global_synth_params.eg2_params.reset_to_zero =
        (bool)dx->m_settings.m_reset_to_zero;
    dx->m_global_synth_params.eg2_params.legato_mode =
        (bool)dx->m_settings.m_legato_mode;

    // EG3
    dx->m_global_synth_params.eg3_params.attack_time_msec =
        dx->m_settings.m_eg3_attack_ms;
    dx->m_global_synth_params.eg3_params.decay_time_msec =
        dx->m_settings.m_eg3_decay_ms;
    dx->m_global_synth_params.eg3_params.sustain_level =
        dx->m_settings.m_eg3_sustain_lvl;
    dx->m_global_synth_params.eg3_params.release_time_msec =
        dx->m_settings.m_eg3_release_ms;
    dx->m_global_synth_params.eg3_params.reset_to_zero =
        (bool)dx->m_settings.m_reset_to_zero;
    dx->m_global_synth_params.eg3_params.legato_mode =
        (bool)dx->m_settings.m_legato_mode;

    // EG4
    dx->m_global_synth_params.eg4_params.attack_time_msec =
        dx->m_settings.m_eg4_attack_ms;
    dx->m_global_synth_params.eg4_params.decay_time_msec =
        dx->m_settings.m_eg4_decay_ms;
    dx->m_global_synth_params.eg4_params.sustain_level =
        dx->m_settings.m_eg4_sustain_lvl;
    dx->m_global_synth_params.eg4_params.release_time_msec =
        dx->m_settings.m_eg4_release_ms;
    dx->m_global_synth_params.eg4_params.reset_to_zero =
        (bool)dx->m_settings.m_reset_to_zero;
    dx->m_global_synth_params.eg4_params.legato_mode =
        (bool)dx->m_settings.m_legato_mode;

    // LFO1
    dx->m_global_synth_params.lfo1_params.waveform =
        dx->m_settings.m_lfo1_waveform;
    dx->m_global_synth_params.lfo1_params.osc_fo = dx->m_settings.m_lfo1_rate;

    // DCA
    dx->m_global_synth_params.dca_params.amplitude_db =
        dx->m_settings.m_volume_db;

    if (dx->m_settings.m_lfo1_mod_dest1 == DX_LFO_DEST_NONE)
    {
        enable_matrix_row(&dx->m_global_modmatrix, SOURCE_LFO1,
                          DEST_OSC1_OUTPUT_AMP, false);
        enable_matrix_row(&dx->m_global_modmatrix, SOURCE_LFO1, DEST_OSC1_FO,
                          false);
    }
    else if (dx->m_settings.m_lfo1_mod_dest1 == DX_LFO_DEST_AMP_MOD)
    {
        enable_matrix_row(&dx->m_global_modmatrix, SOURCE_LFO1,
                          DEST_OSC1_OUTPUT_AMP, true);
        enable_matrix_row(&dx->m_global_modmatrix, SOURCE_LFO1, DEST_OSC1_FO,
                          false);
    }
    else // vibrato
    {
        enable_matrix_row(&dx->m_global_modmatrix, SOURCE_LFO1,
                          DEST_OSC1_OUTPUT_AMP, true);
        enable_matrix_row(&dx->m_global_modmatrix, SOURCE_LFO1, DEST_OSC1_FO,
                          false);
    }

    // LFO1 DEST2
    if (dx->m_settings.m_lfo1_mod_dest2 == DX_LFO_DEST_NONE)
    {
        enable_matrix_row(&dx->m_global_modmatrix, SOURCE_LFO1,
                          DEST_OSC2_OUTPUT_AMP, false);
        enable_matrix_row(&dx->m_global_modmatrix, SOURCE_LFO1, DEST_OSC2_FO,
                          false);
    }
    else if (dx->m_settings.m_lfo1_mod_dest2 == DX_LFO_DEST_AMP_MOD)
    {
        enable_matrix_row(&dx->m_global_modmatrix, SOURCE_LFO1,
                          DEST_OSC2_OUTPUT_AMP, true);
        enable_matrix_row(&dx->m_global_modmatrix, SOURCE_LFO1, DEST_OSC2_FO,
                          false);
    }
    else // vibrato
    {
        enable_matrix_row(&dx->m_global_modmatrix, SOURCE_LFO1,
                          DEST_OSC2_OUTPUT_AMP, true);
        enable_matrix_row(&dx->m_global_modmatrix, SOURCE_LFO1, DEST_OSC2_FO,
                          false);
    }

    // LFO1 DEST3
    if (dx->m_settings.m_lfo1_mod_dest3 == DX_LFO_DEST_NONE)
    {
        enable_matrix_row(&dx->m_global_modmatrix, SOURCE_LFO1,
                          DEST_OSC3_OUTPUT_AMP, false);
        enable_matrix_row(&dx->m_global_modmatrix, SOURCE_LFO1, DEST_OSC3_FO,
                          false);
    }
    else if (dx->m_settings.m_lfo1_mod_dest3 == DX_LFO_DEST_AMP_MOD)
    {
        enable_matrix_row(&dx->m_global_modmatrix, SOURCE_LFO1,
                          DEST_OSC3_OUTPUT_AMP, true);
        enable_matrix_row(&dx->m_global_modmatrix, SOURCE_LFO1, DEST_OSC3_FO,
                          false);
    }
    else // vibrato
    {
        enable_matrix_row(&dx->m_global_modmatrix, SOURCE_LFO1,
                          DEST_OSC3_OUTPUT_AMP, true);
        enable_matrix_row(&dx->m_global_modmatrix, SOURCE_LFO1, DEST_OSC3_FO,
                          false);
    }

    // LFO1 DEST4
    if (dx->m_settings.m_lfo1_mod_dest4 == DX_LFO_DEST_NONE)
    {
        enable_matrix_row(&dx->m_global_modmatrix, SOURCE_LFO1,
                          DEST_OSC4_OUTPUT_AMP, false);
        enable_matrix_row(&dx->m_global_modmatrix, SOURCE_LFO1, DEST_OSC4_FO,
                          false);
    }
    else if (dx->m_settings.m_lfo1_mod_dest4 == DX_LFO_DEST_AMP_MOD)
    {
        enable_matrix_row(&dx->m_global_modmatrix, SOURCE_LFO1,
                          DEST_OSC4_OUTPUT_AMP, true);
        enable_matrix_row(&dx->m_global_modmatrix, SOURCE_LFO1, DEST_OSC4_FO,
                          false);
    }
    else // vibrato
    {
        enable_matrix_row(&dx->m_global_modmatrix, SOURCE_LFO1,
                          DEST_OSC4_OUTPUT_AMP, true);
        enable_matrix_row(&dx->m_global_modmatrix, SOURCE_LFO1, DEST_OSC4_FO,
                          false);
    }
}

void dxsynth_reset_voices(dxsynth *ms)
{
    for (int i = 0; i < MAX_DX_VOICES; i++)
    {
        dxsynth_voice_reset(ms->m_voices[i]);
    }
}

void dxsynth_increment_voice_timestamps(dxsynth *ms)
{
    for (int i = 0; i < MAX_DX_VOICES; i++)
    {
        if (ms->m_voices[i])
        {
            if (ms->m_voices[i]->m_voice.m_note_on)
                ms->m_voices[i]->m_voice.m_timestamp++;
        }
    }
}

dxsynth_voice *dxsynth_get_oldest_voice(dxsynth *ms)
{
    int timestamp = -1;
    dxsynth_voice *found_voice = NULL;
    for (int i = 0; i < MAX_DX_VOICES; i++)
    {
        if (ms->m_voices[i])
        {
            if (ms->m_voices[i]->m_voice.m_note_on &&
                (int)ms->m_voices[i]->m_voice.m_timestamp > timestamp)
            {
                found_voice = ms->m_voices[i];
                timestamp = (int)ms->m_voices[i]->m_voice.m_timestamp;
            }
        }
    }
    return found_voice;
}

dxsynth_voice *dxsynth_get_oldest_voice_with_note(dxsynth *ms,
                                                  unsigned int midi_note)
{
    int timestamp = -1;
    dxsynth_voice *found_voice = NULL;
    for (int i = 0; i < MAX_DX_VOICES; i++)
    {
        if (ms->m_voices[i])
        {
            if (voice_can_note_off(&ms->m_voices[i]->m_voice) &&
                (int)ms->m_voices[i]->m_voice.m_timestamp > timestamp &&
                ms->m_voices[i]->m_voice.m_midi_note_number == midi_note)
            {
                found_voice = ms->m_voices[i];
                timestamp = (int)ms->m_voices[i]->m_voice.m_timestamp;
            }
        }
    }
    return found_voice;
}

void dxsynth::randomize()
{
    // dxsynth_reset(dx);
    // return;
    // printf("Randomizing DXSYNTH!\n");

    m_settings.m_voice_mode = rand() % 8;
    m_settings.m_portamento_time_ms = rand() % 5000;
    m_settings.m_pitchbend_range = (rand() % 12) + 1;
    // m_settings.m_velocity_to_attack_scaling = rand() % 2;
    m_settings.m_note_number_to_decay_scaling = rand() % 2;
    m_settings.m_reset_to_zero = rand() % 2;
    m_settings.m_legato_mode = rand() % 2;

    m_settings.m_lfo1_intensity = ((float)rand()) / RAND_MAX;
    m_settings.m_lfo1_rate = 0.02 + ((float)rand()) / (RAND_MAX / 20);
    m_settings.m_lfo1_waveform = rand() % MAX_LFO_OSC;
    m_settings.m_lfo1_mod_dest1 = rand() % 3;
    m_settings.m_lfo1_mod_dest2 = rand() % 3;
    m_settings.m_lfo1_mod_dest3 = rand() % 3;
    m_settings.m_lfo1_mod_dest4 = rand() % 3;

    m_settings.m_op1_waveform = rand() % MAX_OSC;
    m_settings.m_op1_ratio = 0.1 + ((float)rand()) / (RAND_MAX / 10);
    m_settings.m_op1_detune_cents = (rand() % 20) - 10;
    m_settings.m_eg1_attack_ms = rand() % 300;
    m_settings.m_eg1_decay_ms = rand() % 300;
    m_settings.m_eg1_sustain_lvl = ((float)rand()) / RAND_MAX;
    m_settings.m_eg1_release_ms = rand() % 300;
    // m_settings.m_op1_output_lvl = (rand() % 55) + 35;

    m_settings.m_op2_waveform = rand() % MAX_OSC;
    m_settings.m_op2_ratio = 0.1 + ((float)rand()) / (RAND_MAX / 10);
    m_settings.m_op2_detune_cents = (rand() % 20) - 10;
    m_settings.m_eg2_attack_ms = rand() % 300;
    m_settings.m_eg2_decay_ms = rand() % 400;
    m_settings.m_eg2_sustain_lvl = ((float)rand()) / RAND_MAX;
    m_settings.m_eg2_release_ms = rand() % 400;
    m_settings.m_op2_output_lvl = (rand() % 55) + 15;

    m_settings.m_op3_waveform = rand() % MAX_OSC;
    m_settings.m_op3_ratio = 0.1 + ((float)rand()) / (RAND_MAX / 10);
    m_settings.m_op3_detune_cents = (rand() % 20) - 10;
    m_settings.m_eg3_attack_ms = rand() % 300;
    m_settings.m_eg3_decay_ms = rand() % 400;
    m_settings.m_eg3_sustain_lvl = ((float)rand()) / RAND_MAX;
    m_settings.m_eg3_release_ms = rand() % 400;
    m_settings.m_op3_output_lvl = (rand() % 55) + 15;

    m_settings.m_op4_waveform = rand() % MAX_OSC;
    m_settings.m_op4_ratio = 0.1 + ((float)rand()) / (RAND_MAX / 10);
    m_settings.m_op4_detune_cents = (rand() % 20) - 10;
    m_settings.m_eg4_attack_ms = rand() % 400;
    m_settings.m_eg4_decay_ms = rand() % 500;
    m_settings.m_eg4_sustain_lvl = ((float)rand()) / RAND_MAX;
    m_settings.m_eg4_release_ms = rand() % 500;
    m_settings.m_op4_output_lvl = (rand() % 55) + 15;
    m_settings.m_op4_feedback = rand() % 70;

    printf("UPDATE!\n");
    dxsynth_update(this);
    // dxsynth_print_settings(dx);
}

bool dxsynth_save_settings(dxsynth *ms, char *preset_name)
{
    if (strlen(preset_name) == 0)
    {
        printf("Play tha game, pal, need a name to save yer synth settings "
               "with\n");
        return false;
    }
    printf("Saving '%s' settings for dxsynth to file %s\n", preset_name,
           DX_PRESET_FILENAME);
    FILE *presetzzz = fopen(DX_PRESET_FILENAME, "a+");
    if (presetzzz == NULL)
    {
        printf("Couldn't save settings!!\n");
        return false;
    }

    int settings_count = 0;
    strncpy(ms->m_settings.m_settings_name, preset_name, 256);

    fprintf(presetzzz, "::name=%s", ms->m_settings.m_settings_name);
    settings_count++;

    fprintf(presetzzz, "::m_lfo1_intensity=%f",
            ms->m_settings.m_lfo1_intensity);
    settings_count++;
    fprintf(presetzzz, "::m_lfo1_rate=%f", ms->m_settings.m_lfo1_rate);
    settings_count++;
    fprintf(presetzzz, "::m_lfo1_waveform=%d", ms->m_settings.m_lfo1_waveform);
    settings_count++;
    fprintf(presetzzz, "::m_lfo1_mod_dest1=%d",
            ms->m_settings.m_lfo1_mod_dest1);
    settings_count++;
    fprintf(presetzzz, "::m_lfo1_mod_dest2=%d",
            ms->m_settings.m_lfo1_mod_dest2);
    settings_count++;
    fprintf(presetzzz, "::m_lfo1_mod_dest3=%d",
            ms->m_settings.m_lfo1_mod_dest3);
    settings_count++;
    fprintf(presetzzz, "::m_lfo1_mod_dest4=%d",
            ms->m_settings.m_lfo1_mod_dest4);
    settings_count++;

    fprintf(presetzzz, "::m_op1_waveform=%d", ms->m_settings.m_op1_waveform);
    settings_count++;
    fprintf(presetzzz, "::m_op1_ratio=%f", ms->m_settings.m_op1_ratio);
    settings_count++;
    fprintf(presetzzz, "::m_op1_detune_cents=%f",
            ms->m_settings.m_op1_detune_cents);
    settings_count++;
    fprintf(presetzzz, "::m_eg1_attack_ms=%f", ms->m_settings.m_eg1_attack_ms);
    settings_count++;
    fprintf(presetzzz, "::m_eg1_decay_ms=%f", ms->m_settings.m_eg1_decay_ms);
    settings_count++;
    fprintf(presetzzz, "::m_eg1_sustain_lvl=%f",
            ms->m_settings.m_eg1_sustain_lvl);
    settings_count++;
    fprintf(presetzzz, "::m_eg1_release_ms=%f",
            ms->m_settings.m_eg1_release_ms);
    settings_count++;
    fprintf(presetzzz, "::m_op1_output_lvl=%f",
            ms->m_settings.m_op1_output_lvl);
    settings_count++;

    fprintf(presetzzz, "::m_op2_waveform=%d", ms->m_settings.m_op2_waveform);
    settings_count++;
    fprintf(presetzzz, "::m_op2_ratio=%f", ms->m_settings.m_op2_ratio);
    settings_count++;
    fprintf(presetzzz, "::m_op2_detune_cents=%f",
            ms->m_settings.m_op2_detune_cents);
    settings_count++;
    fprintf(presetzzz, "::m_eg2_attack_ms=%f", ms->m_settings.m_eg2_attack_ms);
    settings_count++;
    fprintf(presetzzz, "::m_eg2_decay_ms=%f", ms->m_settings.m_eg2_decay_ms);
    settings_count++;
    fprintf(presetzzz, "::m_eg2_sustain_lvl=%f",
            ms->m_settings.m_eg2_sustain_lvl);
    settings_count++;
    fprintf(presetzzz, "::m_eg2_release_ms=%f",
            ms->m_settings.m_eg2_release_ms);
    settings_count++;
    fprintf(presetzzz, "::m_op2_output_lvl=%f",
            ms->m_settings.m_op2_output_lvl);
    settings_count++;

    fprintf(presetzzz, "::m_op3_waveform=%d", ms->m_settings.m_op3_waveform);
    settings_count++;
    fprintf(presetzzz, "::m_op3_ratio=%f", ms->m_settings.m_op3_ratio);
    settings_count++;
    fprintf(presetzzz, "::m_op3_detune_cents=%f",
            ms->m_settings.m_op3_detune_cents);
    settings_count++;
    fprintf(presetzzz, "::m_eg3_attack_ms=%f", ms->m_settings.m_eg3_attack_ms);
    settings_count++;
    fprintf(presetzzz, "::m_eg3_decay_ms=%f", ms->m_settings.m_eg3_decay_ms);
    settings_count++;
    fprintf(presetzzz, "::m_eg3_sustain_lvl=%f",
            ms->m_settings.m_eg3_sustain_lvl);
    settings_count++;
    fprintf(presetzzz, "::m_eg3_release_ms=%f",
            ms->m_settings.m_eg3_release_ms);
    settings_count++;
    fprintf(presetzzz, "::m_op3_output_lvl=%f",
            ms->m_settings.m_op3_output_lvl);
    settings_count++;

    fprintf(presetzzz, "::m_op4_waveform=%d", ms->m_settings.m_op4_waveform);
    settings_count++;
    fprintf(presetzzz, "::m_op4_ratio=%f", ms->m_settings.m_op4_ratio);
    settings_count++;
    fprintf(presetzzz, "::m_op4_detune_cents=%f",
            ms->m_settings.m_op4_detune_cents);
    settings_count++;
    fprintf(presetzzz, "::m_eg4_attack_ms=%f", ms->m_settings.m_eg4_attack_ms);
    settings_count++;
    fprintf(presetzzz, "::m_eg4_decay_ms=%f", ms->m_settings.m_eg4_decay_ms);
    settings_count++;
    fprintf(presetzzz, "::m_eg4_sustain_lvl=%f",
            ms->m_settings.m_eg4_sustain_lvl);
    settings_count++;
    fprintf(presetzzz, "::m_eg4_release_ms=%f",
            ms->m_settings.m_eg4_release_ms);
    settings_count++;
    fprintf(presetzzz, "::m_op4_output_lvl=%f",
            ms->m_settings.m_op4_output_lvl);
    settings_count++;
    fprintf(presetzzz, "::m_op4_feedback=%f", ms->m_settings.m_op4_feedback);
    settings_count++;

    fprintf(presetzzz, "::m_portamento_time_ms=%f",
            ms->m_settings.m_portamento_time_ms);
    settings_count++;
    fprintf(presetzzz, "::m_volume_db=%f", ms->m_settings.m_volume_db);
    settings_count++;
    fprintf(presetzzz, "::m_pitchbend_range=%d",
            ms->m_settings.m_pitchbend_range);
    settings_count++;
    fprintf(presetzzz, "::m_voice_mode=%d", ms->m_settings.m_voice_mode);
    settings_count++;
    fprintf(presetzzz, "::m_velocity_to_attack_scaling=%d",
            ms->m_settings.m_velocity_to_attack_scaling);
    settings_count++;
    fprintf(presetzzz, "::m_note_number_to_decay_scaling=%d",
            ms->m_settings.m_note_number_to_decay_scaling);
    settings_count++;
    fprintf(presetzzz, "::m_reset_to_zero=%d", ms->m_settings.m_reset_to_zero);
    settings_count++;
    fprintf(presetzzz, "::m_legato_mode=%d", ms->m_settings.m_legato_mode);
    settings_count++;

    fprintf(presetzzz, ":::\n");
    fclose(presetzzz);
    printf("Wrote %d settings\n", settings_count++);
    return true;
}

bool dxsynth_list_presets()
{
    FILE *presetzzz = fopen(DX_PRESET_FILENAME, "r+");
    if (presetzzz == NULL)
        return false;

    char line[256];
    while (fgets(line, sizeof(line), presetzzz))
    {
        printf("%s\n", line);
    }

    fclose(presetzzz);

    return true;
}

bool dxsynth_check_if_preset_exists(char *preset_to_find)
{
    FILE *presetzzz = fopen(DX_PRESET_FILENAME, "r+");
    if (presetzzz == NULL)
        return false;

    char line[2048];
    char const *sep = "::";
    char *preset_name, *last_s;

    while (fgets(line, sizeof(line), presetzzz))
    {
        preset_name = strtok_r(line, sep, &last_s);
        if (strncmp(preset_to_find, preset_name, 255) == 0)
            return true;
    }

    fclose(presetzzz);
    return false;
}
bool dxsynth_load_settings(dxsynth *ms, char *preset_to_load)
{
    if (strlen(preset_to_load) == 0)
    {
        printf("Play tha game, pal, need a name to LOAD yer synth settings "
               "with\n");
        return false;
    }

    char line[2048];
    char setting_key[512];
    char setting_val[512];
    double scratch_val = 0.;

    FILE *presetzzz = fopen(DX_PRESET_FILENAME, "r+");
    if (presetzzz == NULL)
        return false;

    char *tok, *last_tok;
    char const *sep = "::";

    while (fgets(line, sizeof(line), presetzzz))
    {
        int settings_count = 0;

        for (tok = strtok_r(line, sep, &last_tok); tok;
             tok = strtok_r(NULL, sep, &last_tok))
        {
            sscanf(tok, "%[^=]=%s", setting_key, setting_val);
            sscanf(setting_val, "%lf", &scratch_val);
            // printf("key:%s val:%f\n", setting_key, scratch_val);
            if (strcmp(setting_key, "name") == 0)
            {
                if (strcmp(setting_val, preset_to_load) != 0)
                    break;
                strcpy(ms->m_settings.m_settings_name, setting_val);
                settings_count++;
            }
            else if (strcmp(setting_key, "m_lfo1_intensity") == 0)
            {
                ms->m_settings.m_lfo1_intensity = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_lfo1_rate") == 0)
            {
                ms->m_settings.m_lfo1_rate = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_lfo1_waveform") == 0)
            {
                ms->m_settings.m_lfo1_waveform = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_lfo1_mod_dest1") == 0)
            {
                ms->m_settings.m_lfo1_mod_dest1 = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_lfo1_mod_dest2") == 0)
            {
                ms->m_settings.m_lfo1_mod_dest2 = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_lfo1_mod_dest3") == 0)
            {
                ms->m_settings.m_lfo1_mod_dest3 = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_lfo1_mod_dest4") == 0)
            {
                ms->m_settings.m_lfo1_mod_dest4 = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_op1_waveform") == 0)
            {
                ms->m_settings.m_op1_waveform = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_op1_ratio") == 0)
            {
                ms->m_settings.m_op1_ratio = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_op1_detune_cents") == 0)
            {
                ms->m_settings.m_op1_detune_cents = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_eg1_attack_ms") == 0)
            {
                ms->m_settings.m_eg1_attack_ms = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_eg1_decay_ms") == 0)
            {
                ms->m_settings.m_eg1_decay_ms = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_eg1_sustain_lvl") == 0)
            {
                ms->m_settings.m_eg1_sustain_lvl = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_eg1_release_ms") == 0)
            {
                ms->m_settings.m_eg1_release_ms = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_op1_output_lvl") == 0)
            {
                ms->m_settings.m_op1_output_lvl = scratch_val;
                settings_count++;
            }

            else if (strcmp(setting_key, "m_op2_waveform") == 0)
            {
                ms->m_settings.m_op2_waveform = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_op2_ratio") == 0)
            {
                ms->m_settings.m_op2_ratio = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_op2_detune_cents") == 0)
            {
                ms->m_settings.m_op2_detune_cents = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_eg2_attack_ms") == 0)
            {
                ms->m_settings.m_eg2_attack_ms = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_eg2_decay_ms") == 0)
            {
                ms->m_settings.m_eg2_decay_ms = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_eg2_sustain_lvl") == 0)
            {
                ms->m_settings.m_eg2_sustain_lvl = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_eg2_release_ms") == 0)
            {
                ms->m_settings.m_eg2_release_ms = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_op2_output_lvl") == 0)
            {
                ms->m_settings.m_op2_output_lvl = scratch_val;
                settings_count++;
            }

            else if (strcmp(setting_key, "m_op3_waveform") == 0)
            {
                ms->m_settings.m_op3_waveform = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_op3_ratio") == 0)
            {
                ms->m_settings.m_op3_ratio = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_op3_detune_cents") == 0)
            {
                ms->m_settings.m_op3_detune_cents = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_eg3_attack_ms") == 0)
            {
                ms->m_settings.m_eg3_attack_ms = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_eg3_decay_ms") == 0)
            {
                ms->m_settings.m_eg3_decay_ms = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_eg3_sustain_lvl") == 0)
            {
                ms->m_settings.m_eg3_sustain_lvl = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_eg3_release_ms") == 0)
            {
                ms->m_settings.m_eg3_release_ms = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_op3_output_lvl") == 0)
            {
                ms->m_settings.m_op3_output_lvl = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_op4_waveform") == 0)
            {
                ms->m_settings.m_op4_waveform = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_op4_ratio") == 0)
            {
                ms->m_settings.m_op4_ratio = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_op4_detune_cents") == 0)
            {
                ms->m_settings.m_op4_detune_cents = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_eg4_attack_ms") == 0)
            {
                ms->m_settings.m_eg4_attack_ms = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_eg4_decay_ms") == 0)
            {
                ms->m_settings.m_eg4_decay_ms = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_eg4_sustain_lvl") == 0)
            {
                ms->m_settings.m_eg4_sustain_lvl = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_eg4_release_ms") == 0)
            {
                ms->m_settings.m_eg4_release_ms = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_op4_output_lvl") == 0)
            {
                ms->m_settings.m_op4_output_lvl = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_op4_feedback") == 0)
            {
                ms->m_settings.m_op4_feedback = scratch_val;
                settings_count++;
            }

            else if (strcmp(setting_key, "m_portamento_time_ms") == 0)
            {
                ms->m_settings.m_portamento_time_ms = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_volume_db") == 0)
            {
                ms->m_settings.m_volume_db = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_pitchbend_range") == 0)
            {
                ms->m_settings.m_pitchbend_range = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_voice_mode") == 0)
            {
                ms->m_settings.m_voice_mode = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_velocity_to_attack_scaling") == 0)
            {
                ms->m_settings.m_velocity_to_attack_scaling = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_note_number_to_decay_scaling") == 0)
            {
                ms->m_settings.m_note_number_to_decay_scaling = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_reset_to_zero") == 0)
            {
                ms->m_settings.m_reset_to_zero = scratch_val;
                settings_count++;
            }
            else if (strcmp(setting_key, "m_legato_mode") == 0)
            {
                ms->m_settings.m_legato_mode = scratch_val;
                settings_count++;
            }
        }
        // if (settings_count > 0)
        //    printf("Loaded %d settings\n", settings_count);
        dxsynth_update(ms);
    }

    fclose(presetzzz);
    return true;
}

void dxsynth_print_settings(dxsynth *ms)
{
    printf(ANSI_COLOR_WHITE); // CONTROL PANEL
    printf("///////////////////// SYNTHzzz! ///////////////////////\n");

    printf(ANSI_COLOR_RESET);
}

void dxsynth_print_patterns(dxsynth *ms)
{
    sequence_engine_print_patterns(&ms->engine);
}

void dxsynth_print_modulation_routings(dxsynth *ms)
{
    print_modulation_matrix(&ms->m_global_modmatrix);
}

void dxsynth_set_lfo1_intensity(dxsynth *d, double val)
{
    if (val >= 0.0 && val <= 1.0)
        d->m_settings.m_lfo1_intensity = val;
    else
        printf("Val has to be between 0.0-1.0\n");
}

void dxsynth_set_lfo1_rate(dxsynth *d, double val)
{
    if (val >= 0.02 && val <= 20.0)
        d->m_settings.m_lfo1_rate = val;
    else
        printf("Val has to be between 0.02 - 20.0\n");
}

void dxsynth_set_lfo1_waveform(dxsynth *d, unsigned int val)
{
    if (val < MAX_LFO_OSC)
        d->m_settings.m_lfo1_waveform = val;
    else
        printf("Val has to be between [0-%d]\n", MAX_LFO_OSC);
}

void dxsynth_set_lfo1_mod_dest(dxsynth *d, unsigned int mod_dest,
                               unsigned int dest)
{
    if (dest > 2)
    {
        printf("Dest has to be [0-2]\n");
        return;
    }
    switch (mod_dest)
    {
    case (1):
        d->m_settings.m_lfo1_mod_dest1 = dest;
        break;
    case (2):
        d->m_settings.m_lfo1_mod_dest2 = dest;
        break;
    case (3):
        d->m_settings.m_lfo1_mod_dest3 = dest;
        break;
    case (4):
        d->m_settings.m_lfo1_mod_dest4 = dest;
        break;
    default:
        printf("Huh?! Only got 4 destinations, brah..\n");
    }
}

void dxsynth_set_op_waveform(dxsynth *d, unsigned int op, unsigned int val)
{
    if (val >= MAX_OSC)
    {
        printf("WAV has to be [0-%d)\n", MAX_OSC);
        return;
    }
    switch (op)
    {
    case (1):
        d->m_settings.m_op1_waveform = val;
        break;
    case (2):
        d->m_settings.m_op2_waveform = val;
        break;
    case (3):
        d->m_settings.m_op3_waveform = val;
        break;
    case (4):
        d->m_settings.m_op4_waveform = val;
        break;
    default:
        printf("Huh?! Only got 4 operators, brah..\n");
    }
}

void dxsynth_set_op_ratio(dxsynth *d, unsigned int op, double val)
{
    if (val < 0.01 || val > 10)
    {
        printf("val has to be [0.01-10]\n");
        return;
    }
    switch (op)
    {
    case (1):
        d->m_settings.m_op1_ratio = val;
        break;
    case (2):
        d->m_settings.m_op2_ratio = val;
        break;
    case (3):
        d->m_settings.m_op3_ratio = val;
        break;
    case (4):
        d->m_settings.m_op4_ratio = val;
        break;
    default:
        printf("Huh?! Only got 4 operators, brah..\n");
    }
}
void dxsynth_set_op_detune(dxsynth *d, unsigned int op, double val)
{
    if (val < -100 || val > 100)
    {
        printf("val has to be [-100-100]\n");
        return;
    }
    switch (op)
    {
    case (1):
        d->m_settings.m_op1_detune_cents = val;
        break;
    case (2):
        d->m_settings.m_op2_detune_cents = val;
        break;
    case (3):
        d->m_settings.m_op3_detune_cents = val;
        break;
    case (4):
        d->m_settings.m_op4_detune_cents = val;
        break;
    default:
        printf("Huh?! Only got 4 operators, brah..\n");
    }
}

void dxsynth_set_eg_attack_ms(dxsynth *d, unsigned int eg, double val)
{
    if (val < EG_MINTIME_MS || val > EG_MAXTIME_MS)
    {
        printf("val has to be [%d - %d]\n", EG_MINTIME_MS, EG_MAXTIME_MS);
        return;
    }
    switch (eg)
    {
    case (1):
        d->m_settings.m_eg1_attack_ms = val;
        break;
    case (2):
        d->m_settings.m_eg2_attack_ms = val;
        break;
    case (3):
        d->m_settings.m_eg3_attack_ms = val;
        break;
    case (4):
        d->m_settings.m_eg4_attack_ms = val;
        break;
    default:
        printf("Huh?! Only got 4 operators, brah..\n");
    }
}

void dxsynth_set_eg_decay_ms(dxsynth *d, unsigned int eg, double val)
{
    if (val < EG_MINTIME_MS || val > EG_MAXTIME_MS)
    {
        printf("val has to be [%d - %d]\n", EG_MINTIME_MS, EG_MAXTIME_MS);
        return;
    }
    switch (eg)
    {
    case (1):
        d->m_settings.m_eg1_decay_ms = val;
        break;
    case (2):
        d->m_settings.m_eg2_decay_ms = val;
        break;
    case (3):
        d->m_settings.m_eg3_decay_ms = val;
        break;
    case (4):
        d->m_settings.m_eg4_decay_ms = val;
        break;
    default:
        printf("Huh?! Only got 4 operators, brah..\n");
    }
}

void dxsynth_set_eg_release_ms(dxsynth *d, unsigned int eg, double val)
{
    if (val < EG_MINTIME_MS || val > EG_MAXTIME_MS)
    {
        printf("val has to be [%d - %d]\n", EG_MINTIME_MS, EG_MAXTIME_MS);
        return;
    }
    switch (eg)
    {
    case (1):
        d->m_settings.m_eg1_release_ms = val;
        break;
    case (2):
        d->m_settings.m_eg2_release_ms = val;
        break;
    case (3):
        d->m_settings.m_eg3_release_ms = val;
        break;
    case (4):
        d->m_settings.m_eg4_release_ms = val;
        break;
    default:
        printf("Huh?! Only got 4 operators, brah..\n");
    }
}

void dxsynth_set_eg_sustain_lvl(dxsynth *d, unsigned int eg, double val)
{
    if (val < 0 || val > 1)
    {
        printf("val has to be [0-1]\n");
        return;
    }
    switch (eg)
    {
    case (1):
        d->m_settings.m_eg1_sustain_lvl = val;
        break;
    case (2):
        d->m_settings.m_eg2_sustain_lvl = val;
        break;
    case (3):
        d->m_settings.m_eg3_sustain_lvl = val;
        break;
    case (4):
        d->m_settings.m_eg4_sustain_lvl = val;
        break;
    default:
        printf("Huh?! Only got 4 operators, brah..\n");
    }
}

void dxsynth_set_op_output_lvl(dxsynth *d, unsigned int op, double val)
{
    if (val < 0 || val > 99)
    {
        printf("val has to be [0-99]\n");
        return;
    }
    switch (op)
    {
    case (1):
        d->m_settings.m_op1_output_lvl = val;
        break;
    case (2):
        d->m_settings.m_op2_output_lvl = val;
        break;
    case (3):
        d->m_settings.m_op3_output_lvl = val;
        break;
    case (4):
        d->m_settings.m_op4_output_lvl = val;
        break;
    default:
        printf("Huh?! Only got 4 operators, brah..\n");
    }
}

void dxsynth_set_portamento_time_ms(dxsynth *d, double val)
{
    if (val >= 0 && val <= 5000.0)
        d->m_settings.m_portamento_time_ms = val;
    else
        printf("Val has to be between 0 - 5000.0\n");
}

void dxsynth_set_volume_db(dxsynth *d, double val)
{
    if (val >= -96 && val <= 20)
        d->m_settings.m_volume_db = val;
    else
        printf("Val has to be between -96 and 20\n");
}

void dxsynth_set_pitchbend_range(dxsynth *d, unsigned int val)
{
    if (val <= 12)
        d->m_settings.m_pitchbend_range = val;
    else
        printf("Val has to be between 0 and 12\n");
}
void dxsynth_set_voice_mode(dxsynth *d, unsigned int val)
{
    if (val < MAXDX)
        d->m_settings.m_voice_mode = val;
    else
        printf("Val has to be [0-%d)\n", MAXDX);
}

void dxsynth_set_velocity_to_attack_scaling(dxsynth *d, bool b)
{
    d->m_settings.m_velocity_to_attack_scaling = b;
}
void dxsynth_set_note_number_to_decay_scaling(dxsynth *d, bool b)
{
    d->m_settings.m_note_number_to_decay_scaling = b;
}

void dxsynth_set_reset_to_zero(dxsynth *d, bool b)
{
    d->m_settings.m_reset_to_zero = b;
}

void dxsynth_set_legato_mode(dxsynth *d, bool b)
{
    d->m_settings.m_legato_mode = b;
}

void dxsynth_set_op4_feedback(dxsynth *d, double val)
{
    if (val >= 0 && val <= 70)
        d->m_settings.m_op4_feedback = val;
    else
        printf("Op4 feedback val has to be [0-70]\n");
}

void dxsynth_set_active_midi_osc(dxsynth *dx, int osc_num)
{
    if (osc_num >= 1 && osc_num <= 4)
        dx->active_midi_osc = osc_num;
}