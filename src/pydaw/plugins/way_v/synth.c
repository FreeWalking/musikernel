/* -*- c-basic-offset: 4 -*-  vi:set ts=8 sts=4 sw=4: */
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

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>

#include <math.h>
#include <stdio.h>

#include "../../include/pydaw_plugin.h"

#include "libmodsynth.h"
#include "../../libmodsynth/lib/amp.h"
#include "../../libmodsynth/modules/filter/svf.h"
#include "../../libmodsynth/lib/lms_math.h"
#include "synth.h"
#include "../../src/pydaw_files.h"

static void v_run_wayv(PYFX_Handle instance, int sample_count,
		  t_pydaw_seq_event * events, int EventCount);

static void v_run_wayv_voice(t_wayv *, t_voc_single_voice, t_wayv_poly_voice *,
		      PYFX_Data *, PYFX_Data *, int, int );

PYFX_Descriptor *wayv_PYFX_descriptor(int index);
PYINST_Descriptor *wayv_PYINST_descriptor(int index);


static void v_cleanup_wayv(PYFX_Handle instance)
{
    free(instance);
}

static void v_wayv_or_prep(PYFX_Handle instance)
{
    t_wayv *plugin = (t_wayv *)instance;
    int f_i = 0;
    int f_i2 = 0;
    int f_i3 = 0;

    int f_osc_on[WAYV_OSC_COUNT];

    while(f_i < WAYV_OSC_COUNT)
    {
        f_osc_on[f_i] = ((int)(*plugin->osc_type[f_i]) - 1);
        f_i++;
    }

    while(f_i2 < WAYV_POLYPHONY)
    {
        t_wayv_poly_voice * f_voice = plugin->data[f_i2];
        f_i = 0;
        while(f_i < 1000000)
        {
            f_i3 = 0;
            while(f_i3 < WAYV_OSC_COUNT)
            {
                if(f_osc_on[f_i3] >= 0)
                {
                    v_osc_wav_run_unison_core_only(f_voice->osc_wavtable[f_i3]);
                }
                f_i3++;
            }

            f_i++;
        }
        f_i2++;
    }

    plugin->mono_modules->fm_macro_smoother[0]->last_value =
            (*plugin->fm_macro[0] * 0.01f);

    plugin->mono_modules->fm_macro_smoother[1]->last_value =
            (*plugin->fm_macro[1] * 0.01f);
}

static void wayvPanic(PYFX_Handle instance)
{
    t_wayv *plugin = (t_wayv *)instance;
    int f_i = 0;
    while(f_i < WAYV_POLYPHONY)
    {
        v_adsr_kill(plugin->data[f_i]->adsr_amp);
        v_adsr_kill(plugin->data[f_i]->adsr_main);
        f_i++;
    }
}

static void v_wayv_on_stop(PYFX_Handle instance)
{
    t_wayv *plugin = (t_wayv *)instance;
    int f_i = 0;
    while(f_i < WAYV_POLYPHONY)
    {
        v_wayv_poly_note_off(plugin->data[f_i], 0);
        f_i++;
    }
}

static void v_wayv_connect_buffer(PYFX_Handle instance, int a_index,
        float * DataLocation)
{
    t_wayv *plugin = (t_wayv*)instance;

    switch(a_index)
    {
        case 0:
            plugin->output0 = DataLocation;
            break;
        case 1:
            plugin->output1 = DataLocation;
            break;
        default:
            assert(0);
            break;
    }
}

