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

#ifndef ADSR_H
#define	ADSR_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../../constants.h"
#include "../../lib/amp.h"

#define ADSR_DB 18.0f
#define ADSR_DB_THRESHOLD -18.0f
#define ADSR_DB_THRESHOLD_LINEAR 0.125f

#define ADSR_DB_RELEASE 48.0f
#define ADSR_DB_THRESHOLD_RELEASE -48.0f
#define ADSR_DB_THRESHOLD_LINEAR_RELEASE  0.00390625f

#define ADSR_STAGE_DELAY 0
#define ADSR_STAGE_ATTACK 1
#define ADSR_STAGE_HOLD 2
#define ADSR_STAGE_DECAY 3
#define ADSR_STAGE_SUSTAIN 4
#define ADSR_STAGE_RELEASE 5
#define ADSR_STAGE_OFF 6

typedef struct st_adsr
{
    float output;
    float output_db;
    int stage;  //0=a,1=d,2=s,3=r,4=inactive,6=delay,9=hold
    float a_inc;
    float d_inc;
    float s_value;
    float r_inc;

    float a_inc_db;
    float d_inc_db;
    float r_inc_db;

    float a_time;
    float d_time;
    float r_time;
    
    int time_counter;
    int delay_count;
    int hold_count;

    float sr;
    float sr_recip;
}t_adsr;

void v_adsr_set_a_time(t_adsr*, float);
void v_adsr_set_d_time(t_adsr*, float);
void v_adsr_set_s_value(t_adsr*, float);
void v_adsr_set_s_value_db(t_adsr*, float);
void v_adsr_set_r_time(t_adsr*, float);
void v_adsr_set_fast_release(t_adsr*);

void v_adsr_set_delay_time(t_adsr*, float);
void v_adsr_set_hold_time(t_adsr*, float);

void v_adsr_set_adsr_db(t_adsr*, float, float, float, float);
void v_adsr_set_adsr(t_adsr*, float, float, float, float);

void v_adsr_retrigger(t_adsr *);
void v_adsr_release(t_adsr *);
void v_adsr_run(t_adsr *);
void v_adsr_run_db(t_adsr *);
void v_adsr_kill(t_adsr *);

t_adsr * g_adsr_get_adsr(float);

/* void v_adsr_set_delay_time(t_adsr* a_adsr, float a_time)
 *
 * MUST BE CALLED AFTER RETRIGGER!!!!
 */
void v_adsr_set_delay_time(t_adsr* a_adsr, float a_time)
{
    if(a_time == 0.0f)
    {
        a_adsr->delay_count = 0;
    }
    else
    {
        a_adsr->stage = ADSR_STAGE_DELAY;
        a_adsr->delay_count = (int)(a_time * a_adsr->sr);
    }
}

void v_adsr_set_hold_time(t_adsr* a_adsr, float a_time)
{
    if(a_time == 0.0f)
    {
        a_adsr->hold_count = 0;
    }
    else
    {
        a_adsr->hold_count = (int)(a_time * a_adsr->sr);
    }
}


/* void v_adsr_set_a_time(
 * t_adsr* a_adsr_ptr,
 * float a_time)  //time in seconds
 *
 * Sets the envelope attack time
 */
void v_adsr_set_a_time(t_adsr*__restrict a_adsr_ptr, float a_time)
{
    if((a_adsr_ptr->a_time) == a_time)
        return;

    a_adsr_ptr->a_time = a_time;

    if(a_time <= 0.0f)
    {
        a_adsr_ptr->a_inc = 1.0f;
    }
    else
    {
        a_adsr_ptr->a_inc = (a_adsr_ptr->sr_recip) / (a_adsr_ptr->a_time);
    }

}

/* void v_adsr_set_d_time(
 * t_adsr* a_adsr_ptr,
 * float a_time) //time in seconds
 *
 * Sets the envelope decay time
 */
void v_adsr_set_d_time(t_adsr*__restrict a_adsr_ptr, float a_time)
{
    if((a_adsr_ptr->d_time) == a_time)
        return;

    if(a_time <= 0.0f)
    {
        a_adsr_ptr->d_time = .05;
    }
    else
    {
        a_adsr_ptr->d_time = a_time;
    }

    a_adsr_ptr->d_inc = ((a_adsr_ptr->sr_recip) / (a_adsr_ptr->d_time)) * -1.0f;
}

