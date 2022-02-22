/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2022-01-19 11:47:17.281000.
*/
/**
\brief Definition of the "opentimers" driver.

This driver uses a single hardware timer, which it virtualizes to support
at most MAX_NUM_TIMERS timers.

\author Tengfei Chang <tengfei.chang@inria.fr>, April 2017.
 */

#include "opendefs_obj.h"
#include "opentimers_obj.h"
// some dsp modules are required
#include "sctimer_obj.h"
#include "debugpins_obj.h"
// kernel module is required
#include "scheduler_obj.h"

//=========================== define ==========================================

//=========================== variables =======================================

// declaration of global variable _opentimers_vars_ removed during objectification.

//=========================== prototypes ======================================

void opentimers_timer_callback(OpenMote* self);

//=========================== public ==========================================

/**
\brief Initialize this module.

Initializes data structures and hardware timer.
 */
void opentimers_init(OpenMote* self){

    // initialize local variables
    memset(&(self->opentimers_vars),0,sizeof(opentimers_vars_t));

    // set callback for sctimer module
 sctimer_set_callback(self, opentimers_timer_callback);
}

/**
\brief create a timer by assigning an entry from timer buffer.

create a timer with given id or assigning one if it's general purpose timer.
task_prio gives a priority when opentimer push a task.

\returns the id of the timer will be returned
 */
opentimers_id_t opentimers_create(OpenMote* self, uint8_t timer_id, uint8_t task_prio){
    uint8_t id;

    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();

    if (timer_id==TIMER_TSCH || timer_id==TIMER_INHIBIT){
        if ((self->opentimers_vars).timersBuf[timer_id].isUsed  == FALSE){
            (self->opentimers_vars).timersBuf[timer_id].isUsed   = TRUE;
            // the TSCH timer and inhibit timer won't push a task,
            // hence task_prio is not used
            return timer_id;
        }
    }

    if (timer_id==TIMER_GENERAL_PURPOSE){
        for (id=TIMER_NUMBER_NON_GENERAL;id<MAX_NUM_TIMERS;id++){
            if ((self->opentimers_vars).timersBuf[id].isUsed  == FALSE){
                (self->opentimers_vars).timersBuf[id].isUsed   = TRUE;
                (self->opentimers_vars).timersBuf[id].timer_task_prio = task_prio;
                return id;
            }
        }
    }

    ENABLE_INTERRUPTS();

    // there is no available buffer for this timer
    return ERROR_NO_AVAILABLE_ENTRIES;
}

/**
\brief schedule a period refer to comparing value set last time.

This function will schedule a timer which expires when the timer count reach
to current counter + duration.

\param[in] id indicates the timer id
\param[in] duration indicates the period asked for schedule since last comparing value
\param[in] uint_type indicates the unit type of this schedule: ticks or ms
\param[in] timer_type indicates the timer type of this schedule: oneshot or periodic
\param[in] cb indicates when this scheduled timer fired, call this callback function.
 */
void opentimers_scheduleIn(OpenMote* self, opentimers_id_t    id,
                           uint32_t           duration,
                           time_type_t        uint_type,
                           timer_type_t       timer_type,
                           opentimers_cbt     cb){
    uint8_t  i;
    uint8_t  idToSchedule;
    PORT_TIMER_WIDTH timerGap;
    PORT_TIMER_WIDTH tempTimerGap;

    INTERRUPT_DECLARATION();
    // 1. make sure the timer exist
    for (i=0;i<MAX_NUM_TIMERS;i++){
        if ((self->opentimers_vars).timersBuf[i].isUsed && i == id){
            break;
        }
    }
    if (i==MAX_NUM_TIMERS){
        // doesn't find the timer
        return;
    }

    DISABLE_INTERRUPTS();

    (self->opentimers_vars).timersBuf[id].timerType = timer_type;

    // 2. updat the timer content
    switch (uint_type){
    case TIME_MS:
        (self->opentimers_vars).timersBuf[id].duration = duration*PORT_TICS_PER_MS;
        (self->opentimers_vars).timersBuf[id].wraps_remaining  = (uint32_t)(duration*PORT_TICS_PER_MS)/MAX_TICKS_IN_SINGLE_CLOCK;
        break;
    case TIME_TICS:
        (self->opentimers_vars).timersBuf[id].duration = duration;
        (self->opentimers_vars).timersBuf[id].wraps_remaining  = (uint32_t)(duration)/MAX_TICKS_IN_SINGLE_CLOCK;
        break;
    }

    if ((self->opentimers_vars).timersBuf[id].wraps_remaining==0){
        (self->opentimers_vars).timersBuf[id].currentCompareValue = (self->opentimers_vars).timersBuf[id].duration+ sctimer_readCounter(self);
    } else {
        (self->opentimers_vars).timersBuf[id].currentCompareValue = MAX_TICKS_IN_SINGLE_CLOCK+ sctimer_readCounter(self);
    }

    (self->opentimers_vars).timersBuf[id].isrunning           = TRUE;
    (self->opentimers_vars).timersBuf[id].callback            = cb;

    // 3. find the next timer to fire

    // only execute update the currentCompareValue if I am not inside of ISR or the ISR itself will do this.
    if ((self->opentimers_vars).insideISR==FALSE){
        i = 0;
        while ((self->opentimers_vars).timersBuf[i].isrunning==FALSE){
            i++;
        }
        timerGap     = (self->opentimers_vars).timersBuf[i].currentCompareValue-(self->opentimers_vars).lastCompareValue;
        idToSchedule = i;
        for (i=idToSchedule+1;i<MAX_NUM_TIMERS;i++){
            if ((self->opentimers_vars).timersBuf[i].isrunning){
                tempTimerGap = (self->opentimers_vars).timersBuf[i].currentCompareValue-(self->opentimers_vars).lastCompareValue;
                if (tempTimerGap < timerGap){
                    // timer "i" is more close to lastCompare value
                    timerGap     = tempTimerGap;
                    idToSchedule = i;
                }
            }
        }

        // if I got here, assign the next to be fired timer to given timer
        (self->opentimers_vars).currentCompareValue = (self->opentimers_vars).timersBuf[idToSchedule].currentCompareValue;
 sctimer_setCompare(self, (self->opentimers_vars).currentCompareValue);
    }
    (self->opentimers_vars).running        = TRUE;

    ENABLE_INTERRUPTS();
}