static void v_wayv_connect_port(PYFX_Handle instance, int port,
			  PYFX_Data * data)
{
    t_wayv *plugin;

    plugin = (t_wayv *) instance;

    switch (port)
    {
        case WAYV_ATTACK_MAIN:
            plugin->attack_main = data;
            break;
        case WAYV_DECAY_MAIN:
            plugin->decay_main = data;
            break;
        case WAYV_SUSTAIN_MAIN:
            plugin->sustain_main = data;
            break;
        case WAYV_RELEASE_MAIN:
            plugin->release_main = data;
            break;

        case WAYV_ATTACK1: plugin->attack[0] = data; break;
        case WAYV_DECAY1: plugin->decay[0] = data; break;
        case WAYV_SUSTAIN1: plugin->sustain[0] = data; break;
        case WAYV_RELEASE1: plugin->release[0] = data; break;
        case WAYV_ATTACK2: plugin->attack[1] = data; break;
        case WAYV_DECAY2: plugin->decay[1] = data; break;
        case WAYV_SUSTAIN2: plugin->sustain[1] = data; break;
        case WAYV_RELEASE2: plugin->release[1] = data; break;

        case WAYV_NOISE_AMP:
            plugin->noise_amp = data;
            break;
        case WAYV_MASTER_VOLUME:
            plugin->master_vol = data;
            break;
        case WAYV_OSC1_PITCH:
            plugin->osc_pitch[0] = data;
            break;
        case WAYV_OSC1_TUNE:
            plugin->osc_tune[0] = data;
            break;
        case WAYV_OSC1_TYPE: plugin->osc_type[0] = data; break;
        case WAYV_OSC1_VOLUME:
            plugin->osc_vol[0] = data;
            break;
        case WAYV_OSC2_PITCH:
            plugin->osc_pitch[1] = data;
            break;
        case WAYV_OSC2_TUNE:
            plugin->osc_tune[1] = data;
            break;
        case WAYV_OSC2_TYPE: plugin->osc_type[1] = data; break;
        case WAYV_OSC2_VOLUME:
            plugin->osc_vol[1] = data;
            break;
        case WAYV_OSC1_UNISON_VOICES:
            plugin->osc_uni_voice[0] = data;
            break;
        case WAYV_OSC1_UNISON_SPREAD:
            plugin->osc_uni_spread[0] = data;
            break;
        case WAYV_OSC2_UNISON_VOICES:
            plugin->osc_uni_voice[1] = data;
            break;
        case WAYV_OSC2_UNISON_SPREAD:
            plugin->osc_uni_spread[1] = data;
            break;
        case WAYV_OSC3_UNISON_VOICES:
            plugin->osc_uni_voice[2] = data;
            break;
        case WAYV_OSC3_UNISON_SPREAD:
            plugin->osc_uni_spread[2] = data;
            break;
        case WAYV_MASTER_GLIDE:
            plugin->master_glide = data;
            break;
        case WAYV_MASTER_PITCHBEND_AMT:
            plugin->master_pb_amt = data;
            break;


        case WAYV_ATTACK_PFX1:
            plugin->pfx_attack = data;
            break;
        case WAYV_DECAY_PFX1:
            plugin->pfx_decay = data;
            break;
        case WAYV_SUSTAIN_PFX1:
            plugin->pfx_sustain = data;
            break;
        case WAYV_RELEASE_PFX1:
            plugin->pfx_release = data;
            break;
        case WAYV_ATTACK_PFX2:
            plugin->pfx_attack_f = data;
            break;
        case WAYV_DECAY_PFX2:
            plugin->pfx_decay_f = data;
            break;
        case WAYV_SUSTAIN_PFX2:
            plugin->pfx_sustain_f = data;
            break;
        case WAYV_RELEASE_PFX2:
            plugin->pfx_release_f = data;
            break;
        case WAYV_RAMP_ENV_TIME:
            plugin->pitch_env_time = data;
            break;
        case WAYV_LFO_FREQ:
            plugin->lfo_freq = data;
            break;
        case WAYV_LFO_TYPE:
            plugin->lfo_type = data;
            break;

        case WAYV_FX0_KNOB0: plugin->pfx_mod_knob[0][0][0] = data; break;
        case WAYV_FX0_KNOB1: plugin->pfx_mod_knob[0][0][1] = data; break;
        case WAYV_FX0_KNOB2: plugin->pfx_mod_knob[0][0][2] = data; break;
        case WAYV_FX1_KNOB0: plugin->pfx_mod_knob[0][1][0] = data; break;
        case WAYV_FX1_KNOB1: plugin->pfx_mod_knob[0][1][1] = data; break;
        case WAYV_FX1_KNOB2: plugin->pfx_mod_knob[0][1][2] = data; break;
        case WAYV_FX2_KNOB0: plugin->pfx_mod_knob[0][2][0] = data; break;
        case WAYV_FX2_KNOB1: plugin->pfx_mod_knob[0][2][1] = data; break;
        case WAYV_FX2_KNOB2: plugin->pfx_mod_knob[0][2][2] = data; break;
        case WAYV_FX3_KNOB0: plugin->pfx_mod_knob[0][3][0] = data; break;
        case WAYV_FX3_KNOB1: plugin->pfx_mod_knob[0][3][1] = data; break;
        case WAYV_FX3_KNOB2: plugin->pfx_mod_knob[0][3][2] = data; break;

        case WAYV_FX0_COMBOBOX: plugin->fx_combobox[0][0] = data; break;
        case WAYV_FX1_COMBOBOX: plugin->fx_combobox[0][1] = data; break;
        case WAYV_FX2_COMBOBOX: plugin->fx_combobox[0][2] = data; break;
        case WAYV_FX3_COMBOBOX: plugin->fx_combobox[0][3] = data; break;
        //End from Modulex
        /*PolyFX mod matrix port connections*/
        case WAVV_PFXMATRIX_GRP0DST0SRC0CTRL0: plugin->polyfx_mod_matrix[0][0][0][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC0CTRL1: plugin->polyfx_mod_matrix[0][0][0][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC0CTRL2: plugin->polyfx_mod_matrix[0][0][0][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC1CTRL0: plugin->polyfx_mod_matrix[0][0][1][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC1CTRL1: plugin->polyfx_mod_matrix[0][0][1][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC1CTRL2: plugin->polyfx_mod_matrix[0][0][1][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC2CTRL0: plugin->polyfx_mod_matrix[0][0][2][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC2CTRL1: plugin->polyfx_mod_matrix[0][0][2][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC2CTRL2: plugin->polyfx_mod_matrix[0][0][2][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC3CTRL0: plugin->polyfx_mod_matrix[0][0][3][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC3CTRL1: plugin->polyfx_mod_matrix[0][0][3][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC3CTRL2: plugin->polyfx_mod_matrix[0][0][3][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC0CTRL0: plugin->polyfx_mod_matrix[0][1][0][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC0CTRL1: plugin->polyfx_mod_matrix[0][1][0][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC0CTRL2: plugin->polyfx_mod_matrix[0][1][0][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC1CTRL0: plugin->polyfx_mod_matrix[0][1][1][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC1CTRL1: plugin->polyfx_mod_matrix[0][1][1][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC1CTRL2: plugin->polyfx_mod_matrix[0][1][1][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC2CTRL0: plugin->polyfx_mod_matrix[0][1][2][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC2CTRL1: plugin->polyfx_mod_matrix[0][1][2][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC2CTRL2: plugin->polyfx_mod_matrix[0][1][2][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC3CTRL0: plugin->polyfx_mod_matrix[0][1][3][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC3CTRL1: plugin->polyfx_mod_matrix[0][1][3][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC3CTRL2: plugin->polyfx_mod_matrix[0][1][3][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC0CTRL0: plugin->polyfx_mod_matrix[0][2][0][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC0CTRL1: plugin->polyfx_mod_matrix[0][2][0][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC0CTRL2: plugin->polyfx_mod_matrix[0][2][0][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC1CTRL0: plugin->polyfx_mod_matrix[0][2][1][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC1CTRL1: plugin->polyfx_mod_matrix[0][2][1][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC1CTRL2: plugin->polyfx_mod_matrix[0][2][1][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC2CTRL0: plugin->polyfx_mod_matrix[0][2][2][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC2CTRL1: plugin->polyfx_mod_matrix[0][2][2][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC2CTRL2: plugin->polyfx_mod_matrix[0][2][2][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC3CTRL0: plugin->polyfx_mod_matrix[0][2][3][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC3CTRL1: plugin->polyfx_mod_matrix[0][2][3][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC3CTRL2: plugin->polyfx_mod_matrix[0][2][3][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC0CTRL0: plugin->polyfx_mod_matrix[0][3][0][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC0CTRL1: plugin->polyfx_mod_matrix[0][3][0][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC0CTRL2: plugin->polyfx_mod_matrix[0][3][0][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC1CTRL0: plugin->polyfx_mod_matrix[0][3][1][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC1CTRL1: plugin->polyfx_mod_matrix[0][3][1][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC1CTRL2: plugin->polyfx_mod_matrix[0][3][1][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC2CTRL0: plugin->polyfx_mod_matrix[0][3][2][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC2CTRL1: plugin->polyfx_mod_matrix[0][3][2][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC2CTRL2: plugin->polyfx_mod_matrix[0][3][2][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC3CTRL0: plugin->polyfx_mod_matrix[0][3][3][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC3CTRL1: plugin->polyfx_mod_matrix[0][3][3][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC3CTRL2: plugin->polyfx_mod_matrix[0][3][3][2] = data; break;

        //keyboard tracking
        case WAVV_PFXMATRIX_GRP0DST0SRC4CTRL0: plugin->polyfx_mod_matrix[0][0][4][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC4CTRL1: plugin->polyfx_mod_matrix[0][0][4][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC4CTRL2: plugin->polyfx_mod_matrix[0][0][4][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC4CTRL0: plugin->polyfx_mod_matrix[0][1][4][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC4CTRL1: plugin->polyfx_mod_matrix[0][1][4][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC4CTRL2: plugin->polyfx_mod_matrix[0][1][4][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC4CTRL0: plugin->polyfx_mod_matrix[0][2][4][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC4CTRL1: plugin->polyfx_mod_matrix[0][2][4][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC4CTRL2: plugin->polyfx_mod_matrix[0][2][4][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC4CTRL0: plugin->polyfx_mod_matrix[0][3][4][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC4CTRL1: plugin->polyfx_mod_matrix[0][3][4][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC4CTRL2: plugin->polyfx_mod_matrix[0][3][4][2] = data; break;

        //velocity tracking
        case WAVV_PFXMATRIX_GRP0DST0SRC5CTRL0: plugin->polyfx_mod_matrix[0][0][5][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC5CTRL1: plugin->polyfx_mod_matrix[0][0][5][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC5CTRL2: plugin->polyfx_mod_matrix[0][0][5][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC5CTRL0: plugin->polyfx_mod_matrix[0][1][5][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC5CTRL1: plugin->polyfx_mod_matrix[0][1][5][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC5CTRL2: plugin->polyfx_mod_matrix[0][1][5][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC5CTRL0: plugin->polyfx_mod_matrix[0][2][5][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC5CTRL1: plugin->polyfx_mod_matrix[0][2][5][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC5CTRL2: plugin->polyfx_mod_matrix[0][2][5][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC5CTRL0: plugin->polyfx_mod_matrix[0][3][5][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC5CTRL1: plugin->polyfx_mod_matrix[0][3][5][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC5CTRL2: plugin->polyfx_mod_matrix[0][3][5][2] = data; break;


        case WAYV_NOISE_TYPE: plugin->noise_type = data; break;
        case WAYV_ADSR1_CHECKBOX: plugin->adsr_checked[0] = data; break;
        case WAYV_ADSR2_CHECKBOX: plugin->adsr_checked[1] = data; break;

        case WAYV_LFO_AMP: plugin->lfo_amp = data; break;
        case WAYV_LFO_PITCH: plugin->lfo_pitch = data; break;

        case WAYV_PITCH_ENV_AMT: plugin->pitch_env_amt = data; break;
        case WAYV_LFO_AMOUNT: plugin->lfo_amount = data; break;

        case WAYV_OSC3_PITCH: plugin->osc_pitch[2] = data; break;
        case WAYV_OSC3_TUNE: plugin->osc_tune[2] = data; break;
        case WAYV_OSC3_TYPE: plugin->osc_type[2] = data; break;
        case WAYV_OSC3_VOLUME: plugin->osc_vol[2] = data;  break;

        case WAYV_OSC1_FM1: plugin->osc_fm[0][0] = data;  break;
        case WAYV_OSC1_FM2: plugin->osc_fm[0][1] = data;  break;
        case WAYV_OSC1_FM3: plugin->osc_fm[0][2] = data;  break;

        case WAYV_OSC2_FM1: plugin->osc_fm[1][0] = data;  break;
        case WAYV_OSC2_FM2: plugin->osc_fm[1][1] = data;  break;
        case WAYV_OSC2_FM3: plugin->osc_fm[1][2] = data;  break;

        case WAYV_OSC3_FM1: plugin->osc_fm[2][0] = data;  break;
        case WAYV_OSC3_FM2: plugin->osc_fm[2][1] = data;  break;
        case WAYV_OSC3_FM3: plugin->osc_fm[2][2] = data;  break;

        case WAYV_ATTACK3: plugin->attack[2] = data; break;
        case WAYV_DECAY3: plugin->decay[2] = data; break;
        case WAYV_SUSTAIN3: plugin->sustain[2] = data; break;
        case WAYV_RELEASE3: plugin->release[2] = data; break;

        case WAYV_ADSR3_CHECKBOX: plugin->adsr_checked[2] = data; break;

        case WAYV_PERC_ENV_PITCH1: plugin->perc_env_pitch1 = data; break;
        case WAYV_PERC_ENV_TIME1: plugin->perc_env_time1 = data; break;
        case WAYV_PERC_ENV_PITCH2: plugin->perc_env_pitch2 = data; break;
        case WAYV_PERC_ENV_TIME2: plugin->perc_env_time2 = data; break;
        case WAYV_PERC_ENV_ON: plugin->perc_env_on = data; break;

        case WAYV_RAMP_CURVE: plugin->ramp_curve = data; break;

        case WAYV_MONO_MODE: plugin->mono_mode = data; break;

        case WAYV_OSC1_FM4: plugin->osc_fm[0][3] = data;  break;
        case WAYV_OSC2_FM4: plugin->osc_fm[1][3] = data;  break;
        case WAYV_OSC3_FM4: plugin->osc_fm[2][3] = data;  break;

        case WAYV_OSC4_UNISON_VOICES: plugin->osc_uni_voice[3] = data; break;
        case WAYV_OSC4_UNISON_SPREAD: plugin->osc_uni_spread[3] = data; break;

        case WAYV_OSC4_PITCH: plugin->osc_pitch[3] = data; break;
        case WAYV_OSC4_TUNE: plugin->osc_tune[3] = data; break;
        case WAYV_OSC4_TYPE: plugin->osc_type[3] = data; break;
        case WAYV_OSC4_VOLUME: plugin->osc_vol[3] = data;  break;

        case WAYV_OSC4_FM1: plugin->osc_fm[3][0] = data;  break;
        case WAYV_OSC4_FM2: plugin->osc_fm[3][1] = data;  break;
        case WAYV_OSC4_FM3: plugin->osc_fm[3][2] = data;  break;
        case WAYV_OSC4_FM4: plugin->osc_fm[3][3] = data;  break;

        case WAYV_ATTACK4: plugin->attack[3] = data; break;
        case WAYV_DECAY4: plugin->decay[3] = data; break;
        case WAYV_SUSTAIN4: plugin->sustain[3] = data; break;
        case WAYV_RELEASE4: plugin->release[3] = data; break;

        case WAYV_ADSR4_CHECKBOX: plugin->adsr_checked[3] = data; break;

        case WAYV_FM_MACRO1: plugin->fm_macro[0] = data; break;
        case WAYV_FM_MACRO1_OSC1_FM1: plugin->fm_macro_values[0][0][0] = data; break;
        case WAYV_FM_MACRO1_OSC1_FM2: plugin->fm_macro_values[0][0][1] = data; break;
        case WAYV_FM_MACRO1_OSC1_FM3: plugin->fm_macro_values[0][0][2] = data; break;
        case WAYV_FM_MACRO1_OSC1_FM4: plugin->fm_macro_values[0][0][3] = data; break;
        case WAYV_FM_MACRO1_OSC2_FM1: plugin->fm_macro_values[0][1][0] = data; break;
        case WAYV_FM_MACRO1_OSC2_FM2: plugin->fm_macro_values[0][1][1] = data; break;
        case WAYV_FM_MACRO1_OSC2_FM3: plugin->fm_macro_values[0][1][2] = data; break;
        case WAYV_FM_MACRO1_OSC2_FM4: plugin->fm_macro_values[0][1][3] = data; break;
        case WAYV_FM_MACRO1_OSC3_FM1: plugin->fm_macro_values[0][2][0] = data; break;
        case WAYV_FM_MACRO1_OSC3_FM2: plugin->fm_macro_values[0][2][1] = data; break;
        case WAYV_FM_MACRO1_OSC3_FM3: plugin->fm_macro_values[0][2][2] = data; break;
        case WAYV_FM_MACRO1_OSC3_FM4: plugin->fm_macro_values[0][2][3] = data; break;
        case WAYV_FM_MACRO1_OSC4_FM1: plugin->fm_macro_values[0][3][0] = data; break;
        case WAYV_FM_MACRO1_OSC4_FM2: plugin->fm_macro_values[0][3][1] = data; break;
        case WAYV_FM_MACRO1_OSC4_FM3: plugin->fm_macro_values[0][3][2] = data; break;
        case WAYV_FM_MACRO1_OSC4_FM4: plugin->fm_macro_values[0][3][3] = data; break;

        case WAYV_FM_MACRO2: plugin->fm_macro[1] = data; break;
        case WAYV_FM_MACRO2_OSC1_FM1: plugin->fm_macro_values[1][0][0] = data; break;
        case WAYV_FM_MACRO2_OSC1_FM2: plugin->fm_macro_values[1][0][1] = data; break;
        case WAYV_FM_MACRO2_OSC1_FM3: plugin->fm_macro_values[1][0][2] = data; break;
        case WAYV_FM_MACRO2_OSC1_FM4: plugin->fm_macro_values[1][0][3] = data; break;
        case WAYV_FM_MACRO2_OSC2_FM1: plugin->fm_macro_values[1][1][0] = data; break;
        case WAYV_FM_MACRO2_OSC2_FM2: plugin->fm_macro_values[1][1][1] = data; break;
        case WAYV_FM_MACRO2_OSC2_FM3: plugin->fm_macro_values[1][1][2] = data; break;
        case WAYV_FM_MACRO2_OSC2_FM4: plugin->fm_macro_values[1][1][3] = data; break;
        case WAYV_FM_MACRO2_OSC3_FM1: plugin->fm_macro_values[1][2][0] = data; break;
        case WAYV_FM_MACRO2_OSC3_FM2: plugin->fm_macro_values[1][2][1] = data; break;
        case WAYV_FM_MACRO2_OSC3_FM3: plugin->fm_macro_values[1][2][2] = data; break;
        case WAYV_FM_MACRO2_OSC3_FM4: plugin->fm_macro_values[1][2][3] = data; break;
        case WAYV_FM_MACRO2_OSC4_FM1: plugin->fm_macro_values[1][3][0] = data; break;
        case WAYV_FM_MACRO2_OSC4_FM2: plugin->fm_macro_values[1][3][1] = data; break;
        case WAYV_FM_MACRO2_OSC4_FM3: plugin->fm_macro_values[1][3][2] = data; break;
        case WAYV_FM_MACRO2_OSC4_FM4: plugin->fm_macro_values[1][3][3] = data; break;

        case WAYV_FM_MACRO1_OSC1_VOL: plugin->amp_macro_values[0][0] = data; break;
        case WAYV_FM_MACRO1_OSC2_VOL: plugin->amp_macro_values[0][1] = data; break;
        case WAYV_FM_MACRO1_OSC3_VOL: plugin->amp_macro_values[0][2] = data; break;
        case WAYV_FM_MACRO1_OSC4_VOL: plugin->amp_macro_values[0][3] = data; break;

        case WAYV_FM_MACRO2_OSC1_VOL: plugin->amp_macro_values[1][0] = data; break;
        case WAYV_FM_MACRO2_OSC2_VOL: plugin->amp_macro_values[1][1] = data; break;
        case WAYV_FM_MACRO2_OSC3_VOL: plugin->amp_macro_values[1][2] = data; break;
        case WAYV_FM_MACRO2_OSC4_VOL: plugin->amp_macro_values[1][3] = data; break;

        case WAYV_LFO_PHASE: plugin->lfo_phase = data; break;
        case WAYV_LFO_PITCH_FINE: plugin->lfo_pitch_fine = data; break;
        case WAYV_ADSR_PREFX: plugin->adsr_prefx = data; break;

        case WAYV_ADSR1_DELAY: plugin->adsr_fm_delay[0] = data; break;
        case WAYV_ADSR2_DELAY: plugin->adsr_fm_delay[1] = data; break;
        case WAYV_ADSR3_DELAY: plugin->adsr_fm_delay[2] = data; break;
        case WAYV_ADSR4_DELAY: plugin->adsr_fm_delay[3] = data; break;

        case WAYV_ADSR1_HOLD: plugin->adsr_fm_hold[0] = data; break;
        case WAYV_ADSR2_HOLD: plugin->adsr_fm_hold[1] = data; break;
        case WAYV_ADSR3_HOLD: plugin->adsr_fm_hold[2] = data; break;
        case WAYV_ADSR4_HOLD: plugin->adsr_fm_hold[3] = data; break;

        case WAYV_PFX_ADSR_DELAY: plugin->pfx_delay = data; break;
        case WAYV_PFX_ADSR_F_DELAY: plugin->pfx_delay_f = data; break;

        case WAYV_PFX_ADSR_HOLD: plugin->pfx_hold = data; break;
        case WAYV_PFX_ADSR_F_HOLD: plugin->pfx_hold_f = data; break;
        case WAYV_HOLD_MAIN: plugin->hold_main = data; break;

        case WAYV_DELAY_NOISE: plugin->noise_delay = data; break;
        case WAYV_ATTACK_NOISE: plugin->noise_attack = data; break;
        case WAYV_HOLD_NOISE: plugin->noise_hold = data; break;
        case WAYV_DECAY_NOISE: plugin->noise_decay = data; break;
        case WAYV_SUSTAIN_NOISE: plugin->noise_sustain = data; break;
        case WAYV_RELEASE_NOISE: plugin->noise_release = data; break;
        case WAYV_ADSR_NOISE_ON: plugin->noise_adsr_on = data; break;

        case WAYV_DELAY_LFO: plugin->lfo_delay = data; break;
        case WAYV_ATTACK_LFO: plugin->lfo_attack = data; break;
        case WAYV_HOLD_LFO: plugin->lfo_hold = data; break;
        case WAYV_DECAY_LFO: plugin->lfo_decay = data; break;
        case WAYV_SUSTAIN_LFO: plugin->lfo_sustain = data; break;
        case WAYV_RELEASE_LFO: plugin->lfo_release = data; break;
        case WAYV_ADSR_LFO_ON: plugin->lfo_adsr_on = data; break;

        case WAYV_OSC5_TYPE: plugin->osc_type[4] = data; break;
        case WAYV_OSC5_PITCH: plugin->osc_pitch[4] = data; break;
        case WAYV_OSC5_TUNE: plugin->osc_tune[4] = data; break;
        case WAYV_OSC5_VOLUME: plugin->osc_vol[4] = data; break;
        case WAYV_OSC5_UNISON_VOICES: plugin->osc_uni_voice[4] = data; break;
        case WAYV_OSC5_UNISON_SPREAD: plugin->osc_uni_spread[4] = data; break;
        case WAYV_OSC1_FM5: plugin->osc_fm[0][4] = data; break;
        case WAYV_OSC2_FM5: plugin->osc_fm[1][4] = data; break;
        case WAYV_OSC3_FM5: plugin->osc_fm[2][4] = data; break;
        case WAYV_OSC4_FM5: plugin->osc_fm[3][4] = data; break;
        case WAYV_OSC5_FM5: plugin->osc_fm[4][4] = data; break;
        case WAYV_OSC6_FM5: plugin->osc_fm[5][4] = data; break;
        case WAYV_ADSR5_DELAY: plugin->adsr_fm_delay[4] = data; break;
        case WAYV_ATTACK5 : plugin->attack[4] = data; break;
        case WAYV_ADSR5_HOLD: plugin->adsr_fm_hold[4] = data; break;
        case WAYV_DECAY5  : plugin->decay[4] = data; break;
        case WAYV_SUSTAIN5: plugin->sustain[4] = data; break;
        case WAYV_RELEASE5: plugin->release[4] = data; break;
        case WAYV_ADSR5_CHECKBOX: plugin->adsr_checked[4] = data; break;

        case WAYV_OSC6_TYPE: plugin->osc_type[5] = data; break;
        case WAYV_OSC6_PITCH: plugin->osc_pitch[5] = data; break;
        case WAYV_OSC6_TUNE: plugin->osc_tune[5] = data; break;
        case WAYV_OSC6_VOLUME: plugin->osc_vol[5] = data; break;
        case WAYV_OSC6_UNISON_VOICES: plugin->osc_uni_voice[5] = data; break;
        case WAYV_OSC6_UNISON_SPREAD: plugin->osc_uni_spread[5] = data; break;
        case WAYV_OSC1_FM6: plugin->osc_fm[0][5] = data; break;
        case WAYV_OSC2_FM6: plugin->osc_fm[1][5] = data; break;
        case WAYV_OSC3_FM6: plugin->osc_fm[2][5] = data; break;
        case WAYV_OSC4_FM6: plugin->osc_fm[3][5] = data; break;
        case WAYV_OSC5_FM6: plugin->osc_fm[4][5] = data; break;
        case WAYV_OSC6_FM6: plugin->osc_fm[5][5] = data; break;
        case WAYV_ADSR6_DELAY: plugin->adsr_fm_delay[5] = data; break;
        case WAYV_ATTACK6: plugin->attack[5] = data; break;
        case WAYV_ADSR6_HOLD: plugin->adsr_fm_hold[5] = data; break;
        case WAYV_DECAY6  : plugin->decay[5] = data; break;
        case WAYV_SUSTAIN6: plugin->sustain[5] = data; break;
        case WAYV_RELEASE6: plugin->release[5] = data; break;
        case WAYV_ADSR6_CHECKBOX: plugin->adsr_checked[5] = data; break;

        case WAYV_FM_MACRO1_OSC1_FM5: plugin->fm_macro_values[0][0][4] = data; break;
        case WAYV_FM_MACRO1_OSC2_FM5: plugin->fm_macro_values[0][1][4] = data; break;
        case WAYV_FM_MACRO1_OSC3_FM5: plugin->fm_macro_values[0][2][4] = data; break;
        case WAYV_FM_MACRO1_OSC4_FM5: plugin->fm_macro_values[0][3][4] = data; break;
        case WAYV_FM_MACRO1_OSC5_FM5: plugin->fm_macro_values[0][4][4] = data; break;
        case WAYV_FM_MACRO1_OSC6_FM5: plugin->fm_macro_values[0][5][4] = data; break;

        case WAYV_FM_MACRO1_OSC1_FM6: plugin->fm_macro_values[0][0][5] = data; break;
        case WAYV_FM_MACRO1_OSC2_FM6: plugin->fm_macro_values[0][1][5] = data; break;
        case WAYV_FM_MACRO1_OSC3_FM6: plugin->fm_macro_values[0][2][5] = data; break;
        case WAYV_FM_MACRO1_OSC4_FM6: plugin->fm_macro_values[0][3][5] = data; break;
        case WAYV_FM_MACRO1_OSC5_FM6: plugin->fm_macro_values[0][4][5] = data; break;
        case WAYV_FM_MACRO1_OSC6_FM6: plugin->fm_macro_values[0][5][5] = data; break;

        case WAYV_FM_MACRO1_OSC5_FM1: plugin->fm_macro_values[0][4][0] = data; break;
        case WAYV_FM_MACRO1_OSC5_FM2: plugin->fm_macro_values[0][4][1] = data; break;
        case WAYV_FM_MACRO1_OSC5_FM3: plugin->fm_macro_values[0][4][2] = data; break;
        case WAYV_FM_MACRO1_OSC5_FM4: plugin->fm_macro_values[0][4][3] = data; break;

        case WAYV_FM_MACRO1_OSC6_FM1: plugin->fm_macro_values[0][5][0] = data; break;
        case WAYV_FM_MACRO1_OSC6_FM2: plugin->fm_macro_values[0][5][1] = data; break;
        case WAYV_FM_MACRO1_OSC6_FM3: plugin->fm_macro_values[0][5][2] = data; break;
        case WAYV_FM_MACRO1_OSC6_FM4: plugin->fm_macro_values[0][5][3] = data; break;

        case WAYV_FM_MACRO1_OSC5_VOL: plugin->amp_macro_values[0][4] = data; break;
        case WAYV_FM_MACRO1_OSC6_VOL: plugin->amp_macro_values[0][5] = data; break;

        case WAYV_FM_MACRO2_OSC1_FM5: plugin->fm_macro_values[1][0][4] = data; break;
        case WAYV_FM_MACRO2_OSC2_FM5: plugin->fm_macro_values[1][1][4] = data; break;
        case WAYV_FM_MACRO2_OSC3_FM5: plugin->fm_macro_values[1][2][4] = data; break;
        case WAYV_FM_MACRO2_OSC4_FM5: plugin->fm_macro_values[1][3][4] = data; break;
        case WAYV_FM_MACRO2_OSC5_FM5: plugin->fm_macro_values[1][4][4] = data; break;
        case WAYV_FM_MACRO2_OSC6_FM5: plugin->fm_macro_values[1][5][4] = data; break;

        case WAYV_FM_MACRO2_OSC1_FM6: plugin->fm_macro_values[1][0][5] = data; break;
        case WAYV_FM_MACRO2_OSC2_FM6: plugin->fm_macro_values[1][1][5] = data; break;
        case WAYV_FM_MACRO2_OSC3_FM6: plugin->fm_macro_values[1][2][5] = data; break;
        case WAYV_FM_MACRO2_OSC4_FM6: plugin->fm_macro_values[1][3][5] = data; break;
        case WAYV_FM_MACRO2_OSC5_FM6: plugin->fm_macro_values[1][4][5] = data; break;
        case WAYV_FM_MACRO2_OSC6_FM6: plugin->fm_macro_values[1][5][5] = data; break;

        case WAYV_FM_MACRO2_OSC5_FM1: plugin->fm_macro_values[1][4][0] = data; break;
        case WAYV_FM_MACRO2_OSC5_FM2: plugin->fm_macro_values[1][4][1] = data; break;
        case WAYV_FM_MACRO2_OSC5_FM3: plugin->fm_macro_values[1][4][2] = data; break;
        case WAYV_FM_MACRO2_OSC5_FM4: plugin->fm_macro_values[1][4][3] = data; break;

        case WAYV_FM_MACRO2_OSC6_FM1: plugin->fm_macro_values[1][5][0] = data; break;
        case WAYV_FM_MACRO2_OSC6_FM2: plugin->fm_macro_values[1][5][1] = data; break;
        case WAYV_FM_MACRO2_OSC6_FM3: plugin->fm_macro_values[1][5][2] = data; break;
        case WAYV_FM_MACRO2_OSC6_FM4: plugin->fm_macro_values[1][5][3] = data; break;

        case WAYV_FM_MACRO2_OSC5_VOL: plugin->amp_macro_values[1][4] = data; break;
        case WAYV_FM_MACRO2_OSC6_VOL: plugin->amp_macro_values[1][5] = data; break;

        case WAYV_OSC5_FM1: plugin->osc_fm[4][0] = data; break;
        case WAYV_OSC5_FM2: plugin->osc_fm[4][1] = data; break;
        case WAYV_OSC5_FM3: plugin->osc_fm[4][2] = data; break;
        case WAYV_OSC5_FM4: plugin->osc_fm[4][3] = data; break;

        case WAYV_OSC6_FM1: plugin->osc_fm[5][0] = data; break;
        case WAYV_OSC6_FM2: plugin->osc_fm[5][1] = data; break;
        case WAYV_OSC6_FM3: plugin->osc_fm[5][2] = data; break;
        case WAYV_OSC6_FM4: plugin->osc_fm[5][3] = data; break;

        case WAYV_NOISE_PREFX: plugin->noise_prefx = data; break;

        //fm macro 1
        case WAVV_PFXMATRIX_GRP0DST0SRC6CTRL0: plugin->polyfx_mod_matrix[0][0][6][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC6CTRL1: plugin->polyfx_mod_matrix[0][0][6][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC6CTRL2: plugin->polyfx_mod_matrix[0][0][6][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC6CTRL0: plugin->polyfx_mod_matrix[0][1][6][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC6CTRL1: plugin->polyfx_mod_matrix[0][1][6][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC6CTRL2: plugin->polyfx_mod_matrix[0][1][6][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC6CTRL0: plugin->polyfx_mod_matrix[0][2][6][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC6CTRL1: plugin->polyfx_mod_matrix[0][2][6][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC6CTRL2: plugin->polyfx_mod_matrix[0][2][6][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC6CTRL0: plugin->polyfx_mod_matrix[0][3][6][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC6CTRL1: plugin->polyfx_mod_matrix[0][3][6][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC6CTRL2: plugin->polyfx_mod_matrix[0][3][6][2] = data; break;
        //fm macro 2
        case WAVV_PFXMATRIX_GRP0DST0SRC7CTRL0: plugin->polyfx_mod_matrix[0][0][7][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC7CTRL1: plugin->polyfx_mod_matrix[0][0][7][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST0SRC7CTRL2: plugin->polyfx_mod_matrix[0][0][7][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC7CTRL0: plugin->polyfx_mod_matrix[0][1][7][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC7CTRL1: plugin->polyfx_mod_matrix[0][1][7][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST1SRC7CTRL2: plugin->polyfx_mod_matrix[0][1][7][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC7CTRL0: plugin->polyfx_mod_matrix[0][2][7][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC7CTRL1: plugin->polyfx_mod_matrix[0][2][7][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST2SRC7CTRL2: plugin->polyfx_mod_matrix[0][2][7][2] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC7CTRL0: plugin->polyfx_mod_matrix[0][3][7][0] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC7CTRL1: plugin->polyfx_mod_matrix[0][3][7][1] = data; break;
        case WAVV_PFXMATRIX_GRP0DST3SRC7CTRL2: plugin->polyfx_mod_matrix[0][3][7][2] = data; break;

        case WAYV_MIN_NOTE: plugin->min_note = data; break;
        case WAYV_MAX_NOTE: plugin->max_note = data; break;
    }
}

static PYFX_Handle g_wayv_instantiate(PYFX_Descriptor * descriptor,
            int s_rate, fp_get_wavpool_item_from_host a_host_wavpool_func,
            int a_track_num, fp_queue_message a_queue_func)
{
    t_wayv *plugin_data = (t_wayv *) malloc(sizeof(t_wayv));
    plugin_data->fs = s_rate;

    return (PYFX_Handle) plugin_data;
}

static void v_wayv_activate(PYFX_Handle instance, float * a_port_table)
{
    t_wayv *plugin_data = (t_wayv *) instance;

    plugin_data->mono_modules = v_wayv_mono_init(plugin_data->fs);
    plugin_data->port_table = a_port_table;

    int i;

    plugin_data->voices = g_voc_get_voices(WAYV_POLYPHONY,
            WAYV_POLYPHONY_THRESH);

    for (i = 0; i < WAYV_POLYPHONY; i++)
    {
        plugin_data->data[i] = g_wayv_poly_init(
                plugin_data->fs, plugin_data->mono_modules);
        plugin_data->data[i]->note_f = i;
    }
    plugin_data->sampleNo = 0;

    //plugin_data->pitch = 1.0f;
    plugin_data->sv_pitch_bend_value = 0.0f;
    plugin_data->sv_last_note = -1.0f;  //For glide
}

static void v_run_wayv(PYFX_Handle instance, int sample_count,
		  t_pydaw_seq_event *events, int event_count)
{
    t_wayv *plugin_data = (t_wayv *) instance;

    plugin_data->i_run_poly_voice = 0;
    plugin_data->midi_event_count = 0;

    int midi_event_pos = 0;

    int f_poly_mode = (int)(*plugin_data->mono_mode);

    if(f_poly_mode == 2 && plugin_data->voices->poly_mode != 2)
    {
        wayvPanic(instance);  //avoid hung notes
    }

    plugin_data->voices->poly_mode = f_poly_mode;

    int f_min_note = (int)*plugin_data->min_note;
    int f_max_note = (int)*plugin_data->max_note;

    for(plugin_data->event_pos = 0; (plugin_data->event_pos) < event_count;
            plugin_data->event_pos = (plugin_data->event_pos) + 1)
    {
        if (events[(plugin_data->event_pos)].type == PYDAW_EVENT_NOTEON)
        {
            if (events[(plugin_data->event_pos)].velocity > 0)
            {
                if(events[(plugin_data->event_pos)].note > f_max_note ||
                    events[(plugin_data->event_pos)].note < f_min_note)
                {
                    continue;
                }

                int f_voice = i_pick_voice(plugin_data->voices,
                        events[(plugin_data->event_pos)].note,
                        plugin_data->sampleNo,
                        events[(plugin_data->event_pos)].tick);

                plugin_data->data[f_voice]->amp =
                        f_db_to_linear_fast(
                        ((events[(plugin_data->event_pos)].velocity
                        * 0.094488) - 12), //-12db to 0db
                        plugin_data->mono_modules->amp_ptr);

                plugin_data->data[f_voice]->master_vol_lin =
                        f_db_to_linear_fast(
                            (*(plugin_data->master_vol)),
                            plugin_data->mono_modules->amp_ptr);

                plugin_data->data[f_voice]->keyboard_track =
                        ((float)(events[(plugin_data->event_pos)].note))
                        * 0.007874016f;

                plugin_data->data[f_voice]->velocity_track =
                        ((float)(events[(plugin_data->event_pos)].velocity))
                        * 0.007874016f;

                plugin_data->data[f_voice]->note_f =
                        (float)events[(plugin_data->event_pos)].note;
                plugin_data->data[f_voice]->note =
                        events[(plugin_data->event_pos)].note;

                plugin_data->data[f_voice]->target_pitch =
                        (plugin_data->data[f_voice]->note_f);

                if(plugin_data->sv_last_note < 0.0f)
                {
                    plugin_data->data[f_voice]->last_pitch =
                            (plugin_data->data[f_voice]->note_f);
                }
                else
                {
                    plugin_data->data[f_voice]->last_pitch =
                            (plugin_data->sv_last_note);
                }

                v_rmp_retrigger_glide_t(plugin_data->data[f_voice]->glide_env,
                        (*(plugin_data->master_glide) * 0.01f),
                        (plugin_data->data[f_voice]->last_pitch),
                        (plugin_data->data[f_voice]->target_pitch));

                int f_i = 0;

                float f_db;

                while(f_i < WAYV_OSC_COUNT)
                {
                    int f_osc_type = (int)(*plugin_data->osc_type[f_i]) - 1;

                    if(f_osc_type >= 0)
                    {
                        plugin_data->data[f_voice]->osc_on[f_i] = 1;

                        if(f_poly_mode == 0)
                        {
                            v_osc_wav_note_on_sync_phases(
                                plugin_data->data[f_voice]->osc_wavtable[f_i]);
                        }
                        v_osc_wav_set_waveform(
                                plugin_data->data[f_voice]->osc_wavtable[f_i],
                                plugin_data->mono_modules->
                                wavetables->tables[f_osc_type]->wavetable,
                                plugin_data->mono_modules->wavetables->
                                tables[f_osc_type]->length);
                        v_osc_wav_set_uni_voice_count(
                                plugin_data->data[f_voice]->osc_wavtable[f_i],
                                *plugin_data->osc_uni_voice[f_i]);
                    }
                    else
                    {
                        plugin_data->data[f_voice]->osc_on[f_i] = 0;
                        f_i++;
                        continue;
                    }

                    plugin_data->data[f_voice]->osc_uni_spread[f_i] =
                        (*plugin_data->osc_uni_spread[f_i]) * 0.01f;

                    int f_i2 = 0;
                    while(f_i2 < WAYV_OSC_COUNT)
                    {
                        plugin_data->data[f_voice]->osc_fm[f_i][f_i2] =
                                (*plugin_data->osc_fm[f_i][f_i2]) * 0.005f;
                        f_i2++;
                    }

                    f_db = (*plugin_data->osc_vol[f_i]);

                    v_adsr_retrigger(
                        plugin_data->data[f_voice]->adsr_amp_osc[f_i]);

                    plugin_data->data[f_voice]->osc_linamp[f_i] =
                        f_db_to_linear_fast(f_db,
                            plugin_data->mono_modules->amp_ptr);

                    if(f_db > -29.2f)
                    {
                        plugin_data->data[f_voice]->osc_audible[f_i] = 1;
                    }
                    else
                    {
                        plugin_data->data[f_voice]->osc_audible[f_i] = 0;
                    }

                    plugin_data->data[f_voice]->adsr_amp_on[f_i] =
                        (int)(*(plugin_data->adsr_checked[f_i]));

                    if(plugin_data->data[f_voice]->adsr_amp_on[f_i])
                    {
                        float f_attack1 = *(plugin_data->attack[f_i]) * .01f;
                        f_attack1 = (f_attack1) * (f_attack1);
                        float f_decay1 = *(plugin_data->decay[f_i]) * .01f;
                        f_decay1 = (f_decay1) * (f_decay1);
                        float f_release1 = *(plugin_data->release[f_i]) * .01f;
                        f_release1 = (f_release1) * (f_release1);

                        v_adsr_set_adsr_db(plugin_data->data[f_voice]->adsr_amp_osc[f_i],
                                (f_attack1), (f_decay1), *(plugin_data->sustain[f_i]),
                                (f_release1));

                        v_adsr_set_delay_time(
                            plugin_data->data[f_voice]->adsr_amp_osc[f_i],
                            (*plugin_data->adsr_fm_delay[f_i]) * 0.01f);
                        v_adsr_set_hold_time(
                            plugin_data->data[f_voice]->adsr_amp_osc[f_i],
                            (*plugin_data->adsr_fm_hold[f_i]) * 0.01f);
                    }


                    f_i++;
                }

                plugin_data->data[f_voice]->noise_linamp =
                    f_db_to_linear_fast(*(plugin_data->noise_amp),
                        plugin_data->mono_modules->amp_ptr);

                plugin_data->data[f_voice]->adsr_noise_on =
                        (int)*plugin_data->noise_adsr_on;

                plugin_data->data[f_voice]->noise_prefx =
                        (int)*plugin_data->noise_prefx;

                if(plugin_data->data[f_voice]->adsr_noise_on)
                {
                    v_adsr_retrigger(plugin_data->data[f_voice]->adsr_noise);
                    float f_attack = *(plugin_data->noise_attack) * .01f;
                    f_attack = (f_attack) * (f_attack);
                    float f_decay = *(plugin_data->noise_decay) * .01f;
                    f_decay = (f_decay) * (f_decay);
                    float f_sustain = (*plugin_data->noise_sustain);
                    float f_release = *(plugin_data->noise_release) * .01f;
                    f_release = (f_release) * (f_release);
                    v_adsr_set_adsr_db(plugin_data->data[f_voice]->adsr_noise,
                            f_attack, f_decay, f_sustain, f_release);
                    v_adsr_set_delay_time(
                            plugin_data->data[f_voice]->adsr_noise,
                            (*plugin_data->noise_delay) * 0.01f);
                    v_adsr_set_hold_time(
                            plugin_data->data[f_voice]->adsr_noise,
                            (*plugin_data->noise_hold) * 0.01f);
                }

                plugin_data->data[f_voice]->adsr_lfo_on =
                        (int)*plugin_data->lfo_adsr_on;

                if(plugin_data->data[f_voice]->adsr_lfo_on)
                {
                    v_adsr_retrigger(plugin_data->data[f_voice]->adsr_lfo);
                    float f_attack = *(plugin_data->lfo_attack) * .01f;
                    f_attack = (f_attack) * (f_attack);
                    float f_decay = *(plugin_data->lfo_decay) * .01f;
                    f_decay = (f_decay) * (f_decay);
                    float f_sustain = (*plugin_data->lfo_sustain) * 0.01f;
                    float f_release = *(plugin_data->lfo_release) * .01f;
                    f_release = (f_release) * (f_release);
                    v_adsr_set_adsr(plugin_data->data[f_voice]->adsr_lfo,
                            f_attack, f_decay, f_sustain, f_release);
                    v_adsr_set_delay_time(
                            plugin_data->data[f_voice]->adsr_lfo,
                            (*plugin_data->lfo_delay) * 0.01f);
                    v_adsr_set_hold_time(
                            plugin_data->data[f_voice]->adsr_lfo,
                            (*plugin_data->lfo_hold) * 0.01f);
                }

                v_adsr_retrigger(plugin_data->data[f_voice]->adsr_main);

                float f_attack = *(plugin_data->attack_main) * .01f;
                f_attack = (f_attack) * (f_attack);
                float f_decay = *(plugin_data->decay_main) * .01f;
                f_decay = (f_decay) * (f_decay);
                float f_release = *(plugin_data->release_main) * .01f;
                f_release = (f_release) * (f_release);

                v_adsr_set_adsr_db(plugin_data->data[f_voice]->adsr_main,
                    (f_attack), (f_decay), *(plugin_data->sustain_main),
                        (f_release));

                v_adsr_set_hold_time(plugin_data->data[f_voice]->adsr_main,
                        (*plugin_data->hold_main) * 0.01f);

                plugin_data->data[f_voice]->noise_amp =
                    f_db_to_linear(*(plugin_data->noise_amp),
                        plugin_data->mono_modules->amp_ptr);

                /*Set the last_note property, so the next note can glide from
                 * it if glide is turned on*/
                plugin_data->sv_last_note =
                        (plugin_data->data[f_voice]->note_f);


                plugin_data->active_polyfx_count[f_voice] = 0;
                //Determine which PolyFX have been enabled
                for(plugin_data->i_dst = 0;
                    (plugin_data->i_dst) < WAYV_MODULAR_POLYFX_COUNT;
                    plugin_data->i_dst = (plugin_data->i_dst) + 1)
                {
                    int f_pfx_combobox_index =
                        (int)(*(plugin_data->fx_combobox[0][(plugin_data->i_dst)]));
                    plugin_data->data[f_voice]->fx_func_ptr[(plugin_data->i_dst)] =
                        g_mf3_get_function_pointer(f_pfx_combobox_index);

                    if(f_pfx_combobox_index != 0)
                    {
                        plugin_data->active_polyfx[f_voice]
                                [(plugin_data->active_polyfx_count[f_voice])] =
                                (plugin_data->i_dst);
                        plugin_data->active_polyfx_count[f_voice] =
                                (plugin_data->active_polyfx_count[f_voice]) + 1;
                    }
                }

                /*Calculate an index of which mod_matrix controls to process.
                 * This saves expensive iterations and if/then logic in
                 * the main loop*/
                for(plugin_data->i_fx_grps = 0;
                    (plugin_data->i_fx_grps) < WAYV_EFFECTS_GROUPS_COUNT;
                    plugin_data->i_fx_grps = (plugin_data->i_fx_grps) + 1)
                {
                    for(plugin_data->i_dst = 0;
                        (plugin_data->i_dst) < (plugin_data->active_polyfx_count[f_voice]);
                        plugin_data->i_dst = (plugin_data->i_dst) + 1)
                    {
                        plugin_data->polyfx_mod_counts[f_voice]
                                [(plugin_data->active_polyfx[f_voice]
                                [(plugin_data->i_dst)])] = 0;

                        for(plugin_data->i_src = 0;
                                (plugin_data->i_src) < WAYV_MODULATOR_COUNT;
                                plugin_data->i_src = (plugin_data->i_src) + 1)
                        {
                            for(plugin_data->i_ctrl = 0;
                                (plugin_data->i_ctrl) < WAYV_CONTROLS_PER_MOD_EFFECT;
                                plugin_data->i_ctrl = (plugin_data->i_ctrl) + 1)
                            {
                                if((*(plugin_data->polyfx_mod_matrix[(plugin_data->i_fx_grps)][(plugin_data->active_polyfx[f_voice][(plugin_data->i_dst)])][(plugin_data->i_src)][(plugin_data->i_ctrl)])) != 0)
                                {
                                    plugin_data->polyfx_mod_ctrl_indexes[f_voice][(plugin_data->active_polyfx[f_voice][(plugin_data->i_dst)])][(plugin_data->polyfx_mod_counts[f_voice][(plugin_data->active_polyfx[f_voice][(plugin_data->i_dst)])])] = (plugin_data->i_ctrl);
                                    plugin_data->polyfx_mod_src_index[f_voice][(plugin_data->active_polyfx[f_voice][(plugin_data->i_dst)])][(plugin_data->polyfx_mod_counts[f_voice][(plugin_data->active_polyfx[f_voice][(plugin_data->i_dst)])])] = (plugin_data->i_src);
                                    plugin_data->polyfx_mod_matrix_values[f_voice][(plugin_data->active_polyfx[f_voice][(plugin_data->i_dst)])][(plugin_data->polyfx_mod_counts[f_voice][(plugin_data->active_polyfx[f_voice][(plugin_data->i_dst)])])] =
                                            (*(plugin_data->polyfx_mod_matrix[(plugin_data->i_fx_grps)][(plugin_data->active_polyfx[f_voice][(plugin_data->i_dst)])][(plugin_data->i_src)][(plugin_data->i_ctrl)])) * .01;

                                    plugin_data->polyfx_mod_counts[f_voice][(plugin_data->active_polyfx[f_voice][(plugin_data->i_dst)])] = (plugin_data->polyfx_mod_counts[f_voice][(plugin_data->active_polyfx[f_voice][(plugin_data->i_dst)])]) + 1;
                                }
                            }
                        }
                    }
                }
                //Get the noise function pointer
                plugin_data->data[f_voice]->noise_func_ptr =
                        fp_get_noise_func_ptr((int)(*(plugin_data->noise_type)));

                f_i = 0;

                while(f_i < 2)
                {
                    int f_i2 = 0;
                    while(f_i2 < WAYV_OSC_COUNT)
                    {
                        plugin_data->data[f_voice]->osc_macro_amp[f_i][f_i2] =
                                (*plugin_data->amp_macro_values[f_i][f_i2]);
                        f_i2++;
                    }
                    f_i++;
                }

                v_adsr_retrigger(plugin_data->data[f_voice]->adsr_amp);
                v_adsr_retrigger(plugin_data->data[f_voice]->adsr_filter);
                v_lfs_sync(plugin_data->data[f_voice]->lfo1,
                           *plugin_data->lfo_phase * 0.01f,
                           *plugin_data->lfo_type);

                float f_attack_a = (*(plugin_data->pfx_attack) * .01);
                f_attack_a *= f_attack_a;
                float f_decay_a = (*(plugin_data->pfx_decay) * .01);
                f_decay_a *= f_decay_a;
                float f_release_a = (*(plugin_data->pfx_release) * .01);
                f_release_a *= f_release_a;

                v_adsr_set_adsr_db(plugin_data->data[f_voice]->adsr_amp,
                        f_attack_a, f_decay_a, (*(plugin_data->pfx_sustain)),
                        f_release_a);

                v_adsr_set_delay_time(
                    plugin_data->data[f_voice]->adsr_amp,
                    (*(plugin_data->pfx_delay) * .01));
                v_adsr_set_hold_time(
                    plugin_data->data[f_voice]->adsr_amp,
                    (*(plugin_data->pfx_hold) * .01));

                float f_attack_f = (*(plugin_data->pfx_attack_f) * .01);
                f_attack_f *= f_attack_f;
                float f_decay_f = (*(plugin_data->pfx_decay_f) * .01);
                f_decay_f *= f_decay_f;
                float f_release_f = (*(plugin_data->pfx_release_f) * .01);
                f_release_f *= f_release_f;

                v_adsr_set_adsr(plugin_data->data[f_voice]->adsr_filter,
                        f_attack_f, f_decay_f,
                        (*(plugin_data->pfx_sustain_f) * .01), f_release_f);

                v_adsr_set_delay_time(
                    plugin_data->data[f_voice]->adsr_filter,
                    (*(plugin_data->pfx_delay_f) * .01));
                v_adsr_set_hold_time(
                    plugin_data->data[f_voice]->adsr_filter,
                    (*(plugin_data->pfx_hold_f) * .01));

                /*Retrigger the pitch envelope*/
                v_rmp_retrigger_curve((plugin_data->data[f_voice]->ramp_env),
                        (*(plugin_data->pitch_env_time) * .01), 1.0f,
                        (*plugin_data->ramp_curve) * 0.01f);

                plugin_data->data[f_voice]->noise_amp =
                        f_db_to_linear(*(plugin_data->noise_amp),
                        plugin_data->mono_modules->amp_ptr);

                plugin_data->data[f_voice]->adsr_prefx =
                        (int)*plugin_data->adsr_prefx;

                plugin_data->data[f_voice]->perc_env_on =
                        (int)(*plugin_data->perc_env_on);

                if(plugin_data->data[f_voice]->perc_env_on)
                {
                    v_pnv_set(plugin_data->data[f_voice]->perc_env,
                            (*plugin_data->perc_env_time1) * 0.001f,
                            (*plugin_data->perc_env_pitch1),
                            (*plugin_data->perc_env_time2) * 0.001f,
                            (*plugin_data->perc_env_pitch2),
                            plugin_data->data[f_voice]->note_f);
                }
            }
            /*0 velocity, the same as note-off*/
            else
            {
                v_voc_note_off(plugin_data->voices,
                    events[(plugin_data->event_pos)].note,
                    (plugin_data->sampleNo),
                    (events[(plugin_data->event_pos)].tick));
            }
        }
        else if (events[(plugin_data->event_pos)].type == PYDAW_EVENT_NOTEOFF)
        {
            v_voc_note_off(plugin_data->voices,
                events[(plugin_data->event_pos)].note, (plugin_data->sampleNo),
                (events[(plugin_data->event_pos)].tick));
        }
        else if (events[plugin_data->event_pos].type == PYDAW_EVENT_CONTROLLER)
        {
            //The host already filters out messages for other instruments
            if (events[plugin_data->event_pos].plugin_index != -1)
            {
                assert(events[plugin_data->event_pos].port >=
                        WAYV_FIRST_CONTROL_PORT &&
                        events[plugin_data->event_pos].port < WAYV_COUNT);

                plugin_data->midi_event_types[plugin_data->midi_event_count] =
                        PYDAW_EVENT_CONTROLLER;
                plugin_data->midi_event_ticks[plugin_data->midi_event_count] =
                        events[plugin_data->event_pos].tick;
                plugin_data->midi_event_ports[plugin_data->midi_event_count] =
                        events[plugin_data->event_pos].port;
                plugin_data->midi_event_values[plugin_data->midi_event_count] =
                        events[plugin_data->event_pos].value;
                plugin_data->midi_event_count++;
            }
        }
        else if (events[(plugin_data->event_pos)].type == PYDAW_EVENT_PITCHBEND)
        {
            plugin_data->midi_event_types[plugin_data->midi_event_count] =
                    PYDAW_EVENT_PITCHBEND;
            plugin_data->midi_event_ticks[plugin_data->midi_event_count] =
                    events[plugin_data->event_pos].tick;
            plugin_data->midi_event_values[plugin_data->midi_event_count] =
                    0.00012207 * events[plugin_data->event_pos].value;
            plugin_data->midi_event_count++;
        }
    }

    /*Clear the output buffer*/
    plugin_data->i_iterator = 0;

    while((plugin_data->i_iterator) < sample_count)
    {
        plugin_data->output0[(plugin_data->i_iterator)] = 0.0f;
        plugin_data->output1[(plugin_data->i_iterator)] = 0.0f;

        while(midi_event_pos < plugin_data->midi_event_count &&
                plugin_data->midi_event_ticks[midi_event_pos] == plugin_data->i_iterator)
        {
            if(plugin_data->midi_event_types[midi_event_pos] == PYDAW_EVENT_PITCHBEND)
            {
                plugin_data->sv_pitch_bend_value =
                        plugin_data->midi_event_values[midi_event_pos];
            }
            else if(plugin_data->midi_event_types[midi_event_pos] == PYDAW_EVENT_CONTROLLER)
            {
                plugin_data->port_table[plugin_data->midi_event_ports[midi_event_pos]] =
                        plugin_data->midi_event_values[midi_event_pos];
            }

            midi_event_pos++;
        }

        if(plugin_data->mono_modules->reset_wavetables)
        {
            int f_voice = 0;
            int f_osc_type[WAYV_OSC_COUNT];
            int f_i = 0;

            while(f_i < WAYV_OSC_COUNT)
            {
                f_osc_type[f_i] = (int)(*plugin_data->osc_type[f_i]) - 1;
                f_i++;
            }

            while(f_voice < WAYV_POLYPHONY)
            {
                f_i = 0;

                while(f_i < WAYV_OSC_COUNT)
                {
                    if(f_osc_type[f_i] >= 0)
                    {
                        v_osc_wav_set_waveform(
                            plugin_data->data[f_voice]->osc_wavtable[f_i],
                            plugin_data->mono_modules->wavetables->
                                tables[f_osc_type[f_i]]->wavetable,
                            plugin_data->mono_modules->wavetables->
                                tables[f_osc_type[f_i]]->length);
                    }
                    f_i++;
                }

                f_voice++;
            }

            plugin_data->mono_modules->reset_wavetables = 0;
        }

        v_sml_run(plugin_data->mono_modules->pitchbend_smoother,
                (plugin_data->sv_pitch_bend_value));

        v_sml_run(plugin_data->mono_modules->fm_macro_smoother[0],
                (*plugin_data->fm_macro[0] * 0.01f));

        v_sml_run(plugin_data->mono_modules->fm_macro_smoother[1],
                (*plugin_data->fm_macro[1] * 0.01f));

        plugin_data->i_run_poly_voice = 0;
        while ((plugin_data->i_run_poly_voice) < WAYV_POLYPHONY)
        {
            //if (data[voice].state != inactive)
            if((plugin_data->data[(plugin_data->i_run_poly_voice)]->
                    adsr_main->stage) != ADSR_STAGE_OFF)
            {
                v_run_wayv_voice(plugin_data,
                        plugin_data->voices->voices[(plugin_data->i_run_poly_voice)],
                        plugin_data->data[(plugin_data->i_run_poly_voice)],
                        plugin_data->output0,
                        plugin_data->output1,
                        plugin_data->i_iterator,
                        plugin_data->i_run_poly_voice
                        );
            }
            else
            {
                plugin_data->voices->voices[(plugin_data->i_run_poly_voice)].
                        n_state = note_state_off;
            }

            plugin_data->i_run_poly_voice = (plugin_data->i_run_poly_voice) + 1;
        }

        plugin_data->sampleNo++;
        plugin_data->i_iterator = (plugin_data->i_iterator) + 1;
    }

    //plugin_data->sampleNo += sample_count;
}

static void v_run_wayv_voice(t_wayv *plugin_data,
        t_voc_single_voice a_poly_voice, t_wayv_poly_voice *a_voice,
        PYFX_Data *out0, PYFX_Data *out1, int a_i, int a_voice_num)
{
    a_voice->i_voice = a_i;

    if((plugin_data->sampleNo) < (a_poly_voice.on))
    {
        return;
    }

    if (((plugin_data->sampleNo) == a_poly_voice.off) &&
            ((a_voice->adsr_main->stage) < ADSR_STAGE_RELEASE))
    {
        if(a_poly_voice.n_state == note_state_killed)
        {
            v_wayv_poly_note_off(a_voice, 1);
        }
        else
        {
            v_wayv_poly_note_off(a_voice, 0);
        }
    }

    a_voice->current_sample = 0.0f;

    f_rmp_run_ramp(a_voice->glide_env);

    v_adsr_run_db(plugin_data->data[a_voice_num]->adsr_amp);

    v_adsr_run(plugin_data->data[a_voice_num]->adsr_filter);

    f_rmp_run_ramp_curve(plugin_data->data[a_voice_num]->ramp_env);

    //Set and run the LFO
    v_lfs_set(plugin_data->data[a_voice_num]->lfo1,
            (*(plugin_data->lfo_freq)) * .01);
    v_lfs_run(plugin_data->data[a_voice_num]->lfo1);

    a_voice->lfo_amount_output =
            (a_voice->lfo1->output) * ((*plugin_data->lfo_amount) * 0.01f);

    if(a_voice->adsr_lfo_on)
    {
        v_adsr_run(a_voice->adsr_lfo);
        a_voice->lfo_amount_output *= a_voice->adsr_lfo->output;
    }

    a_voice->lfo_amp_output =
            f_db_to_linear_fast((((*plugin_data->lfo_amp) *
            (a_voice->lfo_amount_output)) -
            (f_lms_abs((*plugin_data->lfo_amp)) * 0.5)),
            a_voice->amp_ptr);

    a_voice->lfo_pitch_output =
            (*plugin_data->lfo_pitch + (*plugin_data->lfo_pitch_fine * 0.01f))
            * (a_voice->lfo_amount_output);

    if(a_voice->perc_env_on)
    {
        a_voice->base_pitch = f_pnv_run(a_voice->perc_env);
    }
    else
    {
        a_voice->base_pitch =
            (a_voice->glide_env->output_multiplied) +
            ((a_voice->ramp_env->output_multiplied) *
            (*plugin_data->pitch_env_amt))
            + (plugin_data->mono_modules->pitchbend_smoother->last_value  *
            (*(plugin_data->master_pb_amt))) + (a_voice->last_pitch) +
            (a_voice->lfo_pitch_output);
    }

    int f_osc_num = 0;
    float f_macro_amp;
    float f_osc_amp;

    while(f_osc_num < WAYV_OSC_COUNT)
    {
        f_macro_amp = 0.0f;

        if(a_voice->osc_on[f_osc_num])
        {
            v_osc_wav_set_unison_pitch(
                    a_voice->osc_wavtable[f_osc_num],
                    (a_voice->osc_uni_spread[f_osc_num]),
                    ((a_voice->base_pitch) +
                    (*plugin_data->osc_pitch[f_osc_num]) +
                    ((*plugin_data->osc_tune[f_osc_num]) * 0.01f) ));

            int f_i = 0;
            while(f_i < WAYV_OSC_COUNT)
            {
                a_voice->fm_osc_values[f_osc_num][f_i] =
                    a_voice->osc_fm[f_osc_num][f_i];
                f_i++;
            }

            f_i = 0;

            while(f_i < 2)
            {
                if(plugin_data->mono_modules->fm_macro_smoother[f_i]->last_value
                        > 0.0f)
                {
                    int f_i2 = 0;
                    while(f_i2 < WAYV_OSC_COUNT)
                    {
                        a_voice->fm_osc_values[f_osc_num][f_i2] +=
                          ((*plugin_data->fm_macro_values[f_i][f_osc_num][f_i2]
                                * 0.005f) *
                            plugin_data->mono_modules->
                                fm_macro_smoother[f_i]->last_value);
                        f_i2++;
                    }

                    if(a_voice->osc_macro_amp[f_i][f_osc_num] != 0.0f)
                    {
                        f_macro_amp +=
                            plugin_data->mono_modules->fm_macro_smoother[f_i]->last_value *
                            a_voice->osc_macro_amp[f_i][f_osc_num];
                    }
                }

                f_i++;
            }

            f_i = 0;

            while(f_i < WAYV_OSC_COUNT)
            {
                if(a_voice->fm_osc_values[f_osc_num][f_i] < 0.0f)
                {
                    a_voice->fm_osc_values[f_osc_num][f_i] = 0.0f;
                }
                else if(a_voice->fm_osc_values[f_osc_num][f_i] > 0.5f)
                {
                    a_voice->fm_osc_values[f_osc_num][f_i] = 0.5f;
                }

                if(f_i <= f_osc_num)
                {
                    v_osc_wav_apply_fm(
                        a_voice->osc_wavtable[f_osc_num],
                        a_voice->fm_last[f_i],
                        a_voice->fm_osc_values[f_osc_num][f_i]);
                }
                else
                {
                    v_osc_wav_apply_fm_direct(
                        a_voice->osc_wavtable[f_osc_num],
                        a_voice->fm_last[f_i],
                        a_voice->fm_osc_values[f_osc_num][f_i]);
                }

                f_i++;
            }

            if(a_voice->adsr_amp_on[f_osc_num])
            {
                v_adsr_run_db(a_voice->adsr_amp_osc[f_osc_num]);
                a_voice->fm_last[f_osc_num] =
                    f_osc_wav_run_unison(a_voice->osc_wavtable[f_osc_num])
                        * (a_voice->adsr_amp_osc[f_osc_num]->output);
            }
            else
            {
                a_voice->fm_last[f_osc_num] =
                        f_osc_wav_run_unison(a_voice->osc_wavtable[f_osc_num]);
            }

            if(a_voice->osc_audible[f_osc_num] || f_macro_amp >= 1.0f)
            {
                f_osc_amp = (a_voice->osc_linamp[f_osc_num]) *
                        f_db_to_linear(f_macro_amp,
                            plugin_data->mono_modules->amp_ptr);

                if(f_osc_amp > 1.0f)  //clip at 0dB
                {
                    a_voice->current_sample += (a_voice->fm_last[f_osc_num]);
                }
                else
                {
                    a_voice->current_sample +=
                        (a_voice->fm_last[f_osc_num]) * f_osc_amp;
                }
            }
        }

        f_osc_num++;
    }

    if(a_voice->noise_prefx)
    {
        if(a_voice->adsr_noise_on)
        {
            v_adsr_run(a_voice->adsr_noise);
            a_voice->current_sample +=
                a_voice->noise_func_ptr(a_voice->white_noise1) *
                (a_voice->noise_linamp) * a_voice->adsr_noise->output;
        }
        else
        {
            a_voice->current_sample +=
                (a_voice->noise_func_ptr(a_voice->white_noise1) *
                (a_voice->noise_linamp));
        }
    }

    v_adsr_run_db(a_voice->adsr_main);

    if(a_voice->adsr_prefx)
    {
        a_voice->current_sample *= (a_voice->adsr_main->output);
    }

    a_voice->current_sample = (a_voice->current_sample) * (a_voice->amp) *
            (a_voice->lfo_amp_output);

    a_voice->modulex_current_sample[0] = (a_voice->current_sample);
    a_voice->modulex_current_sample[1] = (a_voice->current_sample);

    //Modular PolyFX, processed from the index created during note_on
    for(plugin_data->i_dst = 0;
        (plugin_data->i_dst) < (plugin_data->active_polyfx_count[a_voice_num]);
        plugin_data->i_dst = (plugin_data->i_dst) + 1)
    {
        v_mf3_set(a_voice->multieffect[(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])],
            *(plugin_data->pfx_mod_knob[0][(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])][0]),
                *(plugin_data->pfx_mod_knob[0][(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])][1]),
                *(plugin_data->pfx_mod_knob[0][(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])][2]));

        int f_mod_test;

        for(f_mod_test = 0;
            f_mod_test < (plugin_data->polyfx_mod_counts[a_voice_num][(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])]);
            f_mod_test++)
        {
            v_mf3_mod_single(
                    a_voice->multieffect[(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])],
                    *(a_voice->modulator_outputs[(plugin_data->polyfx_mod_src_index[a_voice_num][(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])][f_mod_test])]),
                    (plugin_data->polyfx_mod_matrix_values[a_voice_num][(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])][f_mod_test]),
                    (plugin_data->polyfx_mod_ctrl_indexes[a_voice_num][(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])][f_mod_test])
                    );
        }

        a_voice->fx_func_ptr[(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])](
                a_voice->multieffect[(plugin_data->active_polyfx[a_voice_num][(plugin_data->i_dst)])],
                (a_voice->modulex_current_sample[0]), (a_voice->modulex_current_sample[1]));

        a_voice->modulex_current_sample[0] =
            a_voice->multieffect[(plugin_data->active_polyfx[a_voice_num]
                [(plugin_data->i_dst)])]->output0;
        a_voice->modulex_current_sample[1] =
            a_voice->multieffect[(plugin_data->active_polyfx[a_voice_num]
                [(plugin_data->i_dst)])]->output1;

    }

    if(!a_voice->noise_prefx)
    {
        if(a_voice->adsr_noise_on)
        {
            v_adsr_run(a_voice->adsr_noise);
            float f_noise =
                a_voice->noise_func_ptr(a_voice->white_noise1) *
                (a_voice->noise_linamp) * a_voice->adsr_noise->output *
                a_voice->adsr_main->output;
            out0[(a_voice->i_voice)] += f_noise;
            out1[(a_voice->i_voice)] += f_noise;
        }
        else
        {
            float f_noise =
                (a_voice->noise_func_ptr(a_voice->white_noise1) *
                (a_voice->noise_linamp)) *
                a_voice->adsr_main->output;
            out0[(a_voice->i_voice)] += f_noise;
            out1[(a_voice->i_voice)] += f_noise;
        }
    }

    if(a_voice->adsr_prefx)
    {
        out0[(a_voice->i_voice)] += (a_voice->modulex_current_sample[0]) *
            (a_voice->master_vol_lin);
        out1[(a_voice->i_voice)] += (a_voice->modulex_current_sample[1]) *
            (a_voice->master_vol_lin);
    }
    else
    {
        out0[(a_voice->i_voice)] += (a_voice->modulex_current_sample[0]) *
            (a_voice->adsr_main->output) * (a_voice->master_vol_lin);
        out1[(a_voice->i_voice)] += (a_voice->modulex_current_sample[1]) *
            (a_voice->adsr_main->output) * (a_voice->master_vol_lin);
    }
}


float * f_char_to_wavetable(char * a_char)
{
    float * f_result;

    if(posix_memalign((void**)&f_result, 16, (sizeof(float) * 1024)) != 0)
    {
        return 0;
    }

    t_1d_char_array * f_arr = c_split_str(a_char, '|', 1025, 32);

    int f_i = 1;

    int f_count = atoi(f_arr->array[0]);
    printf("%i\n", f_count);

    while(f_i < 1025)
    {
        f_result[f_i - 1] = atof(f_arr->array[f_i]);
        f_i++;
    }

    g_free_1d_char_array(f_arr);

    return f_result;
}

char *c_wayv_configure(PYFX_Handle instance, char *key,
        char *value, pthread_mutex_t * a_mutex)
{
    t_wayv *plugin_data = (t_wayv*)instance;

    if (!strcmp(key, "wayv_add_eng0"))
    {
        float * f_table = f_char_to_wavetable(value);
        v_wt_set_wavetable(plugin_data->mono_modules->wavetables, 17, f_table,
                1024, a_mutex, &plugin_data->mono_modules->reset_wavetables);
    }
    else if (!strcmp(key, "wayv_add_eng1"))
    {
        float * f_table = f_char_to_wavetable(value);
        v_wt_set_wavetable(plugin_data->mono_modules->wavetables, 18, f_table,
                1024, a_mutex, &plugin_data->mono_modules->reset_wavetables);
    }
    else if (!strcmp(key, "wayv_add_eng2"))
    {
        float * f_table = f_char_to_wavetable(value);
        v_wt_set_wavetable(plugin_data->mono_modules->wavetables, 19, f_table,
                1024, a_mutex, &plugin_data->mono_modules->reset_wavetables);
    }
    else
    {
        printf("Way-V unhandled configure key %s\n", key);
    }

    return NULL;
}


PYFX_Descriptor *wayv_PYFX_descriptor(int index)
{
    PYFX_Descriptor *f_result =
            pydaw_get_pyfx_descriptor(987564, "Way-V", WAYV_COUNT);

    pydaw_set_pyfx_port(f_result, WAYV_ATTACK_MAIN, 10.0f, 0.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, WAYV_DECAY_MAIN, 50.0f, 10.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, WAYV_SUSTAIN_MAIN, 0.0f, -30.0f, 0.0f);
    pydaw_set_pyfx_port(f_result, WAYV_RELEASE_MAIN, 50.0f, 10.0f, 400.0f);
    pydaw_set_pyfx_port(f_result, WAYV_ATTACK1, 10.0f, 0.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, WAYV_DECAY1, 50.0f, 10.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, WAYV_SUSTAIN1, 0.0f, -30.0f, 0.0f);
    pydaw_set_pyfx_port(f_result, WAYV_RELEASE1, 50.0f, 10.0f, 400.0f);
    pydaw_set_pyfx_port(f_result, WAYV_ATTACK2, 10.0f, 0.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, WAYV_DECAY2, 50.0f, 10.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, WAYV_SUSTAIN2, 0.0f, -30.0f, 0.0f);
    pydaw_set_pyfx_port(f_result, WAYV_RELEASE2, 50.0f, 10.0f, 400.0f);
    pydaw_set_pyfx_port(f_result, WAYV_NOISE_AMP, -30.0f, -60.0f, 0.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC1_TYPE, 1.0f, 0.0f, (float)WT_TOTAL_WAVETABLE_COUNT);
    pydaw_set_pyfx_port(f_result, WAYV_OSC1_PITCH, 0.0f, -72.0f, 72.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC1_TUNE, 0.0f, -100.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC1_VOLUME, -6.0f, -30.0f, 0.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC2_TYPE, 0.0f, 0.0f, (float)WT_TOTAL_WAVETABLE_COUNT);
    pydaw_set_pyfx_port(f_result, WAYV_OSC2_PITCH, 0.0f, -72.0f, 72.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC2_TUNE, 0.0f, -100.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC2_VOLUME, -6.0f, -30.0f, 0.0f);
    pydaw_set_pyfx_port(f_result, WAYV_MASTER_VOLUME, -6.0f, -30.0f, 12.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC1_UNISON_VOICES, 1.0f, 1.0f, 7.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC1_UNISON_SPREAD, 50.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_MASTER_GLIDE, 0.0f, 0.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, WAYV_MASTER_PITCHBEND_AMT, 18.0f, 1.0f, 36.0f);
    pydaw_set_pyfx_port(f_result, WAYV_ATTACK_PFX1, 10.0f, 0.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, WAYV_DECAY_PFX1, 50.0f, 10.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, WAYV_SUSTAIN_PFX1, 0.0f, -30.0f, 0.0f);
    pydaw_set_pyfx_port(f_result, WAYV_RELEASE_PFX1, 50.0f, 10.0f, 400.0f);
    pydaw_set_pyfx_port(f_result, WAYV_ATTACK_PFX2, 10.0f, 0.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, WAYV_DECAY_PFX2, 50.0f, 10.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, WAYV_SUSTAIN_PFX2, 100.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_RELEASE_PFX2, 50.0f, 10.0f, 400.0f);
    pydaw_set_pyfx_port(f_result, WAYV_RAMP_ENV_TIME, 100.0f, 0.0f, 600.0f);
    pydaw_set_pyfx_port(f_result, WAYV_LFO_FREQ, 200.0f, 10, 1600);
    pydaw_set_pyfx_port(f_result, WAYV_LFO_TYPE, 0.0f, 0.0f, 2.0f);
    pydaw_set_pyfx_port(f_result, WAYV_FX0_KNOB0, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(f_result, WAYV_FX0_KNOB1, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(f_result, WAYV_FX0_KNOB2, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(f_result, WAYV_FX0_COMBOBOX, 0.0f, 0.0f, MULTIFX3KNOB_MAX_INDEX);
    pydaw_set_pyfx_port(f_result, WAYV_FX1_KNOB0, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(f_result, WAYV_FX1_KNOB1, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(f_result, WAYV_FX1_KNOB2, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(f_result, WAYV_FX1_COMBOBOX, 0.0f, 0.0f, MULTIFX3KNOB_MAX_INDEX);
    pydaw_set_pyfx_port(f_result, WAYV_FX2_KNOB0, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(f_result, WAYV_FX2_KNOB1, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(f_result, WAYV_FX2_KNOB2, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(f_result, WAYV_FX2_COMBOBOX, 0.0f, 0.0f, MULTIFX3KNOB_MAX_INDEX);
    pydaw_set_pyfx_port(f_result, WAYV_FX3_KNOB0, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(f_result, WAYV_FX3_KNOB1, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(f_result, WAYV_FX3_KNOB2, 64.0f, 0.0f, 127.0f);
    pydaw_set_pyfx_port(f_result, WAYV_FX3_COMBOBOX, 0.0f, 0.0f, MULTIFX3KNOB_MAX_INDEX);

    int f_i = WAVV_PFXMATRIX_GRP0DST0SRC0CTRL0;

    while(f_i <= WAVV_PFXMATRIX_GRP0DST3SRC3CTRL2)
    {
        pydaw_set_pyfx_port(f_result, f_i,  0.0f, -100.0f, 100.0f);
        f_i++;
    }

    pydaw_set_pyfx_port(f_result, WAYV_NOISE_TYPE, 0.0f, 0, 2);
    pydaw_set_pyfx_port(f_result, WAYV_ADSR1_CHECKBOX, 0.0f, 0, 1);
    pydaw_set_pyfx_port(f_result, WAYV_ADSR2_CHECKBOX, 0.0f, 0, 1);
    pydaw_set_pyfx_port(f_result, WAYV_LFO_AMP, 0.0f, -24.0f, 24.0f);
    pydaw_set_pyfx_port(f_result, WAYV_LFO_PITCH, 0.0f, -36.0f, 36.0f);
    pydaw_set_pyfx_port(f_result, WAYV_PITCH_ENV_AMT, 0.0f, -60.0f, 60.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC2_UNISON_VOICES, 1.0f, 1.0f, 7.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC2_UNISON_SPREAD, 50.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_LFO_AMOUNT, 100.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC3_TYPE, 0.0f, 0.0f, (float)WT_TOTAL_WAVETABLE_COUNT);
    pydaw_set_pyfx_port(f_result, WAYV_OSC3_PITCH, 0.0f, -72.0f, 72.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC3_TUNE, 0.0f, -100.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC3_VOLUME, -6.0f, -30.0f, 0.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC3_UNISON_VOICES, 1.0f, 1.0f, 7.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC3_UNISON_SPREAD, 50.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC1_FM1, 0.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC1_FM2, 0.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC1_FM3, 0.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC2_FM1, 0.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC2_FM2, 0.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC2_FM3, 0.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC3_FM1, 0.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC3_FM2, 0.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC3_FM3, 0.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_ATTACK3, 10.0f, 0.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, WAYV_DECAY3, 50.0f, 10.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, WAYV_SUSTAIN3, 0.0f, -30.0f, 0.0f);
    pydaw_set_pyfx_port(f_result, WAYV_RELEASE3, 50.0f, 10.0f, 400.0f);
    pydaw_set_pyfx_port(f_result, WAYV_ADSR3_CHECKBOX, 0.0f, 0, 1);

    f_i = WAVV_PFXMATRIX_GRP0DST0SRC4CTRL0;

    while(f_i <= WAVV_PFXMATRIX_GRP0DST3SRC5CTRL2)
    {
        pydaw_set_pyfx_port(f_result, f_i, 0.0f, -100.0f, 100.0f);
        f_i++;
    }

    pydaw_set_pyfx_port(f_result, WAYV_PERC_ENV_TIME1, 10.0f, 2.0f, 40.0f);
    pydaw_set_pyfx_port(f_result, WAYV_PERC_ENV_PITCH1, 66.0f, 42.0f, 120.0f);
    pydaw_set_pyfx_port(f_result, WAYV_PERC_ENV_TIME2, 100.0f, 20.0f, 400.0f);
    pydaw_set_pyfx_port(f_result, WAYV_PERC_ENV_PITCH2, 48.0f, 33.0f, 63.0f);
    pydaw_set_pyfx_port(f_result, WAYV_PERC_ENV_ON, 0.0f, 0.0f, 1.0f);
    pydaw_set_pyfx_port(f_result, WAYV_RAMP_CURVE, 50.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_MONO_MODE, 0.0f, 0.0f, 3.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC1_FM4, 0.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC2_FM4, 0.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC3_FM4, 0.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC4_TYPE, 0.0f, 0.0f, (float)WT_TOTAL_WAVETABLE_COUNT);
    pydaw_set_pyfx_port(f_result, WAYV_OSC4_PITCH, 0.0f, -72.0f, 72.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC4_TUNE, 0.0f, -100.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC4_VOLUME, -6.0f, -30.0f, 0.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC4_UNISON_VOICES, 1.0f, 1.0f, 7.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC4_UNISON_SPREAD, 50.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC4_FM1, 0.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC4_FM2, 0.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC4_FM3, 0.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC4_FM4, 0.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_ATTACK4, 10.0f, 0.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, WAYV_DECAY4, 50.0f, 10.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, WAYV_SUSTAIN4, 0.0f, -30.0f, 0.0f);
    pydaw_set_pyfx_port(f_result, WAYV_RELEASE4, 50.0f, 10.0f, 400.0f);
    pydaw_set_pyfx_port(f_result, WAYV_ADSR4_CHECKBOX, 0.0f, 0, 1);
    pydaw_set_pyfx_port(f_result, WAYV_LFO_PHASE, 0.0f, 0.0f, 100.0);

    f_i = 0;
    int f_port = WAYV_FM_MACRO1;

    while(f_i < 2)
    {
        pydaw_set_pyfx_port(f_result, f_port, 0.0f, 0.0f, 100.0f);
        f_port++;

        int f_i2 = 0;

        while(f_i2 < 4)
        {
            int f_i3 = 0;

            while(f_i3 < 4)
            {
                pydaw_set_pyfx_port(f_result, f_port, 0.0f, -100.0f, 100.0f);
                f_port++;
                f_i3++;
            }

            f_i2++;
        }

        f_i++;
    }


    f_i = 0;
    f_port = WAYV_FM_MACRO1_OSC1_VOL;

    while(f_i < 2)
    {
        int f_i2 = 0;

        while(f_i2 < 4)
        {
            pydaw_set_pyfx_port(f_result, f_port, 0.0f, -30.0f, 30.0f);
            f_port++;
            f_i2++;
        }

        f_i++;
    }

    pydaw_set_pyfx_port(f_result, WAYV_LFO_PITCH_FINE, 0.0f, -100.0f, 100.0);
    pydaw_set_pyfx_port(f_result, WAYV_ADSR_PREFX, 0.0f, 0.0f, 1.0);

    f_port = WAYV_ADSR1_DELAY;

    // The loop covers the hold and delay ports
    while(f_port <= WAYV_HOLD_MAIN)
    {
        pydaw_set_pyfx_port(f_result, f_port, 0.0f, 0.0f, 200.0f);
        f_port++;
    }

    pydaw_set_pyfx_port(f_result, WAYV_DELAY_NOISE, 0.0f, 0.0f, 200.0);
    pydaw_set_pyfx_port(f_result, WAYV_ATTACK_NOISE, 10.0f, 0.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, WAYV_HOLD_NOISE, 0.0f, 0.0f, 200.0);
    pydaw_set_pyfx_port(f_result, WAYV_DECAY_NOISE, 50.0f, 10.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, WAYV_SUSTAIN_NOISE, 0.0f, -30.0f, 0.0f);
    pydaw_set_pyfx_port(f_result, WAYV_RELEASE_NOISE, 50.0f, 10.0f, 400.0f);
    pydaw_set_pyfx_port(f_result, WAYV_ADSR_NOISE_ON, 0.0f, 0.0f, 1.0f);
    pydaw_set_pyfx_port(f_result, WAYV_DELAY_LFO, 0.0f, 0.0f, 200.0);
    pydaw_set_pyfx_port(f_result, WAYV_ATTACK_LFO, 10.0f, 0.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, WAYV_HOLD_LFO, 0.0f, 0.0f, 200.0);
    pydaw_set_pyfx_port(f_result, WAYV_DECAY_LFO, 50.0f, 10.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, WAYV_SUSTAIN_LFO, 100.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_RELEASE_LFO, 50.0f, 10.0f, 400.0f);
    pydaw_set_pyfx_port(f_result, WAYV_ADSR_LFO_ON, 0.0f, 0.0f, 1.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC5_TYPE, 0.0f, 0.0f, (float)WT_TOTAL_WAVETABLE_COUNT);
    pydaw_set_pyfx_port(f_result, WAYV_OSC5_PITCH, 0.0f, -72.0f, 72.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC5_TUNE, 0.0f, -100.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC5_VOLUME, -6.0f, -30.0f, 0.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC5_UNISON_VOICES, 1.0f, 1.0f, 7.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC5_UNISON_SPREAD, 50.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_ADSR5_DELAY, 0.0f, 0.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, WAYV_ATTACK5, 10.0f, 0.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, WAYV_ADSR5_HOLD, 0.0f, 0.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, WAYV_DECAY5, 50.0f, 10.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, WAYV_SUSTAIN5, 0.0f, -30.0f, 0.0f);
    pydaw_set_pyfx_port(f_result, WAYV_RELEASE5, 50.0f, 10.0f, 400.0f);
    pydaw_set_pyfx_port(f_result, WAYV_ADSR5_CHECKBOX, 0.0f, 0, 1);
    pydaw_set_pyfx_port(f_result, WAYV_OSC6_TYPE, 0.0f, 0.0f, (float)WT_TOTAL_WAVETABLE_COUNT);
    pydaw_set_pyfx_port(f_result, WAYV_OSC6_PITCH, 0.0f, -72.0f, 72.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC6_TUNE, 0.0f, -100.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC6_VOLUME, -6.0f, -30.0f, 0.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC6_UNISON_VOICES, 1.0f, 1.0f, 7.0f);
    pydaw_set_pyfx_port(f_result, WAYV_OSC6_UNISON_SPREAD, 50.0f, 0.0f, 100.0f);
    pydaw_set_pyfx_port(f_result, WAYV_ADSR6_DELAY, 0.0f, 0.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, WAYV_ATTACK6, 10.0f, 0.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, WAYV_ADSR6_HOLD, 0.0f, 0.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, WAYV_DECAY6, 50.0f, 10.0f, 200.0f);
    pydaw_set_pyfx_port(f_result, WAYV_SUSTAIN6, 0.0f, -30.0f, 0.0f);
    pydaw_set_pyfx_port(f_result, WAYV_RELEASE6, 50.0f, 10.0f, 400.0f);
    pydaw_set_pyfx_port(f_result, WAYV_ADSR6_CHECKBOX, 0.0f, 0, 1);
    pydaw_set_pyfx_port(f_result, WAYV_MIN_NOTE, 0.0f, 0.0f, 120.0f);
    pydaw_set_pyfx_port(f_result, WAYV_MAX_NOTE, 120.0f, 0.0f, 120.0f);

    f_port = WAYV_FM_MACRO1_OSC1_FM5;

    while(f_port <= WAYV_FM_MACRO2_OSC6_VOL)
    {
        pydaw_set_pyfx_port(f_result, f_port, 0.0f, -100.0f, 100.0f);
        f_port++;
    }

    f_port = WAYV_OSC5_FM1;

    while(f_port <= WAYV_OSC6_FM4)
    {
        pydaw_set_pyfx_port(f_result, f_port, 0.0f, 0.0f, 100.0f);
        f_port++;
    }

    f_port = WAYV_OSC1_FM5;

    while(f_port <= WAYV_OSC6_FM5)
    {
        pydaw_set_pyfx_port(f_result, f_port, 0.0f, 0.0f, 100.0f);
        f_port++;
    }

    f_port = WAYV_OSC1_FM6;

    while(f_port <= WAYV_OSC6_FM6)
    {
        pydaw_set_pyfx_port(f_result, f_port, 0.0f, 0.0f, 100.0f);
        f_port++;
    }

    pydaw_set_pyfx_port(f_result, WAYV_NOISE_PREFX, 0.0f, 0, 1);

    f_port = WAVV_PFXMATRIX_GRP0DST0SRC6CTRL0;

    while(f_port <= WAVV_PFXMATRIX_GRP0DST3SRC7CTRL2)
    {
        pydaw_set_pyfx_port(f_result, f_port,  0.0f, -100.0f, 100.0f);
        f_port++;
    }

    f_result->activate = v_wayv_activate;
    f_result->cleanup = v_cleanup_wayv;
    f_result->connect_port = v_wayv_connect_port;
    f_result->connect_buffer = v_wayv_connect_buffer;
    f_result->deactivate = NULL;
    f_result->instantiate = g_wayv_instantiate;
    f_result->panic = wayvPanic;


    return f_result;
}

PYINST_Descriptor *wayv_PYINST_descriptor(int index)
{
    PYINST_Descriptor *LMSDDescriptor = NULL;

    LMSDDescriptor = (PYINST_Descriptor *) malloc(sizeof(PYINST_Descriptor));

    if (LMSDDescriptor)
    {
	LMSDDescriptor->PYINST_API_Version = 1;
	LMSDDescriptor->PYFX_Plugin = wayv_PYFX_descriptor(0);
	LMSDDescriptor->configure = c_wayv_configure;
	LMSDDescriptor->run_synth = v_run_wayv;
        LMSDDescriptor->offline_render_prep = v_wayv_or_prep;
        LMSDDescriptor->on_stop = v_wayv_on_stop;
    }

    return LMSDDescriptor;
}