/* void v_adsr_set_r_time(
 * t_adsr* a_adsr_ptr,
 * float a_time) //time in seconds
 *
 * Sets the envelope release time
 */
void v_adsr_set_r_time(t_adsr*__restrict a_adsr_ptr, float a_time)
{
    if((a_adsr_ptr->r_time) == a_time)
        return;

    if(a_time <= 0.0f)
    {
        a_adsr_ptr->r_time = .05f;
    }
    else
    {
        a_adsr_ptr->r_time = a_time;
    }

    a_adsr_ptr->r_inc = ((a_adsr_ptr->sr_recip) / (a_adsr_ptr->r_time)) * -1.0f;
}

/* void v_adsr_set_fast_release(t_adsr* a_adsr_ptr)
 *
 * This method is for killing voices by allowing a quick fade-out
 * instead of directly stealing a voice, which should
 * allow a quick transition without a click
 * TODO:  The total time of the fadeout is not consistent
 * between different sample rates.
 */
void v_adsr_set_fast_release(t_adsr*__restrict a_adsr_ptr)
{
    a_adsr_ptr->r_time = .01f;
    a_adsr_ptr->r_inc = -.002f;
    a_adsr_ptr->r_inc_db = ((a_adsr_ptr->sr) * -0.01f) / ADSR_DB_RELEASE;
    a_adsr_ptr->stage = ADSR_STAGE_RELEASE;
}

/* void v_adsr_set_s_value(
 * t_adsr* a_adsr_ptr,
 * float a_value) //The sustain value, range: 0 to 1
 */
void v_adsr_set_s_value(t_adsr*__restrict a_adsr_ptr, float a_value)
{
    a_adsr_ptr->s_value = a_value;

    if((a_adsr_ptr->s_value) <= 0.0f)
    {
        a_adsr_ptr->s_value = .001f;
    }
}

/* void v_adsr_set_s_value_db(
 * t_adsr* a_adsr_ptr,
 * float a_value)  //The decibel value of sustain, typically -30 to 0
 */
void v_adsr_set_s_value_db(t_adsr*__restrict a_adsr_ptr, float a_value)
{
    v_adsr_set_s_value(a_adsr_ptr, f_db_to_linear_fast(a_value));
}

/* void v_adsr_set_adsr(
 * t_adsr* a_adsr_ptr,
 * float a_a, //attack
 * float a_d, //decay
 * float a_s, //sustain
 * float a_r) //release
 *
 * Set allADSR values, with a range of 0 to 1 for sustain
 */
void v_adsr_set_adsr(t_adsr*__restrict a_adsr_ptr, float a_a,
        float a_d, float a_s, float a_r)
{
    v_adsr_set_a_time(a_adsr_ptr, a_a);
    v_adsr_set_d_time(a_adsr_ptr, a_d);
    v_adsr_set_s_value(a_adsr_ptr, a_s);
    v_adsr_set_r_time(a_adsr_ptr, a_r);
}


/* void v_adsr_set_adsr(
 * t_adsr* a_adsr_ptr,
 * float a_a, //attack
 * float a_d, //decay
 * float a_s, //sustain
 * float a_r) //release
 *
 * Set all ADSR values, with a range of -30 to 0 for sustain
 */
void v_adsr_set_adsr_db(t_adsr*__restrict a_adsr_ptr, float a_a,
        float a_d, float a_s, float a_r)
{
    v_adsr_set_a_time(a_adsr_ptr, a_a);
    v_adsr_set_d_time(a_adsr_ptr, a_d);
    v_adsr_set_s_value_db(a_adsr_ptr, a_s);
    v_adsr_set_r_time(a_adsr_ptr, a_r);

    a_adsr_ptr->a_inc_db = (a_adsr_ptr->a_inc) * ADSR_DB;
    a_adsr_ptr->d_inc_db = (a_adsr_ptr->d_inc) * (ADSR_DB);
    a_adsr_ptr->r_inc_db = (a_adsr_ptr->r_inc) * (ADSR_DB_RELEASE);
}

/* void v_adsr_retrigger(t_adsr * a_adsr_ptr)
 *
 * Reset the ADSR to the beginning of the attack phase
 */