/**
\brief schedule a period refer to given reference.

This function will schedule a timer which expires when the timer count reach
to duration + reference. This function will be used in the implementation of slot FSM.
All timers use this function are ONE_SHOT type timer.

\param[in] id indicates the timer id
\param[in] duration indicates the period asked for schedule after a given time indicated by reference parameter.
\param[in] reference indicates the reference for duration. The timer will be fired at reference+duration.
\param[in] uint_type indicates the unit type of this schedule: ticks or ms
\param[in] cb indicates when this scheduled timer fired, call this callback function.
 */
void opentimers_scheduleAbsolute(OpenMote* self, opentimers_id_t    id,
                                 uint32_t           duration,
                                 PORT_TIMER_WIDTH   reference ,
                                 time_type_t        uint_type,
                                 opentimers_cbt     cb){
    uint8_t  i;
    uint8_t idToSchedule;
    PORT_TIMER_WIDTH timerGap;
    PORT_TIMER_WIDTH tempTimerGap;

    INTERRUPT_DECLARATION();

    // 1. make sure the timer exist
    for (i=0;i<MAX_NUM_TIMERS;i++){
        if ((self->opentimers_vars).timersBuf[i].isUsed && i == id){
            break;
        }
    }
    if (i==MAX_NUM_TIMERS){
        // doesn't find the timer
        return;
    }

    DISABLE_INTERRUPTS();

    // absolute scheduling is for one shot timer
    (self->opentimers_vars).timersBuf[id].timerType = TIMER_ONESHOT;

    // 2. updat the timer content
    switch (uint_type){
    case TIME_MS:
        (self->opentimers_vars).timersBuf[id].duration = duration*PORT_TICS_PER_MS;
        (self->opentimers_vars).timersBuf[id].wraps_remaining  = (uint32_t)(duration*PORT_TICS_PER_MS)/MAX_TICKS_IN_SINGLE_CLOCK;;
        break;
    case TIME_TICS:
        (self->opentimers_vars).timersBuf[id].duration = duration;
        (self->opentimers_vars).timersBuf[id].wraps_remaining  = (uint32_t)duration/MAX_TICKS_IN_SINGLE_CLOCK;
        break;
    }

    if ((self->opentimers_vars).timersBuf[id].wraps_remaining==0){
        (self->opentimers_vars).timersBuf[id].currentCompareValue = (self->opentimers_vars).timersBuf[id].duration+reference;
    } else {
        (self->opentimers_vars).timersBuf[id].currentCompareValue = MAX_TICKS_IN_SINGLE_CLOCK+reference;
    }

    (self->opentimers_vars).timersBuf[id].isrunning = TRUE;
    (self->opentimers_vars).timersBuf[id].callback  = cb;

    // 3. find the next timer to fire

    // only execute update the currentCompareValue if I am not inside of ISR or the ISR itself will do this.
    if ((self->opentimers_vars).insideISR==FALSE){
        i = 0;
        while ((self->opentimers_vars).timersBuf[i].isrunning==FALSE){
            i++;
        }
        timerGap     = (self->opentimers_vars).timersBuf[i].currentCompareValue-(self->opentimers_vars).lastCompareValue;
        idToSchedule = i;
        for (i=idToSchedule+1;i<MAX_NUM_TIMERS;i++){
            if ((self->opentimers_vars).timersBuf[i].isrunning){
                tempTimerGap = (self->opentimers_vars).timersBuf[i].currentCompareValue-(self->opentimers_vars).lastCompareValue;
                if (tempTimerGap < timerGap){
                    // timer "i" is more close to lastCompare value
                    timerGap     = tempTimerGap;
                    idToSchedule = i;
                }
            }
        }

        // if I got here, assign the next to be fired timer to given timer
        (self->opentimers_vars).currentCompareValue = (self->opentimers_vars).timersBuf[idToSchedule].currentCompareValue;
 sctimer_setCompare(self, (self->opentimers_vars).currentCompareValue);
    }
    (self->opentimers_vars).running = TRUE;

    ENABLE_INTERRUPTS();
}

