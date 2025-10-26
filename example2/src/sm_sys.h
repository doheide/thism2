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
        logger_get()->logf(HWAL_Log::Warning, HWAL_Log::NoColor, "lala");
    }

    void reboot() {
        printf("Restarting now.\n");
        fflush(stdout);
        exit(0);
    }

    void sleep_ticks(uint64_t ticks) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ticks/10));
    }

    uint64_t get_time() {
        time_t now = std::time(0);
        return (uint64_t) now;
    }

    void sysTick_MutexLockOrWait() final {};
    void sysTick_MutexUnLock() final {};

    void raiseEvent_MutexLockOrWait() final {};
    void raiseEvent_MutexUnLock() final {}

    uint64_t get_time_since_start() override { return get_time(); }
};


// **********************************************************
// **********************************************************
// Define Events
MAKE_EVENT(E_Next, 0);

MAKE_SM_EVENT_LIST(SM_Events, E_Next);

MAKE_EVENT_LIST(EventList, SM_Events);

// **********************************************************
struct S_LogTest_L2;
struct S_LogTest_L3;
struct S_LogTest_L4;

struct S_LogTest_L1 : public StateBase {
    StateSetupWLL(S_LogTest_L1, "", HWAL_Log::Warning) { }

    typedef StateDetails< void, false, TransitionListT<TransitionT<E_Next, S_LogTest_L2>> > details;

    void onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering, void*) final;
};

struct S_LogTest_L2 : public StateBase {
    StateSetupWLL(S_LogTest_L2, "", HWAL_Log::Info) { }

    typedef StateDetails< void, false, TransitionListT<TransitionT<E_Next, S_LogTest_L3>> > details;

    void onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering, void*) final;
};

struct S_LogTest_L3 : public StateBase {
    StateSetupWLL(S_LogTest_L3, "", HWAL_Log::Details) { }

    typedef StateDetails< void, false, TransitionListT<TransitionT<E_Next, S_LogTest_L4>> > details;

    void onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering, void*) final;
};

struct S_LogTest_L4 : public StateBase {
    StateSetupWLL(S_LogTest_L4, "", HWAL_Log::Error) { }

    typedef StateDetails< void, false, TransitionListT<> > details;

    void onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering, void*) final;
};

// **********************************************************
Make_StateMachine(SM_State, true, MarkInitialState<S_LogTest_L1>, S_LogTest_L2, S_LogTest_L3, S_LogTest_L4);

typedef SMTimerListTmpl<EventList> SMTimerList;
typedef SMSystem<EventList, Collector<SM_State>, SMTimerList, HWAL_Std_No_HW, Collector<>> SMSys;

extern SMSys *smsys;

#endif //SM_SYS_H
 
