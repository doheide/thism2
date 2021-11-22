//
// Created by dheide on 18.11.21.
//

#ifndef SM_SYS_H
#define SM_SYS_H

#include "thism2/sm.h"
#include "thism2/hwal_log_std.h"

// **********************************************************
// **********************************************************
struct HWAL_Std_No_HW : public HWAL_LoggerT<HWAL_Log_Std> {
    HWAL_Std_No_HW(HWAL_Log::LogLevel ll) : HWAL_LoggerT<HWAL_Log_Std>(ll) { }

    virtual void set_led(uint8_t v);

    void print_chip_info() {
        logger_get()->logf(HWAL_Log::Warning, "lala");
    }

    void reboot() {
        printf("Restarting now.\n");
        fflush(stdout);
        exit(0);
    }

    void sleep_ms(uint64_t msecs) {
        std::this_thread::sleep_for(std::chrono::milliseconds(msecs));
    }

    uint64_t get_time() {
        time_t now = std::time(0);
        return (uint64_t) now;
    }
};


// **********************************************************
// **********************************************************
// Define Events
MAKE_EVENT(E_LED_on, 0);
MAKE_EVENT(E_LED_off, 0);
MAKE_EVENT(E_On, 0);
MAKE_EVENT(E_Off, 0);
MAKE_EVENT(E_On_Blink, 0);

struct EventList; MAKE_EVENT_LIST(EventList, E_LED_on, E_LED_off, E_On, E_Off, E_On_Blink);
        //, E_Off_Blink);

// **********************************************************
// **********************************************************
struct S_LED_On; struct S_On;
struct S_Blink; struct S_Blink_Off; struct S_Blink_On;

struct S_LED_Off : public StateBase {
    StateSetup(S_LED_Off, "LED is off.") { }

    typedef StateDetails< void, TransitionListT<TransitionT<E_LED_on, S_LED_On>> > details;

    void onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering) final;
};

struct S_LED_On : public StateBase {
    StateSetup(S_LED_On, "LED is on.") { }

    typedef StateDetails< void, TransitionListT<TransitionT<E_LED_off, S_LED_Off>> > details;

    void onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering) final;
};


// **********************************************************
// **********************************************************
struct S_Off : public StateBase {
    StateSetup(S_Off, "Off state.") { }

    typedef StateDetails< void, TransitionListT<
            TransitionT<E_On, S_On>, TransitionT<E_On_Blink, S_Blink_On>
    > > details;

    void onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering) final;
};

struct S_On : public StateBase {
    StateSetup(S_On, "On state.") { }

    typedef StateDetails< void, TransitionListT<
        TransitionT<E_Off, S_Off>, // TransitionT<E_Off_Blink, S_Off>,
        TransitionT<E_On_Blink, S_Blink_Off>
    > > details;

    void onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering) final;
};

struct S_Blink : public StateBase {
StateSetup(S_Blink, "Blink off state.") { }

    typedef StateDetails< void, TransitionListT<
            //TransitionT<E_Off_Blink, S_Off>,
            TransitionT<E_Off, S_Off>,
            TransitionT<E_On, S_On>
            //TransitionT<E_Initial, S_Blink_On>
    > > details;
};

struct S_Blink_On : public StateBase {
StateSetup(S_Blink_On, "Blink off state.") { }

    typedef StateDetails<S_Blink, TransitionListT<
            TransitionT<E_Timer, S_Blink_Off>
    > > details;

    void onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering) final;
};

struct S_Blink_Off : public StateBase {
StateSetup(S_Blink_Off, "Blink off state.") { }

    typedef StateDetails<S_Blink, TransitionListT<
            TransitionT<E_Timer, S_Blink_On>
    > > details;

    void onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering) final;
};


//struct SM_State;
Make_StateMachine(SM_State, MarkInitialState<S_Off>, S_On, S_Blink, S_Blink_Off, S_Blink_On);
// struct SM_LED;
Make_StateMachine(SM_LED, MarkInitialState<S_LED_Off>, S_LED_On);

typedef SMTimer<Collector<S_Blink_On, S_Blink_Off>> SMT_Std;
typedef SMTimerListTmpl<EventList, SMT_Std> SMTimerList;

typedef SMSystem<EventList, Collector<SM_State, SM_LED>, SMTimerList> SMSys;

extern SMSys *smsys;

#endif //SM_SYS_H
 