void v_adsr_retrigger(t_adsr *__restrict a_adsr_ptr)
{
    a_adsr_ptr->stage = ADSR_STAGE_ATTACK;
    a_adsr_ptr->output = 0.0f;
    a_adsr_ptr->output_db = ADSR_DB_THRESHOLD;
    a_adsr_ptr->time_counter = 0;
}

void v_adsr_kill(t_adsr *__restrict a_adsr_ptr)
{
    a_adsr_ptr->stage = ADSR_STAGE_OFF;
    a_adsr_ptr->output = 0.0f;
    a_adsr_ptr->output_db = ADSR_DB_THRESHOLD;
}

/* void v_adsr_release(t_adsr * a_adsr_ptr)
 *
 * Set the ADSR to the release phase
 */
void v_adsr_release(t_adsr *__restrict a_adsr_ptr)
{
    if(a_adsr_ptr->stage < ADSR_STAGE_RELEASE)
    {
        a_adsr_ptr->stage = ADSR_STAGE_RELEASE;
    }
}

void g_adsr_init(t_adsr * f_result, float a_sr)
{
    f_result->sr = a_sr;
    f_result->sr_recip = 1.0f / a_sr;

    f_result->output = 0.0f;
    f_result->stage = ADSR_STAGE_OFF;

    //Set these to nonsensical values so that comparisons aren't
    //happening with invalid numbers
    f_result->a_inc = -100.5f;
    f_result->a_time = -100.5f;
    f_result->d_inc = -100.5f;
    f_result->d_time =  -100.5f;
    f_result->r_inc = -100.5f;
    f_result->r_time = -100.5f;
    f_result->s_value = -100.5f;

    f_result->output_db = -100.5f;
    f_result->a_inc_db = 0.1f;
    f_result->d_inc_db = 0.1f;
    f_result->r_inc_db = 0.1f;

    v_adsr_set_a_time(f_result, .05);
    v_adsr_set_d_time(f_result, .5);
    v_adsr_set_s_value_db(f_result, -12.0f);
    v_adsr_set_r_time(f_result, .5);

    f_result->time_counter = 0;
    f_result->delay_count = 0;
    f_result->hold_count = 0;
}

/* t_adsr * g_adsr_get_adsr(
 * float a_sr_recip) // 1.0f/sample_rate (TODO: use sample_rate instead)
 *
 */
t_adsr * g_adsr_get_adsr(float a_sr)
{
    t_adsr * f_result;
    hpalloc((void**)&f_result, sizeof(t_adsr));
    g_adsr_init(f_result, a_sr);
    return f_result;
}

/* void v_adsr_run(t_adsr * a_adsr_ptr)
 *
 * Run the ADSR envelope
 */
void v_adsr_run(t_adsr *__restrict a_adsr_ptr)
{
    if((a_adsr_ptr->stage) != ADSR_STAGE_OFF)
    {
        switch(a_adsr_ptr->stage)
        {
            case ADSR_STAGE_ATTACK:
                a_adsr_ptr->output = (a_adsr_ptr->output) + (a_adsr_ptr->a_inc);
                if((a_adsr_ptr->output) >= 1.0f)
                {
                    a_adsr_ptr->output = 1.0f;

                    if(a_adsr_ptr->hold_count)
                    {
                        a_adsr_ptr->stage = ADSR_STAGE_HOLD;
                    }
                    else
                    {
                        a_adsr_ptr->stage = ADSR_STAGE_DECAY;
                    }
                }
                break;
            case ADSR_STAGE_DECAY:
                a_adsr_ptr->output =
                        (a_adsr_ptr->output) + (a_adsr_ptr->d_inc);
                if((a_adsr_ptr->output) <= (a_adsr_ptr->s_value))
                {
                    a_adsr_ptr->output = a_adsr_ptr->s_value;
                    a_adsr_ptr->stage = ADSR_STAGE_SUSTAIN;
                }
                break;
            case ADSR_STAGE_SUSTAIN:
                break;
            case ADSR_STAGE_RELEASE:
                a_adsr_ptr->output =
                        (a_adsr_ptr->output) + (a_adsr_ptr->r_inc);
                if((a_adsr_ptr->output) <= 0.0f)
                {
                    a_adsr_ptr->output = 0.0f;
                    a_adsr_ptr->stage = ADSR_STAGE_OFF;
                }
                break;
            case ADSR_STAGE_DELAY:
                ++a_adsr_ptr->time_counter;
                if(a_adsr_ptr->time_counter >= a_adsr_ptr->delay_count)
                {
                    a_adsr_ptr->stage = ADSR_STAGE_ATTACK;
                    a_adsr_ptr->time_counter = 0;
                }
                break;
            case ADSR_STAGE_HOLD:
                ++a_adsr_ptr->time_counter;
                if(a_adsr_ptr->time_counter >= a_adsr_ptr->hold_count)
                {
                    a_adsr_ptr->stage = ADSR_STAGE_DECAY;
                    a_adsr_ptr->time_counter = 0;
                }
                break;
        }
    }
}

