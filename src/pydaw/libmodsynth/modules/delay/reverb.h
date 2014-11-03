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

#ifndef PYDAW_REVERB_H
#define	PYDAW_REVERB_H

#define PYDAW_REVERB_DIFFUSER_COUNT 4
#define PYDAW_REVERB_TAP_COUNT 8


#include "../../lib/amp.h"
#include "../../lib/lmalloc.h"
#include "../../lib/pitch_core.h"
#include "../filter/comb_filter.h"
#include "../filter/svf.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct
{
    float color;
    float wet;
    float wet_linear;
    float time;
    float output;
    float volume_factor;
    t_state_variable_filter diffusers[PYDAW_REVERB_DIFFUSER_COUNT];
    t_state_variable_filter hp;
    t_state_variable_filter lp;
    t_comb_filter taps[PYDAW_REVERB_TAP_COUNT];
    float comb_tunings[PYDAW_REVERB_TAP_COUNT];
    float allpass_tunings[PYDAW_REVERB_DIFFUSER_COUNT];
    int predelay_counter;
    int predelay_size;
    float last_predelay;
    float * predelay_buffer;
    float sr;
} t_rvb_reverb;

t_rvb_reverb * g_rvb_reverb_get(float);
void v_rvb_reverb_set(t_rvb_reverb *, float, float, float, float);
inline void v_rvb_reverb_run(t_rvb_reverb *, float, float);

/* void v_rvb_reverb_set(t_rvb_reverb * a_reverb,
 * float a_time,  //0 to 1, not attempting to use RT60 because the algo
 *                //will be non-standard and may change...
 * float a_wet, //0 to 1, I may change the meaning later...
 * float a_color) //0 to 1, I may change the meaning later...
 */
void v_rvb_reverb_set(t_rvb_reverb * a_reverb, float a_time, float a_wet,
        float a_color, float a_predelay)
{
    if(unlikely(a_time != a_reverb->time))
    {
        a_reverb->time = a_time;

        float f_feedback = (a_time) + -1.10f;

        int f_i2 = 0;

        while(f_i2 < PYDAW_REVERB_TAP_COUNT)
        {
            v_cmb_set_all(&a_reverb->taps[f_i2], 0.0f, f_feedback,
                    a_reverb->comb_tunings[f_i2]);
            ++f_i2;
        }
    }

    if(unlikely(a_wet != a_reverb->wet))
    {
        a_reverb->wet = a_wet;
        a_reverb->wet_linear =  a_wet * (a_reverb->volume_factor);
    }

    if(unlikely(a_color != a_reverb->color))
    {
        a_reverb->color = a_color;

        float f_cutoff = (a_color * 40.0f) + 70.0f;

        v_svf_set_cutoff_base(&a_reverb->lp, f_cutoff);
        v_svf_set_cutoff(&a_reverb->lp);
    }

    if(unlikely(a_reverb->last_predelay != a_predelay))
    {
        a_reverb->last_predelay = a_predelay;
        a_reverb->predelay_size = (int)(a_reverb->sr * a_predelay);
        if(a_reverb->predelay_counter >= a_reverb->predelay_size)
        {
            a_reverb->predelay_counter = 0;
        }
    }
}

inline void v_rvb_reverb_run(t_rvb_reverb * a_reverb, float a_input0,
        float a_input1)
{
    int iter1 = 0;

    a_reverb->output = 0.0f;

    float f_tmp_sample = v_svf_run_2_pole_lp(&a_reverb->lp,
            (a_input0 + a_input1));
    f_tmp_sample = v_svf_run_2_pole_hp(&a_reverb->hp, f_tmp_sample);
    f_tmp_sample *= (a_reverb->wet_linear);

    while((iter1) < PYDAW_REVERB_TAP_COUNT)
    {
        v_cmb_run(&a_reverb->taps[iter1], f_tmp_sample);

        a_reverb->output += (a_reverb->taps[iter1].output_sample);

        ++iter1;
    }

    iter1 = 0;

    while((iter1) < PYDAW_REVERB_DIFFUSER_COUNT)
    {
        a_reverb->output =
            v_svf_run_2_pole_allpass(&a_reverb->diffusers[iter1],
            a_reverb->output);
        ++iter1;
    }

    a_reverb->predelay_buffer[(a_reverb->predelay_counter)] = a_reverb->output;
    ++a_reverb->predelay_counter;
    if(unlikely(a_reverb->predelay_counter >= a_reverb->predelay_size))
    {
        a_reverb->predelay_counter = 0;
    }
    a_reverb->output = a_reverb->predelay_buffer[(a_reverb->predelay_counter)];

}

t_rvb_reverb * g_rvb_reverb_get(float a_sr)
{
    t_rvb_reverb * f_result;

    hpalloc((void**)&f_result, sizeof(t_rvb_reverb));

    f_result->color = 1.0f;
    f_result->time = 0.5f;
    f_result->wet = 0.0f;
    f_result->wet_linear = 0.0f;

    f_result->sr = a_sr;

    f_result->comb_tunings[0] = 24.0f;
    f_result->comb_tunings[1] = 25.0f;
    f_result->comb_tunings[2] = 26.0f;
    f_result->comb_tunings[3] = 27.0f;
    f_result->comb_tunings[4] = 28.0f;
    f_result->comb_tunings[5] = 29.0f;
    f_result->comb_tunings[6] = 30.0f;
    f_result->comb_tunings[7] = 31.0f;

    f_result->allpass_tunings[0] = 33.0f;
    f_result->allpass_tunings[1] = 40.0f;
    f_result->allpass_tunings[2] = 47.0f;
    f_result->allpass_tunings[3] = 54.0f;

    f_result->output = 0.0f;

    g_svf_init(&f_result->hp, a_sr);
    v_svf_set_cutoff_base(&f_result->hp, 60.0f);
    v_svf_set_res(&f_result->hp, -24.0f);
    v_svf_set_cutoff(&f_result->hp);

    g_svf_init(&f_result->lp, a_sr);
    v_svf_set_res(&f_result->lp, -36.0f);

    f_result->volume_factor = (1.0f / (float)PYDAW_REVERB_DIFFUSER_COUNT) * 0.5;

    int f_i2 = 0;

    while(f_i2 < PYDAW_REVERB_TAP_COUNT)
    {
        g_cmb_init(&f_result->taps[f_i2], a_sr, 1);
        ++f_i2;
    }

    f_i2 = 0;

    while(f_i2 < PYDAW_REVERB_DIFFUSER_COUNT)
    {
        g_svf_init(&f_result->diffusers[f_i2], a_sr);
        v_svf_set_cutoff_base(&f_result->diffusers[f_i2],
            f_result->allpass_tunings[f_i2]);
        v_svf_set_res(&f_result->diffusers[f_i2], -1.0f);
        v_svf_set_cutoff(&f_result->diffusers[f_i2]);
        ++f_i2;
    }

    f_result->predelay_counter = 0;
    f_result->last_predelay = -1234.0f;
    f_result->predelay_size = (int)(a_sr * 0.01f);

    hpalloc((void**)&f_result->predelay_buffer,
        sizeof(float) * (a_sr + 5000));

    f_i2 = 0;
    while(f_i2 < (a_sr + 5000))
    {
        f_result->predelay_buffer[f_i2] = 0.0f;
        ++f_i2;
    }

    v_rvb_reverb_set(f_result, 0.5f, 0.0f, 0.5f, 0.01f);

    return f_result;
}

#ifdef	__cplusplus
}
#endif

#endif	/* PYDAW_REVERB_H */