/**
\brief update the duration of timer.

This function should be called in the callback of the timer interrupt.

\param[in] id the timer id
\param[in] duration the timer duration
 */
void opentimers_updateDuration(OpenMote* self, opentimers_id_t id,
                                           PORT_TIMER_WIDTH duration){
    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();

    (self->opentimers_vars).timersBuf[id].duration = duration;

    ENABLE_INTERRUPTS();
}

/**
\brief cancel a running timer.

This function disable the timer temperally by removing its callback and marking
isrunning as false. The timer may be recover later.

\param[in] id the timer id
 */
void opentimers_cancel(OpenMote* self, opentimers_id_t id){

    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();

    (self->opentimers_vars).timersBuf[id].isrunning = FALSE;
    (self->opentimers_vars).timersBuf[id].callback  = NULL;

    ENABLE_INTERRUPTS();
}

/**
\brief destroy a stored timer.

Reset the whole entry of given timer including the id.

\param[in] id the timer id

\returns False if the given can't be found or return Success
 */
bool opentimers_destroy(OpenMote* self, opentimers_id_t id){
    if (id<MAX_NUM_TIMERS){
        memset(&(self->opentimers_vars).timersBuf[id],0,sizeof(opentimers_t));
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
\brief get the current counter value of sctimer.

\returns the current counter value.
 */
PORT_TIMER_WIDTH opentimers_getValue(OpenMote* self){
    return sctimer_readCounter(self);
}

/**
\brief get the currentCompareValue variable of opentimer2.

\returns currentCompareValue.
 */
PORT_TIMER_WIDTH opentimers_getCurrentCompareValue(OpenMote* self){
    return (self->opentimers_vars).currentCompareValue;
}

/**
\brief is the given timer running?

\returns isRunning variable.
 */
bool opentimers_isRunning(OpenMote* self, opentimers_id_t id){
    return (self->opentimers_vars).timersBuf[id].isrunning;
}
// ========================== task ============================================

// ========================== callback ========================================

/**
\brief this is the callback function of opentimer.

This function is called when sctimer interrupt happens. The function looks the
whole timer buffer and find out the correct timer responding to the interrupt
and call the callback recorded for that timer.
 */
void opentimers_timer_callback(OpenMote* self){
    uint8_t i;
    uint8_t idToSchedule;
    PORT_TIMER_WIDTH timerGap;
    PORT_TIMER_WIDTH tempTimerGap;

    if (
        (self->opentimers_vars).timersBuf[TIMER_INHIBIT].isrunning==TRUE &&
        (self->opentimers_vars).currentCompareValue == (self->opentimers_vars).timersBuf[TIMER_INHIBIT].currentCompareValue
    ){
        (self->opentimers_vars).timersBuf[TIMER_INHIBIT].isrunning  = FALSE;
        (self->opentimers_vars).timersBuf[TIMER_INHIBIT].callback(self, TIMER_INHIBIT);
        // the next timer selection will be done after SPLITE_TIMER_DURATION ticks
 sctimer_setCompare(self, sctimer_readCounter(self)+SPLITE_TIMER_DURATION);
        return;
    } else {
        if ((self->opentimers_vars).timersBuf[TIMER_INHIBIT].currentCompareValue == (self->opentimers_vars).currentCompareValue){
            // this is the timer interrupt right after inhibit timer, pre call the non-tsch, non-inhibit timer interrupt here to avoid interrupt during receiving serial bytes
            for (i=0;i<MAX_NUM_TIMERS;i++){
                if ((self->opentimers_vars).timersBuf[i].isrunning==TRUE){
                    if (i!=TIMER_TSCH && i!=TIMER_INHIBIT && (self->opentimers_vars).timersBuf[i].currentCompareValue - (self->opentimers_vars).currentCompareValue < PRE_CALL_TIMER_WINDOW){
                        (self->opentimers_vars).timersBuf[i].currentCompareValue = (self->opentimers_vars).currentCompareValue;
                    }
                }
            }
        }
        for (i=0;i<MAX_NUM_TIMERS;i++){
            if ((self->opentimers_vars).timersBuf[i].isrunning==TRUE){
                if ((self->opentimers_vars).currentCompareValue == (self->opentimers_vars).timersBuf[i].currentCompareValue){
                    // this timer expired, mark as expired
                    (self->opentimers_vars).timersBuf[i].lastCompareValue    = (self->opentimers_vars).timersBuf[i].currentCompareValue;
                    if (i==TIMER_TSCH){
                        (self->opentimers_vars).insideISR = TRUE;
                        (self->opentimers_vars).timersBuf[i].isrunning  = FALSE;
                        (self->opentimers_vars).timersBuf[i].callback(self, i);
                        (self->opentimers_vars).insideISR = FALSE;
                    } else {
                        if ((self->opentimers_vars).timersBuf[i].wraps_remaining==0){
                            (self->opentimers_vars).timersBuf[i].isrunning  = FALSE;
 scheduler_push_task(self, (task_cbt)((self->opentimers_vars).timersBuf[i].callback),(task_prio_t)(self->opentimers_vars).timersBuf[i].timer_task_prio);
                            if ((self->opentimers_vars).timersBuf[i].timerType==TIMER_PERIODIC){
                                (self->opentimers_vars).insideISR = TRUE;
 opentimers_scheduleIn(self, 
                                    i,
                                    (self->opentimers_vars).timersBuf[i].duration,
                                    TIME_TICS,
                                    TIMER_PERIODIC,
                                    (self->opentimers_vars).timersBuf[i].callback
                                );
                                (self->opentimers_vars).insideISR = FALSE;
                            }
                        } else {
                            (self->opentimers_vars).timersBuf[i].wraps_remaining--;
                            if ((self->opentimers_vars).timersBuf[i].wraps_remaining == 0){
                                (self->opentimers_vars).timersBuf[i].currentCompareValue = ((self->opentimers_vars).timersBuf[i].duration+(self->opentimers_vars).timersBuf[i].lastCompareValue) & MAX_TICKS_IN_SINGLE_CLOCK;
                                if ((self->opentimers_vars).timersBuf[i].currentCompareValue - (self->opentimers_vars).currentCompareValue < PRE_CALL_TIMER_WINDOW){
                                    // pre-call the timer here if it will be fired within PRE_CALL_TIMER_WINDOW, when wraps_remaining decrease to 0
                                    (self->opentimers_vars).timersBuf[i].isrunning  = FALSE;
 scheduler_push_task(self, (task_cbt)((self->opentimers_vars).timersBuf[i].callback),(task_prio_t)(self->opentimers_vars).timersBuf[i].timer_task_prio);
                                    if ((self->opentimers_vars).timersBuf[i].timerType==TIMER_PERIODIC){
                                        (self->opentimers_vars).insideISR = TRUE;
 opentimers_scheduleIn(self, 
                                            i,
                                            (self->opentimers_vars).timersBuf[i].duration,
                                            TIME_TICS,
                                            TIMER_PERIODIC,
                                            (self->opentimers_vars).timersBuf[i].callback
                                        );
                                        (self->opentimers_vars).insideISR = FALSE;
                                    }
                                }
                            } else {
                                (self->opentimers_vars).timersBuf[i].currentCompareValue = (self->opentimers_vars).timersBuf[i].lastCompareValue + MAX_TICKS_IN_SINGLE_CLOCK;
                            }
                        }
                    }
                }
            }
        }
    }
    (self->opentimers_vars).lastCompareValue = (self->opentimers_vars).currentCompareValue;

    // find the next timer to be fired
    i = 0;
    while ((self->opentimers_vars).timersBuf[i].isrunning==FALSE && i<MAX_NUM_TIMERS){
        i++;
    }
    if(i<MAX_NUM_TIMERS){
        timerGap     = (self->opentimers_vars).timersBuf[i].currentCompareValue-(self->opentimers_vars).lastCompareValue;
        idToSchedule = i;
        for (i=idToSchedule+1;i<MAX_NUM_TIMERS;i++){
            if ((self->opentimers_vars).timersBuf[i].isrunning){
                tempTimerGap = (self->opentimers_vars).timersBuf[i].currentCompareValue-(self->opentimers_vars).lastCompareValue;
                if (tempTimerGap < timerGap){
                    timerGap     = tempTimerGap;
                    idToSchedule = i;
                }
            }
        }

        // reschedule the timer
        (self->opentimers_vars).currentCompareValue = (self->opentimers_vars).timersBuf[idToSchedule].currentCompareValue;
 sctimer_setCompare(self, (self->opentimers_vars).currentCompareValue);
    } else {
        (self->opentimers_vars).running        = FALSE;
    }
}