void v_adsr_run_db(t_adsr *__restrict a_adsr_ptr)
{
    if((a_adsr_ptr->stage) != ADSR_STAGE_OFF)
    {
        switch(a_adsr_ptr->stage)
        {
            case ADSR_STAGE_ATTACK:
                if((a_adsr_ptr->output) < ADSR_DB_THRESHOLD_LINEAR)
                {
                    a_adsr_ptr->output = (a_adsr_ptr->output) + 0.005f;
                }
                else
                {
                    a_adsr_ptr->output_db =
                            (a_adsr_ptr->output_db) + (a_adsr_ptr->a_inc_db);
                    a_adsr_ptr->output =
                            f_db_to_linear_fast((a_adsr_ptr->output_db));

                    if((a_adsr_ptr->output) >= 1.0f)
                    {
                        a_adsr_ptr->output = 1.0f;

                        if(a_adsr_ptr->hold_count)
                        {
                            a_adsr_ptr->stage = ADSR_STAGE_HOLD;
                        }
                        else
                        {
                            a_adsr_ptr->stage = ADSR_STAGE_DECAY;
                        }
                    }
                }

                break;
            case ADSR_STAGE_DECAY:
                if((a_adsr_ptr->output) < ADSR_DB_THRESHOLD_LINEAR)
                {
                    a_adsr_ptr->output =
                            (a_adsr_ptr->output) + (a_adsr_ptr->d_inc);
                }
                else
                {
                    a_adsr_ptr->output_db =
                            (a_adsr_ptr->output_db) + (a_adsr_ptr->d_inc_db);
                    a_adsr_ptr->output =
                            f_db_to_linear_fast(a_adsr_ptr->output_db);
                }

                if((a_adsr_ptr->output) <= (a_adsr_ptr->s_value))
                {
                    a_adsr_ptr->output = a_adsr_ptr->s_value;
                    a_adsr_ptr->stage = ADSR_STAGE_SUSTAIN;
                }
                break;
            case ADSR_STAGE_SUSTAIN:
                break;
            case ADSR_STAGE_RELEASE:
                if((a_adsr_ptr->output) < ADSR_DB_THRESHOLD_LINEAR_RELEASE)
                {
                    a_adsr_ptr->output_db = (a_adsr_ptr->output_db) - 0.05f;

                    if(a_adsr_ptr->output_db < -96.0f)
                    {
                        a_adsr_ptr->stage = ADSR_STAGE_OFF;
                        a_adsr_ptr->output = 0.0f;
                    }
                    else
                    {
                        a_adsr_ptr->output =
                            f_db_to_linear_fast(a_adsr_ptr->output_db);
                    }
                }
                else
                {
                    a_adsr_ptr->output_db = (a_adsr_ptr->output_db) +
                            (a_adsr_ptr->r_inc_db);
                    a_adsr_ptr->output =
                            f_db_to_linear_fast(a_adsr_ptr->output_db);
                }

                break;
            case ADSR_STAGE_DELAY:
                ++a_adsr_ptr->time_counter;
                if(a_adsr_ptr->time_counter >= a_adsr_ptr->delay_count)
                {
                    a_adsr_ptr->stage = ADSR_STAGE_ATTACK;
                    a_adsr_ptr->time_counter = 0;
                }
                break;
            case ADSR_STAGE_HOLD:
                ++a_adsr_ptr->time_counter;
                if(a_adsr_ptr->time_counter >= a_adsr_ptr->hold_count)
                {
                    a_adsr_ptr->stage = ADSR_STAGE_DECAY;
                    a_adsr_ptr->time_counter = 0;
                }
                break;
        }
    }
}


#ifdef	__cplusplus
}
#endif

#endif	/* ADSR_H */

