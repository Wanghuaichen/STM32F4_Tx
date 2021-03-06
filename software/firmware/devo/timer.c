/*
 This project is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Deviation is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Deviation.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "common.h"
#include "timer.h"
#include "music.h"
#include "model.h"
#include "tx.h"
#include <stdlib.h>

static uint8_t timer_state[NUM_TIMERS];
static s32 timer_val[NUM_TIMERS];
static s32 last_time[NUM_TIMERS];

void TIMER_SetString(char *str, s32 time)
{
    //uint8_t h = time / 3600;
    //uint8_t m = (time - h*3600) / 60;
    //uint8_t s = time -h*3600 - m*60;
    //sprintf(str, "%02d:%02d:%02d", h, m, s);
    char neg;
    if (time < 0) {
        neg = 1;
        time = -time;
    } else {
        neg = 0;
    }
    time = time / 1000; //Convert to seconds
    unsigned h = time / 3600;
    unsigned m = (time - h*3600) / 60;
    unsigned s = time -h*3600 - m*60;
    if( h < 1)
         sprintf(str, "%s%02d:%02d", neg ? "-" : "", m, s);
    else
        sprintf(str, "%s%02d:%02d:%02d", neg ? "-" : "", h, m, s);
}

const char *TIMER_Name(char *str, unsigned timer)
{
    sprintf(str, "%s%d", _tr("Timer"), timer+1);
    return str;
}

void TIMER_StartStop(unsigned timer)
{
    if (Model.timer[timer].type == TIMER_STOPWATCH_PROP ||
        Model.timer[timer].type == TIMER_COUNTDOWN_PROP)
    {
        return;
    }
    timer_state[timer] ^= 1;
    if(timer_state[timer]) {
        last_time[timer] = CLOCK_getms();
    }
}

void TIMER_Reset(unsigned timer)
{
    if (Model.timer[timer].type == TIMER_STOPWATCH_PROP
        || Model.timer[timer].type == TIMER_COUNTDOWN_PROP)
    {
        timer_state[timer] = 1;
        last_time[timer] = CLOCK_getms();
    } else {
        timer_state[timer] = 0;
    }
    if (Model.timer[timer].type == TIMER_STOPWATCH || Model.timer[timer].type == TIMER_STOPWATCH_PROP) {
        timer_val[timer] = 0;
    } else if(Model.timer[timer].type == TIMER_COUNTDOWN
              || Model.timer[timer].type == TIMER_COUNTDOWN_PROP)
    {
        timer_val[timer] = Model.timer[timer].timer * 1000;
    } else if(Model.timer[timer].type == TIMER_PERMANENT ) {
        timer_val[timer] = Model.timer[timer].val;
    }
}

s32 TIMER_GetValue(unsigned timer)
{
    return timer_val[timer];
}

void TIMER_SetValue(unsigned timer, s32 value)
{
    if (Model.timer[timer].type == TIMER_PERMANENT) {
        timer_val[timer] = value;
    }
}

void TIMER_Init()
{
    unsigned i;
    for (i = 0; i < NUM_TIMERS; i++)
        TIMER_Reset(i);
}

void TIMER_Power(){
    static uint32_t timer = 0;
    uint32_t alert = Transmitter.power_alarm * 60 * 1000;
    static uint16_t throttle;
    uint16_t new_throttle;
    uint16_t elevator;
    unsigned mode = MODE_2 == Transmitter.mode || MODE_4 == Transmitter.mode ? 2 : 1;

    if( 0 == timer)
        timer =  CLOCK_getms() + alert;

    elevator = 2 == mode ? abs(CHAN_ReadInput(INP_THROTTLE)) : abs(CHAN_ReadInput(INP_ELEVATOR));
    new_throttle = 2 == mode ?  abs(CHAN_ReadInput(INP_ELEVATOR)) : abs(CHAN_ReadInput(INP_THROTTLE));
    new_throttle = abs(new_throttle - throttle);
     
    if( elevator < 1000 && abs(CHAN_ReadInput(INP_AILERON)) < 1000 && 
                new_throttle < 1000 && abs(CHAN_ReadInput(INP_RUDDER)) < 1000 &&
                !ScanButtons() && (!HAS_TOUCH || !SPITouch_IRQ()) ) {
        if ( CLOCK_getms() > timer ) {
            timer =  CLOCK_getms() + 2000;
            MUSIC_Play(MUSIC_SHUTDOWN);
        }                
    } else 
           timer =  CLOCK_getms() + alert;
    throttle = 2 == mode ?  abs(CHAN_ReadInput(INP_ELEVATOR)) : abs(CHAN_ReadInput(INP_THROTTLE));
}

void TIMER_Update()
{
    unsigned i;
    unsigned chan_val = 0;
    uint32_t t = CLOCK_getms();
    if (PROTOCOL_WaitingForSafe())
        return;
    if( Transmitter.power_alarm > 0 )
        TIMER_Power();
    for (i = 0; i < NUM_TIMERS; i++) {
        if (Model.timer[i].src) {
            s16 val;
            if (MIXER_SRC(Model.timer[i].src) <= NUM_INPUTS) {
                volatile s16 *raw = MIXER_GetInputs();
                val = raw[MIXER_SRC(Model.timer[i].src)];
            } else {
                val = MIXER_GetChannel(Model.timer[i].src - NUM_INPUTS - 1, APPLY_SAFETY);
            }
            if (MIXER_SRC_IS_INV(Model.timer[i].src))
                val = -val;
            if (Model.timer[i].type == TIMER_STOPWATCH_PROP
                || Model.timer[i].type == TIMER_COUNTDOWN_PROP)
            {
               chan_val = RANGE_TO_PCT(abs(val));
               if (chan_val > 100)
                   chan_val = 100;
            } else {
                unsigned new_state = (val - CHAN_MIN_VALUE > (CHAN_MAX_VALUE - CHAN_MIN_VALUE) / 20) ? 1 : 0;
                if (new_state != timer_state[i]) {
                    if (new_state)
                        last_time[i] = t;
                    timer_state[i] = new_state;
                }
            }
        }
        if (timer_state[i]) {
            s32 delta = t - last_time[i];
            if (Model.timer[i].type == TIMER_STOPWATCH_PROP || Model.timer[i].type == TIMER_COUNTDOWN_PROP) {
                delta = delta * chan_val / 100;
            }
            if (Model.timer[i].type == TIMER_PERMANENT) {
                timer_val[i] += delta;
                if( timer_val[i] >= 359999900) // Reset when 99h59mn59sec
                    timer_val[i] = 0 ;
                Model.timer[i].val = timer_val[i];
            } else if (Model.timer[i].type == TIMER_STOPWATCH || Model.timer[i].type == TIMER_STOPWATCH_PROP) {
                timer_val[i] += delta;
            } else {
                s32 warn_time;
                // start to beep  for each prealert_interval at the last prealert_time(seconds)
                if (Transmitter.countdown_timer_settings.prealert_time != 0 &&
                    Transmitter.countdown_timer_settings.prealert_interval != 0 &&
                    timer_val[i] > Transmitter.countdown_timer_settings.prealert_interval &&
                    timer_val[i] < (s32)Transmitter.countdown_timer_settings.prealert_time + 1000) { // give extra 1seconds
                    warn_time = ((timer_val[i] / Transmitter.countdown_timer_settings.prealert_interval)
                            * Transmitter.countdown_timer_settings.prealert_interval);
                    if (timer_val[i] > warn_time && (timer_val[i] - delta) <= warn_time) {
                        MUSIC_Play(MUSIC_TIMER_WARNING);
                    }
                }
                // Beep once for each timeup_interval past 0
                if (timer_val[i] < 0 && Transmitter.countdown_timer_settings.timeup_interval != 0) {
                    warn_time = ((timer_val[i] - Transmitter.countdown_timer_settings.timeup_interval) / Transmitter.countdown_timer_settings.timeup_interval)
                            * Transmitter.countdown_timer_settings.timeup_interval;
                    if (timer_val[i] > warn_time && (timer_val[i] - delta) <= warn_time) {
                        MUSIC_Play(MUSIC_ALARM1 + i);
                    }
                }
                if (timer_val[i] >= 0 && timer_val[i] < delta) {
                    MUSIC_Play(MUSIC_ALARM1 + i);
                }
                timer_val[i] -= delta;
            }
            last_time[i] = t;
        } 
        if (Model.timer[i].resetsrc) {
            s16 val;
            if (MIXER_SRC(Model.timer[i].resetsrc) <= NUM_INPUTS) {
                volatile s16 *raw = MIXER_GetInputs();
                val = raw[MIXER_SRC(Model.timer[i].resetsrc)];
            } else {
                val = MIXER_GetChannel(Model.timer[i].resetsrc - NUM_INPUTS - 1, APPLY_SAFETY);
            }
            if (MIXER_SRC_IS_INV(Model.timer[i].resetsrc))
                val = -val;
            if (val - CHAN_MIN_VALUE > (CHAN_MAX_VALUE - CHAN_MIN_VALUE) / 20) {
                TIMER_Reset(i);
            }
        }
    }
}
