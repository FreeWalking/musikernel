/*
This file is part of the PyDAW project, Copyright PyDAW Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef WAYV_LIBMODSYNTH_H
#define	WAYV_LIBMODSYNTH_H

#ifdef	__cplusplus
extern "C" {
#endif

/*Total number of LFOs, ADSRs, other envelopes, etc...
 * Used for the PolyFX mod matrix*/
#define WAYV_MODULATOR_COUNT 8
//How many modular PolyFX
#define WAYV_MODULAR_POLYFX_COUNT 4

#define WAYV_FM_MACRO_COUNT 2

#define WAYV_OSC_COUNT 6

#include "../../libmodsynth/constants.h"
#include "../../libmodsynth/lib/osc_core.h"
#include "../../libmodsynth/lib/pitch_core.h"
#include "../../libmodsynth/lib/smoother-linear.h"
#include "../../libmodsynth/modules/oscillator/osc_simple.h"
#include "../../libmodsynth/modules/oscillator/noise.h"
#include "../../libmodsynth/modules/filter/svf.h"
#include "../../libmodsynth/modules/distortion/clipper.h"
#include "../../libmodsynth/modules/modulation/adsr.h"
#include "../../libmodsynth/modules/signal_routing/audio_xfade.h"
#include "../../libmodsynth/modules/modulation/ramp_env.h"
#include "../../libmodsynth/modules/oscillator/lfo_simple.h"
#include "../../libmodsynth/modules/oscillator/osc_wavetable.h"
#include "../../libmodsynth/modules/multifx/multifx3knob.h"
#include "../../libmodsynth/modules/modulation/perc_env.h"

typedef struct
{
    t_wt_wavetables * wavetables;
    t_smoother_linear * pitchbend_smoother;
    t_smoother_linear * fm_macro_smoother[WAYV_FM_MACRO_COUNT];
    int reset_wavetables;
}t_wayv_mono_modules;


typedef struct
{
    t_osc_wav_unison * osc_wavtable[WAYV_OSC_COUNT];

    float osc_uni_spread[WAYV_OSC_COUNT];

    float osc_fm[WAYV_OSC_COUNT][WAYV_OSC_COUNT];

    float osc_macro_amp[2][WAYV_OSC_COUNT];

    float fm_osc_values[WAYV_OSC_COUNT][WAYV_OSC_COUNT];

    float fm_last[WAYV_OSC_COUNT];

    t_white_noise * white_noise1;
    t_adsr * adsr_main;

    t_adsr * adsr_amp_osc[WAYV_OSC_COUNT];
    int adsr_amp_on[WAYV_OSC_COUNT];

    float osc_linamp[WAYV_OSC_COUNT];
    int osc_audible[WAYV_OSC_COUNT];
    int osc_on[WAYV_OSC_COUNT];

    float noise_amp;

    t_smoother_linear * glide_smoother;
    t_ramp_env * glide_env;

    //For simplicity, this is used whether glide is turned on or not
    float last_pitch;
    //base pitch for all oscillators, to avoid redundant calculations
    float base_pitch;
    float target_pitch;
    /*This corresponds to the current sample being processed on this voice.
     * += this to the output buffer when finished.*/
    float current_sample;

    float   amp;
    float master_vol_lin;
    float note_f;
    int note;

    float noise_linamp;

    t_mf3_multi * multieffect[WAYV_MODULAR_POLYFX_COUNT];
    fp_mf3_run fx_func_ptr[WAYV_MODULAR_POLYFX_COUNT];
    float modulex_current_sample[2];
    float * modulator_outputs[WAYV_MODULATOR_COUNT];
    fp_noise_func_ptr noise_func_ptr;
    t_lfs_lfo * lfo1;
    float lfo_amount_output, lfo_amp_output, lfo_pitch_output;
    t_adsr * adsr_filter;
    t_adsr * adsr_amp;
    t_adsr * adsr_noise;
    t_adsr * adsr_lfo;
    int adsr_noise_on;
    int noise_prefx;
    int adsr_lfo_on;
    t_ramp_env * ramp_env;

    float filter_output;
    float noise_sample;



    float velocity_track;
    float keyboard_track;

    int adsr_prefx;
    int perc_env_on;
    t_pnv_perc_env * perc_env;
}t_wayv_poly_voice;

t_wayv_poly_voice * g_wayv_poly_init(float a_sr, t_wayv_mono_modules* a_mono);

/*initialize all of the modules in an instance of poly_voice*/

t_wayv_poly_voice * g_wayv_poly_init(float a_sr, t_wayv_mono_modules* a_mono)
{
    t_wayv_poly_voice * f_voice =
            (t_wayv_poly_voice*)malloc(sizeof(t_wayv_poly_voice));

    int f_i = 0;

    while(f_i < WAYV_OSC_COUNT)
    {
        f_voice->osc_wavtable[f_i] = g_osc_get_osc_wav_unison(a_sr);
        f_voice->osc_uni_spread[f_i] = 0.0f;
        f_voice->osc_on[f_i] = 0;
        f_voice->fm_last[f_i] = 0.0;
        f_voice->adsr_amp_osc[f_i] = g_adsr_get_adsr(a_sr);
        f_voice->adsr_amp_on[f_i] = 0;
        f_voice->osc_linamp[f_i] = 1.0f;
        f_voice->osc_audible[f_i] = 1;

        int f_i2 = 0;
        while(f_i2 < WAYV_OSC_COUNT)
        {
            f_voice->osc_fm[f_i][f_i2] = 0.0;
            f_i2++;
        }
        f_i++;
    }


    f_voice->adsr_main = g_adsr_get_adsr(a_sr);

    f_voice->white_noise1 = g_get_white_noise(a_sr);
    f_voice->noise_amp = 0;

    f_voice->glide_env = g_rmp_get_ramp_env(a_sr);

    //f_voice->real_pitch = 60.0f;

    f_voice->target_pitch = 66.0f;
    f_voice->last_pitch = 66.0f;
    f_voice->base_pitch = 66.0f;

    f_voice->current_sample = 0.0f;

    f_voice->amp = 1.0f;
    f_voice->note_f = 1.0f;

    f_voice->noise_linamp = 1.0f;
    f_voice->adsr_prefx = 0;

    f_voice->lfo_amount_output = 0.0f;
    f_voice->lfo_amp_output = 0.0f;
    f_voice->lfo_pitch_output = 0.0f;

    int f_i2 = 0;
    while(f_i2 < WAYV_OSC_COUNT)
    {
        int f_i3 = 0;
        while(f_i3 < WAYV_OSC_COUNT)
        {
            f_voice->fm_osc_values[f_i2][f_i3] = 0.0f;
            f_i3++;
        }

        f_i2++;
    }

    f_voice->adsr_amp = g_adsr_get_adsr(a_sr);
    f_voice->adsr_filter = g_adsr_get_adsr(a_sr);
    f_voice->adsr_noise = g_adsr_get_adsr(a_sr);
    f_voice->adsr_lfo = g_adsr_get_adsr(a_sr);
    f_voice->adsr_noise_on = 0;
    f_voice->adsr_lfo_on = 0;

    f_voice->noise_amp = 0.0f;

    f_voice->glide_env = g_rmp_get_ramp_env(a_sr);
    f_voice->ramp_env = g_rmp_get_ramp_env(a_sr);

    f_voice->filter_output = 0.0f;

    f_voice->lfo1 = g_lfs_get(a_sr);

    f_voice->noise_sample = 0.0f;


    for(f_i = 0; f_i < WAYV_MODULAR_POLYFX_COUNT; f_i++)
    {
        f_voice->multieffect[f_i] = g_mf3_get(a_sr);
        f_voice->fx_func_ptr[f_i] = v_mf3_run_off;
    }

    f_voice->modulator_outputs[0] = &(f_voice->adsr_amp->output);
    f_voice->modulator_outputs[1] = &(f_voice->adsr_filter->output);
    f_voice->modulator_outputs[2] = &(f_voice->ramp_env->output);
    f_voice->modulator_outputs[3] = &(f_voice->lfo_amount_output);
    f_voice->modulator_outputs[4] = &(f_voice->keyboard_track);
    f_voice->modulator_outputs[5] = &(f_voice->velocity_track);
    f_voice->modulator_outputs[6] = &(a_mono->fm_macro_smoother[0]->last_value);
    f_voice->modulator_outputs[7] = &(a_mono->fm_macro_smoother[1]->last_value);

    f_voice->noise_func_ptr = f_run_noise_off;

    f_voice->perc_env_on = 0;
    f_voice->perc_env = g_pnv_get(a_sr);

    return f_voice;
}


void v_wayv_poly_note_off(t_wayv_poly_voice * a_voice, int a_fast);

void v_wayv_poly_note_off(t_wayv_poly_voice * a_voice, int a_fast)
{
    if(a_fast)
    {
        v_adsr_set_fast_release(a_voice->adsr_main);
    }
    else
    {
        v_adsr_release(a_voice->adsr_main);
    }

    v_adsr_release(a_voice->adsr_lfo);
    v_adsr_release(a_voice->adsr_noise);
    v_adsr_release(a_voice->adsr_amp);
    v_adsr_release(a_voice->adsr_filter);

    int f_i = 0;

    while(f_i < WAYV_OSC_COUNT)
    {
        v_adsr_release(a_voice->adsr_amp_osc[f_i]);
        f_i++;
    }

}

t_wayv_mono_modules * v_wayv_mono_init(float);


/*Initialize any modules that will be run monophonically*/
t_wayv_mono_modules * v_wayv_mono_init(float a_sr)
{
    t_wayv_mono_modules * a_mono =
            (t_wayv_mono_modules*)malloc(sizeof(t_wayv_mono_modules));
    a_mono->pitchbend_smoother =
            g_sml_get_smoother_linear(a_sr, 1.0f, -1.0f, 0.2f);

    int f_i = 0;
    while(f_i < WAYV_FM_MACRO_COUNT)
    {
        a_mono->fm_macro_smoother[f_i] =
                g_sml_get_smoother_linear(a_sr, 0.5f, 0.0f, 0.02f);
        f_i++;
    }

    a_mono->wavetables = g_wt_wavetables_get();
    //indicates that wavetables must be re-pointered immediately
    a_mono->reset_wavetables = 0;
    return a_mono;
}


#ifdef	__cplusplus
}
#endif

#endif	/* LIBMODSYNTH_H */

