//
// MIT License
//
// Copyright (c) 2018 Dominik Heide
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#pragma once
//#ifndef SM_H
//#define SM_H

#include <type_traits>
#include <stdint.h>
#include <algorithm>
#include "hwal_x.h"

#ifndef LOG_INITIAL_EVENT
#define LOG_INITIAL_EVENT false
#endif

// *****************************************************************
/*
#ifndef BAHA_TYPE
#  error BAHA_TYPE not set.
#endif

#ifndef BAHA_INCLUDE
#  error BAHA_INCLUDE not set.
#endif

#define STRINGIFY(x) STRINGIFY_(x)
#define STRINGIFY_(x) #x
#include STRINGIFY(BAHA_INCLUDE)
*/

// *****************************************************************
//#define __make_treeuml

// *****************************************************************
// *****************************************************************
#define EVENT_BUFFER_SIZE_V 6
#define STATE_NESTING_LEVEL_MAX 30

//#define __useNames
//#define __noNames

//#define __useDescription
//#define __noDescription

#if defined(__useNames) && defined(__noNames)
#error useNames or nonNames options must not be defined at the same time.
#endif
#if !defined(__useNames) && !defined(__noNames)
#error Either __useNames or __noNames options have to be defined.
#endif

#if defined(__useDescription) && defined(__noDescription)
#error Description options must not be defined at the same time.
#endif
#ifndef __useDescription
#ifndef __noDescription
#define __noDescription
#endif
#endif



/*
// *****************************************************************
// *****************************************************************
#if !defined(__useNames) && !defined(__noNames)
static_assert(false, "SM: Either '__useNames' or '__noNames' have to be defined.");
#endif

#if defined(__useNames) && defined(__noNames)
static_assert(false, "SM: Either '__useNames' or '__noNames' have to be defined, not both!");
#endif

#if !defined(__useDescription) && !defined(__noDescription)
static_assert(false, "SM: Either '__noDescription' or '__noDescription' have to be defined.");
#endif

#if defined(__useDescription) && defined(__noDescription)
static_assert(false, "SM: Either '__noDescription' or '__noDescription' have to be defined, not both!");
#endif

#if !defined(__useNames) && defined(__useDescription)
static_assert(false, "SM: If '__useDescription' is active '__useNames' have also to be defined activated.");
#endif
*/




// ******************************************************************
// ******************************************************************
#include <thism2/sm_base.h>

// ******************************************************************
// ******************************************************************
#define V_UNUSED(x) (void)x;

// ******************************************************************
// ******************************************************************
class SystemBase;

struct EventIdT { uint16_t id; };
struct StateIdT { uint16_t id; };


/*class LogBase {
    BAHA_TYPE *baha;

public:
    LogBase(BAHA_TYPE *_baha) : baha(_baha) { }

    void log(const char *c) {
        baha->log(c);
    }
    void log(uint8_t n);
    void log(uint16_t n);
    void log(uint32_t n);
    void log(EventIdT id);
    void log(StateIdT id);

    void logTime();

    template<typename ...A>
    void logLine(A... a) {
        logTime();
        log(" | ");
        logLineImpl(a...);
        logLineEnd();
    }
    void logLineEnd() {
        baha->logLineEnd();
    }

protected:
    template<typename A, typename ...As>
    void logLineImpl(A ca, As... c) {
        log(ca);
        logLineImpl(c ...);
    }
    void logLineImpl() { }

    void logNumberImpl(uint32_t n, uint8_t digits);

};*/

// ******************************************************************
// ******************************************************************
namespace sys_detail {
    namespace helper {
        namespace id_helper {
            template <typename T> struct ClassNotInList { enum { value = false }; };
            struct ErrorOnVoid { enum { value = false }; };
            struct ReturnFFFFOnVoid { enum { value = true }; };

        };
        // @todo rename VoidBehaviour to NotFoundInListBehaviour
        template <typename ...>
        struct Id_Impl;
        template <typename CSTATE, uint16_t IDX, typename STATE, typename ... STATEs, typename VoidBehaviour>
        struct Id_Impl<CSTATE, std::integral_constant<uint16_t, IDX>, Collector<STATE, STATEs...>, VoidBehaviour> {
            typedef typename Id_Impl<CSTATE, std::integral_constant<uint16_t, IDX+1>, Collector<STATEs...>, VoidBehaviour>::type type;
        };
        template <typename CSTATE, uint16_t IDX, typename ... STATEs, typename VoidBehaviour>
        struct Id_Impl<CSTATE, std::integral_constant<uint16_t, IDX>, Collector<CSTATE, STATEs...>, VoidBehaviour> {
            typedef typename std::integral_constant<uint16_t, IDX> type;
        };
        template <typename CSTATE, uint16_t IDX, typename VoidBehaviour>
        struct Id_Impl<CSTATE, std::integral_constant<uint16_t, IDX>, Collector<>, VoidBehaviour> {
            static_assert(id_helper::ClassNotInList<CSTATE>::value, "Object not in list");
            typedef typename std::integral_constant<uint16_t, 0xFFFF> type;
        };
        template <uint16_t IDX, typename VoidBehaviour>
        struct Id_Impl<void, std::integral_constant<uint16_t, IDX>, Collector<>, VoidBehaviour> {
            static_assert(VoidBehaviour::value, "Event unknown");
            typedef typename std::integral_constant<uint16_t, 0xFFFF> type;
        };
    }
}

// ******************************************************************
struct EventPayloadBase {};

#define EOPT_ONLY_FROM_SELF 1
#define EOPT_ONLY_FROM_SELF_OR_PARENT 3
#define EOPT_ONLY_FROM_SELF_OR_PARENT_OR_CHILD 7
#define EOPT_IGNORE_IF_DEST_STATE_IS_ACTIVE 8

#ifdef __useNames
#define MAKE_EVENT_W_PAYLOAD(EVENTNAME, OPTS, PAYLOAD_TYPE) \
struct EVENTNAME {                                         \
    typedef PAYLOAD_TYPE payload_type; \
    typedef event_details::EventBase details; \
    static const char * name() { return #EVENTNAME; }\
    typedef std::integral_constant<uint8_t, OPTS> val_t; \
}
#define MAKE_EVENT(EVENTNAME, OPTS) \
MAKE_EVENT_W_PAYLOAD(EVENTNAME, OPTS, EventPayloadBase)

#else
#define MAKE_EVENT_W_PAYLOAD(EVENTNAME, OPTS, PAYLOAD_TYPE) \
struct EVENTNAME { \
    typedef PAYLOAD_TYPE payload_type; \
    typedef event_details::EventBase details; \
    std::integral_constant<uint8_t, OPTS> val_t; \
}
#define MAKE_EVENT(EVENTNAME, OPTS) \
MAKE_EVENT_W_PAYLOAD(EVENTNAME, OPTS, void)

#endif

namespace event_details {
    struct EventBase {};

    // *******************************************************
    template<typename X>
    struct EventConceptCheck {
        static_assert(detail::has_type_details<X>::value,
                      "CTC: Event struct has to be have a member ::details of the type EventBase.");
        static_assert(std::is_same<EventBase, typename X::details>::value,
                      "CTC: Event struct has to be have a member ::details of the type EventBase.");
#ifdef __useNames
//        static_assert(detail::has_static_name<X>::value,
//                      "CTC Event struct has to be have a member static method named ::name returning const char*.");
#endif
        static constexpr auto value = std::true_type();
    };
    template<typename X, typename ...A>
    struct EventEnsureEveryConcept {
        static_assert(EventConceptCheck<X>::value, "");
        static constexpr auto value = EventEnsureEveryConcept<A...>::value;
    };
    template<typename X>
    struct EventEnsureEveryConcept<X> {
        static constexpr auto  value = EventConceptCheck<X>::value;
    };

    // ***************************************************************
    template<typename ...>
    struct GetEventNameImpl;
    template<uint16_t CID, typename EVENT, typename ... EVENTS>
    struct GetEventNameImpl<std::integral_constant<uint16_t, CID>, Collector<EVENT, EVENTS...>> {
        const char* impl(uint16_t eid) {
            if(eid == CID)
                return EVENT::name();

            GetEventNameImpl<std::integral_constant<uint16_t, CID+1>, Collector<EVENTS...>> geni;
            return geni.impl(eid);
        }
    };
    template<uint16_t CID>
    struct GetEventNameImpl<std::integral_constant<uint16_t, CID>, Collector<>> {
        const char* impl(uint16_t ) {
            return "E_UNKNOWN";
        }
    };
    template<typename EVENTLIST>
    const char* getEventName(uint16_t eid) {
        GetEventNameImpl<std::integral_constant<uint16_t, 0>, typename EVENTLIST::AllEvents::type> geni;
        return geni.impl(eid);
    }
}

// ******************************************************************
// *****************************************************************
namespace event_details {
    struct EventListBase { };

    template <typename ...>
    struct EventListAll;

    template<typename ...A>
    struct EventListAll<Collector<A...>> {
        static_assert(event_details::EventEnsureEveryConcept<A...>::value, "lala");
        typedef Collector<A...> type;
        static const uint16_t size = sizeof...(A);

        template<typename CEV>
        using EventId = typename sys_detail::helper::Id_Impl<CEV, std::integral_constant<uint16_t, 0>, Collector<A...>,
                sys_detail::helper::id_helper::ErrorOnVoid>::type;

        uint8_t eventOpts[sizeof...(A)];

        EventListAll() : eventOpts{ A::val_t::value ... }
        { }
    };

/*    template<typename ...A>
    struct EventListAll {
        static_assert(event_details::EventEnsureEveryConcept<A...>::value, "lala");
        typedef Collector<A...> type;
        static const uint16_t size = sizeof...(A);

        template<typename CEV>
        using EventId = typename sys_detail::helper::Id_Impl<CEV, std::integral_constant<uint16_t, 0>, Collector<A...>,
                sys_detail::helper::id_helper::ErrorOnVoid>::type;

        uint8_t eventOpts[sizeof...(A)];

        EventListAll() : eventOpts{ A::val_t::value ... }
        { }
    };*/
}


struct EventBinaryPayload : EventPayloadBase {
    char *data;
    int16_t data_size;

    EventBinaryPayload(EventBinaryPayload&& arg) {
        data = arg.data;
        data_size = arg.data_size;
        arg.data = nullptr;
        arg.data_size = 0;
    }

    explicit EventBinaryPayload(const char *data_in, int16_t _data_size) : data(nullptr), data_size(_data_size) {
        if(data_in != nullptr)
            copy_data(data_in, data_size);
    }
    void copy_data(const char *data_in, int32_t _data_size) {
        delete_data();

        data_size = _data_size;

        data = new char[data_size];
        for(int i=0; i!=data_size; i++)
            data[i] = data_in[i];
    }
    virtual ~EventBinaryPayload() {
        delete_data();
    }
    void delete_data() {
        delete [] data;
        data = nullptr;
    }
};
struct EventStringPayload : EventBinaryPayload {
    EventStringPayload(EventStringPayload&& arg) : EventBinaryPayload(std::move(arg))
    { }

    explicit EventStringPayload(const char *str_in) : EventBinaryPayload(nullptr, 0) {
        int len;
        for(len=0; len < 300 && str_in[len] != 0; len++);

        len++;
        copy_data(str_in, len);

        data[len - 1] = 0;
    }
    virtual ~EventStringPayload() { }
};

struct E_FatalError; MAKE_EVENT_W_PAYLOAD(E_FatalError, 0, EventStringPayload);
struct E_Initial; MAKE_EVENT(E_Initial, EOPT_ONLY_FROM_SELF);
struct E_Timer; MAKE_EVENT(E_Timer, EOPT_ONLY_FROM_SELF);

#define ID_E_FatalError 0
#define ID_E_Initial 1
#define ID_E_Timer 2
template<typename ...A>
struct EventListAll : event_details::EventBase {
    typedef typename event_details::EventListAll<typename detail::JoinCollectors<
            Collector<E_FatalError, E_Initial, E_Timer>, A...>::type> AllEvents;
    typedef typename AllEvents::type type;

    template<typename CEV>
    using EventId = typename AllEvents::template EventId<CEV>;

    AllEvents events;
};
#define MAKE_EVENT_LIST(XXX, ...) struct XXX : EventListAll<__VA_ARGS__> {}

#define MAKE_SM_EVENT_LIST(XXX, ...) typedef Collector<__VA_ARGS__> XXX

// *****************************************************************
// *****************************************************************
struct StateBase;

namespace state_details {
    template<typename X>
    struct StateConceptCheck {
        static_assert(detail::has_type_details<X>::value,
                      "CTC: State struct has to have a member ::details specialized from StateDetails.");
        static_assert(detail::has_type_details_TransitionsT<X>::value,
                      "CTC: State struct has to have a member ::transitions of the type StateBase.");
        static_assert(std::is_base_of<StateBase, X>::value,
                      "CTC: State struct has to have a member ::details of the type StateBase.");

        static constexpr auto value = std::true_type();
    };
    template<>
    struct StateConceptCheck<void> {
        static constexpr auto value = std::true_type();
    };

    template<typename X, typename ...A>
    struct StateEnsureAllConcept {
        static_assert(StateConceptCheck<X>::value, "");
        static constexpr auto value = StateEnsureAllConcept<A...>::value;
    };
    template<typename X, typename ...A>
    struct StateEnsureAllConcept<Collector<X, A...>> {
        static constexpr auto value = StateEnsureAllConcept<X, A...>::value;
    };
    template<typename X>
    struct StateEnsureAllConcept<X> {
        static constexpr auto  value = StateConceptCheck<X>::value;
    };
}

struct TransitionBase { };

template<typename Event, typename State>
struct TransitionT : TransitionBase {
    typedef Event event;
    typedef State destState;
};
template<typename ...TransitionsT>
struct TransitionListT {
    typedef Collector<TransitionsT...> type;
};

struct TransitionImpl {
    uint16_t eventId;
    uint16_t stateId;
};

// todo check that ParentT is part of the state machine
template<typename ParentT_, bool EmitInitialEventOnEnter, typename TransitionsT_>
struct StateDetails  {
    typedef ParentT_ ParentT;
    typedef TransitionsT_ TransitionsT;
//    static bool emitInitialEventOnEnter() { return EmitInitialEventOnEnter; }
    typedef std::integral_constant<bool, EmitInitialEventOnEnter> emitInitialEventOnEnterT;
};

// *****
// TO BE REMOVED
//#include <iostream>

struct StateBase {
    HWAL *hwal;
    HWAL_Log::LogLevel llstate;
    bool emitInitialEventOnEnter;

#ifdef __useNames
    const char *name; // to be set by derived class
#endif
#ifdef __useDescription
    const char *description; // to be set by derived class
#endif

protected:
    // Disable copy and moveconstructors, states are not to be copied or moved
    StateBase(HWAL_Log::LogLevel _llstate)
            : hwal(nullptr), llstate(_llstate) { }
    StateBase(const StateBase &) : hwal(nullptr), llstate() { }
    StateBase(StateBase &&) noexcept : hwal(nullptr), llstate() { }
    StateBase(const StateBase &&) noexcept : hwal(nullptr), llstate(){ }
    StateBase& operator=(const StateBase&);      // Prevent assignment

    virtual ~StateBase() {}

public:
    virtual void internalTransition(uint16_t event, uint16_t sender, void *payload) { };
    //virtual void internalTransition_withPayload(uint16_t event, uint16_t sender, void *payload) { };
    virtual void onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering, void *payload) { };
    virtual void onExit(uint16_t event, void *payload) { }

    void logf(HWAL_Log::LogLevel ll, int8_t color, const char *format, ...)
#ifdef __GNUC__
    __attribute__ ((format (printf, 4, 5)))
#endif
    {
        va_list args;
        va_start(args, format);
#ifdef __useNames
        hwal->logger_get()->log_args(ll, color, format, args, name, llstate);
#else
        hwal->logger_get()->log_args(ll, color, format, args, "X", llstate);
#endif
    }
    void set_hwal(HWAL *_hwal) { this->hwal = _hwal; }
    HWAL_Log::LogLevel llstate_get() { return llstate; }

    //bool emitInitialEventOnEnter()
//    virtual bool emitInitialEventOnEnter() = 0;
};

/*
struct StateBase {
    HWAL *hwal;
    HWAL_Log::LogLevel llstate;
    bool emitInitialEventOnEnter_val;

#ifdef __useNames
    char *name;
protected:
    // Disable copy and moveconstructors, states are not to be copied or moved
    StateBase(const char *, const char *, HWAL_Log::LogLevel _llstate)
    : hwal(nullptr), llstate(_llstate) { }
    StateBase(const StateBase &) : hwal(nullptr), llstate() { }
    StateBase(StateBase &&) noexcept : hwal(nullptr), llstate() { }
    StateBase(const StateBase &&) noexcept : hwal(nullptr), llstate(){ }
    StateBase& operator=(const StateBase&);      // Prevent assignment

public:
    virtual void internalTransition(uint16_t event, uint16_t sender, void *payload) { };
    //virtual void internalTransition_withPayload(uint16_t event, uint16_t sender, void *payload) { };
    virtual void onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering, void *payload) { };
    virtual void onExit(uint16_t event, void *payload) { }
#ifdef __useNames
    virtual const char * name() = 0; //{ return ""; } // @todo make abstract
#endif
#ifdef __useDescription
    virtual const char * description() = 0; //{ return ""; } // @todo make abstract
#endif

    void logf(HWAL_Log::LogLevel ll, int8_t color, const char *format, ...) {
        va_list args;
        va_start(args, format);
        hwal->logger_get()->log_args(ll, color, format, args, name(), llstate);
    }
    void set_hwal(HWAL *_hwal) { this->hwal = _hwal; }
    HWAL_Log::LogLevel llstate_get() { return llstate; }

    bool emitInitialEventOnEnter() {
//    virtual bool emitInitialEventOnEnter() = 0;
    };

*/

#ifdef __noNames
#define StateSetup(STATECLASSNAME, DESCRIPTION) \
public: \
typedef STATECLASSNAME ThisState;               \
virtual bool emitInitialEventOnEnter() final { return details::emitInitialEventOnEnter(); } \
void dummy() { static_assert(std::is_same<std::remove_reference<decltype(this)>::type, STATECLASSNAME*>::value, \
    "CTC: state_setup(): First argument has to be the type of the parent class."); } \
using StateBase::internalTransition; \
STATECLASSNAME() : StateBase(#STATECLASSNAME, DESCRIPTION)
#endif


#if defined(__useNames) && !defined(__useDescription)
/*#define StateDefine(STATECLASSNAME, DESCRIPTION) \
struct STATECLASSNAME : public StateBase {       \
StateSetup(STATECLASSNAME, DESCRIPTION)*/


#define StateSetup(STATECLASSNAME, DESCRIPTION, ...) \
StateSetupWLL(STATECLASSNAME, DESCRIPTION, HWAL_Log::Always, ##__VA_ARGS__)

#define StateSetupWLL(STATECLASSNAME, STATEDESCRIPTION, LL, ...) \
public: \
typedef STATECLASSNAME ThisState; \
void dummy() { static_assert(std::is_same<std::remove_reference<decltype(this)>::type, STATECLASSNAME*>::value, \
    "CTC: state_setup(): First argument has to be the type of the parent class."); } \
using StateBase::internalTransition;                   \
constexpr static const char *C_NAME = #STATECLASSNAME;                                                       \
STATECLASSNAME() : StateBase(LL), ##__VA_ARGS__ {\
    this->name = C_NAME; \
    this->emitInitialEventOnEnter = details::emitInitialEventOnEnterT::value; \
    local_init(); \
}                                                                \
void local_init()
#endif


#if defined(__useNames) && defined(__useDescription)
#define StateDefine(STATECLASSNAME, DESCRIPTION) \
struct STATECLASSNAME : public StateBase {       \
StateSetup(STATECLASSNAME, DESCRIPTION)

#define StateSetup(STATECLASSNAME, DESCRIPTION, ...) \
StateSetupWLL(STATECLASSNAME, DESCRIPTION, HWAL_Log::Always, ##__VA_ARGS__)

#define StateSetupWLL(STATECLASSNAME, STATEDESCRIPTION, LL, ...) \
public: \
typedef STATECLASSNAME ThisState; \
void dummy() { static_assert(std::is_same<std::remove_reference<decltype(this)>::type, STATECLASSNAME*>::value, \
    "CTC: state_setup(): First argument has to be the type of the parent class."); } \
using StateBase::internalTransition;                   \
constexpr static const char *C_NAME = #STATECLASSNAME;                                                       \
constexpr static const char *C_DESCRIPTION = STATEDESCRIPTION;                                                       \
STATECLASSNAME() : StateBase(LL), ##__VA_ARGS__ {\
    this->name = C_NAME; \
    this->description = C_DESCRIPTION;                              \
    this->emitInitialEventOnEnter = details::emitInitialEventOnEnterT::value; \
    local_init(); \
} \
void local_init()
#endif




template<typename STATE>
struct MarkInitialState {
    typedef STATE type;
};

// *****************************************************************
// *****************************************************************

//namespace Simu_helper {
//#ifdef DO_SIMULATION
//    struct SimuCallbacks {
//        void onActivateState(uint16_t /*stateId*/) { }
//        void onDeactivateState(uint16_t /*stateId*/) { }
//        void onRaiseEvent(uint16_t /*eventId*/) { }
//    };
//#endif
//}

// **********************************************************************************
// **********************************************************************************
namespace sys_detail {
	struct EventBuffer {
		uint16_t event;
		uint16_t sender;
        EventPayloadBase *payload;
	};

    struct TransitionsForState {
        TransitionImpl *transitions;
        uint16_t transitionNum;
    };
}

#ifdef __make_treeuml
#include <QString>
#endif

class SystemBase {
#ifdef __make_treeuml
    template <typename SM, typename SYS> friend QString make_treeuml(SYS *sys);
#endif

public:
    static constexpr uint16_t ID_S_Undefined = 0xFFFF;
    static constexpr uint16_t ID_E_Undefined = 0xFFFF;
    static constexpr uint16_t ID_S_Initialization = 0xFFFE;

    uint32_t sysTime;
    uint16_t timerNum;
    uint32_t *timerCounter;

protected:
    uint16_t numberOfStates;
    uint16_t maxLevel;

    HWAL *hwal;

    sys_detail::EventBuffer eventBuffer[1<<EVENT_BUFFER_SIZE_V];
    uint8_t eventBufferWritePos, eventBufferReadPos;

    StateBase **statesBP;
    uint8_t *stateFlags;
    uint16_t *stateListTemp;
    uint16_t *stateParents;
    uint16_t *stateLevels;
    uint8_t *eventOpts;

    uint16_t *transitionsNumberPerState;
    TransitionImpl **transitions;

    uint32_t *timerCounterRepeat;
    uint16_t *timerOwner;
    uint16_t *timerInitiator;
    uint16_t *timerEvents;

public:
    SystemBase(HWAL *_hwal);
    virtual ~SystemBase() { }

    void processEvents();

//    virtual void logEventName(uint16_t) { } // @todo make abstract
//    virtual void logStateName(uint16_t) { } @todo make abstract

    HWAL *hwalBaseGet() {
        return hwal;
    }

    uint16_t numberOfStatesGet() { return numberOfStates; }
    uint16_t maxLevelGet() { return maxLevel; }
    uint8_t eventsInBuffer() { return eventBufferWritePos - eventBufferReadPos; }
    uint8_t eventBufferReadPosGet() { return eventBufferReadPos; }
    uint8_t eventBufferWritePosGet() { return eventBufferWritePos; }
    sys_detail::EventBuffer eventBufferGetElement(uint8_t idx) { return eventBuffer[idx]; }

    uint16_t getParentIdBI(uint16_t cstate);

    sys_detail::TransitionsForState transitionsForStateGetBI(uint16_t id) {
        sys_detail::TransitionsForState tfs;
        tfs.transitionNum = 0; tfs.transitions = 0;
        if(id < numberOfStates) {
            tfs.transitionNum = this->transitionsNumberPerState[id];
            tfs.transitions = this->transitions[id];
        }
        return tfs;
    }

    virtual const char *getEventName(uint16_t id) = 0;

    virtual bool getLogForStateInStateMachine(uint16_t stateId) = 0;

    const char *getStateName(uint16_t id) {
        if(id==SystemBase::ID_S_Undefined)
            return "S_Undefined";
        else if(id==SystemBase::ID_S_Initialization)
            return "Initialization";
        return statesBP[id]->name;
//        return statesBP[id]->name();
    }

    bool doLogTransitions, doLogRaiseEvent, doLogEnterState, doLogExitState, doLogEventFromBuffer;

//    virtual void logStateName(uint16_t id) {
//#ifdef __useNames
//        if(id==SystemBase::ID_S_Undefined)
//            baha->log("S_Undefined");
//        else
//            baha->log(statesBP[id]->name());
//#else
//        baha->log("S_");
//        baha->log(id);
//#endif
//    }

    void sysTickCallback();
    void sysTickProcess();

//protected:
//    virtual void sysTick_MutexLockOrWait() {}
//    virtual void sysTick_MutexUnLock() {}

public:
//#ifdef DO_SIMULATION
//    Simu_helper::SimuCallbacks *sicaba;
//    void setSimuCallbacks(Simu_helper::SimuCallbacks *_sicaba) {
//        this->sicaba = _sicaba;
//    }
//#endif

    uint32_t sysTimeGet() {
        return sysTime;
    }

    /// \brief Activate inital states
    void initialSetup();
    void clearEvents();

protected:
    /// Checks the protection for an event processing a transition from \p startStateId to \p destStateId.
    /// \param cevent information regarding the current event
    /// \param startStateId Id of the start state of the transition to be executed
    /// \param destStateId Id of the destination state of the transition to be executed
    /// \return false if the protection inhibits the execution of a transition
    virtual bool checkEventProtection(sys_detail::EventBuffer &cevent, uint16_t startStateId, uint16_t destStateId);

    bool isStateActiveBI(uint16_t cstate);
    void isStateActiveSetBI(uint16_t cstate, bool v);
    bool isStateBlockedBI(uint16_t cstate);
    void isStateBlockedSetBI(uint16_t cstate, bool v);

//    uint16_t getParentIdBI(uint16_t cstate);

    uint16_t getStateLevelBI(uint16_t cstate);

    virtual StateBase *getStateById(uint16_t /*id*/) { return 0; } // @todo make abstract?

    void raiseEventIdByIds(uint16_t eventId, uint16_t senderStateId, bool preventLog=false, EventPayloadBase *payload=0,
                           bool do_filter=false);

    void executeTransition(uint16_t startState, uint16_t destState, sys_detail::EventBuffer &cevent,
                           bool blockActivatedStates, bool doLog);
    void activateStateFullByIds(uint16_t curStateId, uint16_t destStateId, sys_detail::EventBuffer &cevent,
                                bool blockActivatedStates, bool doLog, bool initMode=false);

//    void executeTransition(uint16_t startState, uint16_t destState, uint16_t senderState, uint16_t event,
//                           bool blockActivatedStates, bool doLog);
//    void activateStateFullByIds(uint16_t curStateId, uint16_t destStateId, uint16_t senderStateId, uint16_t event,
//                                bool blockActivatedStates, bool doLog, bool initMode=false);

//    void deactivateStateFullById(uint16_t curStateId, uint16_t event, bool doLog);
    void deactivateStateFullById(uint16_t curStateId, sys_detail::EventBuffer &cevent, bool doLog);

    void activateStateAndParentsByIds(uint16_t destState, sys_detail::EventBuffer &cevent, bool doLog,
                                      bool blockActivatedStates=true, bool initMode=false);
//    void activateStateAndParentsByIds(uint16_t destState, uint16_t senderState, uint16_t event, bool doLog,
//                                      bool blockActivatedStates=true, bool initMode=false);

    virtual void raiseEvent_MutexLockOrWait() { hwal->raiseEvent_MutexLockOrWait(); }
    virtual void raiseEvent_MutexUnLock() { hwal->raiseEvent_MutexUnLock(); }

public:
    bool checkIfStateIsChildOfOrSame(uint16_t parentState, uint16_t childState);

    virtual bool doPrintEvent(uint16_t eventId) = 0;
};

// **********************************************************************************
namespace sys_detail {
    // *****
    template<typename STATE>
    struct StateRemoveInitialFlag {
        static_assert(state_details::StateConceptCheck<STATE>::value, "");
        typedef STATE type;
    };
    template<typename STATE>
    struct StateRemoveInitialFlag<MarkInitialState<STATE>> {
        typedef STATE type;
    };

    // *****
    template<typename ...>
    struct StatesGetInitial;
    template<typename STATE, typename ... STATEs>
    struct StatesGetInitial<Collector<MarkInitialState<STATE>, STATEs...>> {
        typedef STATE type;
    };
    template<typename STATE, typename ... STATEs>
    struct StatesGetInitial<Collector<STATE, STATEs...>> {
        typedef typename StatesGetInitial<Collector<STATEs...>>::type type;
    };
    template<>
    struct StatesGetInitial<Collector<>> {
        typedef void type;
    };

    // ***
    namespace helper {
        template<typename...>
        struct MaxT;
        template<uint16_t u, uint16_t v>
        struct MaxT<std::integral_constant<uint16_t, u>, std::integral_constant<uint16_t, v>> {
            typedef std::integral_constant<uint16_t, (u > v) ? u : v> type;
        };
    }
    template<typename ...>
    struct StateLevelImpl;
    template<typename STATE, uint16_t u>
    struct StateLevelImpl<STATE, std::integral_constant<uint16_t, u>> {
        typedef typename StateLevelImpl<typename STATE::details::ParentT, std::integral_constant<uint16_t, u+1>>::type type;
    };
    template<uint16_t u>
    struct StateLevelImpl<void, std::integral_constant<uint16_t, u>> {
        typedef std::integral_constant<uint16_t, u> type;
    };
    template<typename STATE>
    using StateLevel = StateLevelImpl<STATE, std::integral_constant<uint16_t, 0>>;

    template<typename ...> struct MaxStateLevel;
    template<typename STATE, typename ... STATEs>
    struct MaxStateLevel<STATE, STATEs...> {
        typedef typename helper::MaxT<typename StateLevel<STATE>::type, typename MaxStateLevel<STATEs...>::type>::type type;
    };
    template<> struct MaxStateLevel<> {
        typedef std::integral_constant<uint16_t, 0> type;
    };
    template<typename ...> struct MaxStateLevelC;
    template<typename ... STATEs>
    struct MaxStateLevelC<Collector<STATEs...>> {
        typedef typename MaxStateLevel<STATEs...>::type type;
    };

    // *****
    namespace helper {
        template<typename STATELIST, typename Transition> TransitionImpl makeTransition() {
            return TransitionImpl{ STATELIST::EventListT::template EventId<typename Transition::event>::value,
                        STATELIST::template StateId<typename Transition::destState>::value };
        }

        template<typename ...>
        struct TransitionList;
        template<typename STATELIST, typename ... TransitionsT>
        struct TransitionList<STATELIST, TransitionListT<TransitionsT...>> {
            typedef std::integral_constant<uint16_t, sizeof...(TransitionsT)> sizeT;
            TransitionImpl impl[sizeT::value];

            TransitionList() : impl { makeTransition<STATELIST, TransitionsT>()... } {}
        };

        template<typename STATELIST, typename STATE>
        struct StateListElement {
            STATE state;

            typedef TransitionList<STATELIST, typename STATE::details::TransitionsT> TransitionsT;
            TransitionsT transitions;
        };

        template<typename ...> struct StateListImpl;
        template<typename STATELIST, typename ... STATEs>
        struct StateListImpl<STATELIST, Collector<STATEs ...>> : StateListElement<STATELIST, STATEs> ... { };

//        template <typename ...>
//        struct StateId_Impl;
//        template <typename CSTATE, uint16_t IDX, typename STATE, typename ... STATEs>
//        struct StateId_Impl<CSTATE, std::integral_constant<uint16_t, IDX>, Collector<STATE, STATEs...>> {
//            typedef typename StateId_Impl<CSTATE, std::integral_constant<uint16_t, IDX+1>, Collector<STATEs...>>::type type;
//        };
//        template <typename CSTATE, uint16_t IDX, typename ... STATEs>
//        struct StateId_Impl<CSTATE, std::integral_constant<uint16_t, IDX>, Collector<CSTATE, STATEs...>> {
//            typedef typename std::integral_constant<uint16_t, IDX> type;
//        };
//        template <typename CSTATE, uint16_t IDX>
//        struct StateId_Impl<CSTATE, std::integral_constant<uint16_t, IDX>, Collector<>> {
//            typedef typename std::integral_constant<uint16_t, 0xFFFF> type;
//        };

        // *****
        struct StateList_Base { };
    }

    namespace helper {
        template<typename STATE> struct IsInitialState
        { typedef std::integral_constant<uint8_t, 0> type; };
        template<typename STATE> struct IsInitialState<MarkInitialState<STATE>>
        { typedef std::integral_constant<uint8_t, 1> type; };
    }

    template<typename ... >
    struct StateList;
    template<typename L_EventListT, typename ... STATEs, typename ... STATESWINITIALFLAG>
    struct StateList<L_EventListT, Collector<STATEs...>, Collector<STATESWINITIALFLAG...>> : helper::StateList_Base {
        typedef StateList<L_EventListT, Collector<STATEs...>, Collector<STATESWINITIALFLAG...>> ThisT;

        typedef L_EventListT EventListT;

        template<typename CSTATE>
        using StateId = typename helper::Id_Impl<CSTATE, std::integral_constant<uint16_t, 0>, Collector<STATEs...>,
                sys_detail::helper::id_helper::ReturnFFFFOnVoid>::type;

        typedef std::integral_constant<uint16_t, sizeof...(STATEs)> numberOfStatesT;

        helper::StateListImpl<ThisT, Collector<STATEs...>> states;

        StateBase *statesBP[numberOfStatesT::value];
        uint8_t stateFlags[numberOfStatesT::value];
        uint16_t stateParents[numberOfStatesT::value];

        uint16_t transitionsNumber[numberOfStatesT::value];
        TransitionImpl *transitions[numberOfStatesT::value];

        uint16_t stateLevels[numberOfStatesT::value];

        StateList() : states(), statesBP{ getBase<STATEs>() ... },
            stateFlags{ helper::IsInitialState<STATESWINITIALFLAG>::type::value ... },
            stateParents { getParent<STATEs>() ... },
            transitionsNumber { getTransitionsNumber<STATEs>() ... },
            transitions { getTransitions<STATEs>() ... },
            stateLevels { sys_detail::StateLevel<STATEs>::type::value ... } {
		}

        StateBase *getById(uint16_t id) {
            return statesBP[id];
        }
        template<typename STATE>
        STATE *get() {
            STATE &s = static_cast<helper::StateListElement<ThisT, STATE>*>(&states)->state;
            return &s;
        }
        template<typename STATE>
        STATE &getRef() {
            STATE &s = static_cast<helper::StateListElement<ThisT, STATE>*>(&states)->state;
            return s;
        }
        template<typename STATE>
        StateBase *getBase() {
            return get<STATE>();
        }
        template<typename STATE>
        uint16_t getParent() {
            return StateId<typename STATE::details::ParentT>::type::value;
        }

        template<typename STATE>
        TransitionImpl *getTransitions() {
            TransitionImpl *t = static_cast<helper::StateListElement<ThisT, STATE>*>(&states)->transitions.impl;
            return t;
        }
        template<typename STATE>
        uint16_t getTransitionsNumber() {
            return helper::StateListElement<ThisT, STATE>::TransitionsT::sizeT::value;
        }

    };

}

// **********************************************************************************************************************
// **********************************************************************************************************************

namespace statemachine_detail {
    template<typename ... > struct CheckTransitionsEvent;
    template<typename TRANSITION>
    struct CheckTransitionsEvent<TRANSITION, void> {
        static constexpr bool value = true;
    };
    template<typename TRANSITION, typename EVENTLIST>
    struct CheckTransitionsEvent<TRANSITION, EVENTLIST> {
//        static_assert(std::is_base_of<event_details::EventListBase, EVENTLIST>::value,
//                      "EventList is wrong type.");

        static constexpr bool value = detail::is_one_of_collection<
                typename TRANSITION::event, typename EVENTLIST::type>::value;
        static_assert(value, "CTC: Transition event is not an event of the parent-system.");
    };

    template<typename ... > struct CheckTransitionsDest;
    template<typename TRANSITION>
    struct CheckTransitionsDest<TRANSITION, void> {
        static constexpr bool value = true;
    };
    template<typename TRANSITION, typename ... STATEs>
    struct CheckTransitionsDest<TRANSITION, Collector<STATEs ...>> {
        static constexpr bool value = detail::is_one_of_collection<
                typename TRANSITION::destState, Collector<STATEs ...>>::value;
        static_assert(value, "CTC: Transition destination is not a state of the parent-statemachine");
    };

    template<typename ... > struct CheckTransitions;
    template<typename TRANSITION, typename ... TRANSITIONs, typename STATELIST, typename EVENTLIST>
    struct CheckTransitions<Collector<TRANSITION, TRANSITIONs ...>, STATELIST, EVENTLIST> {
        static_assert(CheckTransitionsDest<TRANSITION, STATELIST>::value, "" );
        static_assert(CheckTransitionsEvent<TRANSITION, EVENTLIST>::value, "" );

        static constexpr bool value = CheckTransitions<Collector<TRANSITIONs ...>,
            STATELIST, EVENTLIST>::value;
    };
    template<typename STATELIST, typename EVENTLIST>
    struct CheckTransitions<Collector<>, STATELIST, EVENTLIST> {
        static constexpr bool value = true;
    };

    template<typename ... > struct CheckStateTransitions_Impl;
    template<typename STATE, typename ... STATEs, typename STATESLIST, typename EVENTLIST>
    struct CheckStateTransitions_Impl<Collector<STATE, STATEs...>, STATESLIST, EVENTLIST> {
        static_assert(CheckTransitions<typename STATE::details::TransitionsT::type,
                      STATESLIST, EVENTLIST>::value, "");
        static constexpr bool value = CheckStateTransitions_Impl<Collector<STATEs...>,
            STATESLIST, EVENTLIST>::value;
    };
    template<typename STATELIST, typename EVENTLIST>
    struct CheckStateTransitions_Impl<Collector<>, STATELIST, EVENTLIST> {
        static constexpr bool value = true;
    };

    template<typename ... > struct CheckStateTransitions_State;
    template<typename ... STATES_ALL, typename EVENTLIST>
    struct CheckStateTransitions_State<Collector<STATES_ALL...>, EVENTLIST> {
        static_assert(state_details::StateEnsureAllConcept<Collector<STATES_ALL...>>::value, "Class has to be used with states.");
        static constexpr bool value = CheckStateTransitions_Impl<Collector<STATES_ALL...>,
            Collector<STATES_ALL...>, EVENTLIST>::value;
    };
    template<typename ... > struct CheckStateTransitions_Events;
    template<typename ... STATES_ALL, typename EVENTLIST>
    struct CheckStateTransitions_Events<Collector<STATES_ALL...>, EVENTLIST> {
        static_assert(state_details::StateEnsureAllConcept<Collector<STATES_ALL...>>::value, "Class has to be used with states.");
        static constexpr bool value = CheckStateTransitions_Impl<Collector<STATES_ALL...>,
            void, EVENTLIST>::value;
    };
}

#ifdef __noNames
#define Make_StateMachine(SMName, ...) \
struct SMName : StateMachine<__VA_ARGS__> { }
#endif
#ifdef __useNames
#define Make_StateMachine(SMName, _doLog, ...) \
    struct SMName : StateMachine<__VA_ARGS__> { \
        static const char *name() { return #SMName; } \
        SMName() { doLog = _doLog; } \
        bool doLog; \
    }
#endif

template< typename ... STATEs>
struct StateMachine {
    typedef Collector<STATEs...> StatesWithInitialFlagT;
    typedef Collector<typename sys_detail::StateRemoveInitialFlag<STATEs>::type...> StatesT;
    typedef typename sys_detail::StatesGetInitial<Collector<STATEs...>>::type InitialStateT;

    static_assert(state_details::StateEnsureAllConcept<StatesT>::value, "CTC: Not all given states have a correct type");
    static_assert(!std::is_void<InitialStateT>::value, "CTC: No initial state defined.");

    static_assert(detail::no_duplicates_in_collection<StatesT>::value, "CTC: No duplicates are allowed in state list of state machine.");
    static_assert(statemachine_detail::CheckStateTransitions_State<StatesT, void>::value, "");
};

namespace sm_helper {
    template<typename ...>
    struct StateMachineImpl;
    template<typename ALLSTATESLIST, typename ... SMs>
    struct StateMachineImpl<ALLSTATESLIST, Collector<SMs ...>> : SMs ... { };

    // ****
    template <typename ...> struct print_sms;
    template <typename SM, typename ...SMs, typename SMsType>
    struct print_sms<Collector<SM, SMs...>, SMsType> {
        static void print(SMsType *sms) {
            SM* sm = static_cast<SM*>(sms);
            printf("  > %s: doLog=%d\n", sm->name(), sm->doLog);
            print_sms<Collector<SMs...>, SMsType>::print(sms);
        }
    };
    template <typename SMsType>
    struct print_sms<Collector<>, SMsType> {
        static void print(SMsType *sms) {}
    };

    // ****
    template <typename ...> struct state_in_sms;
    template <typename CSTATE, typename STATE, typename ...STATEs, typename SM, typename ...SMs>
    struct state_in_sms<CSTATE, Collector<STATE, STATEs...>, Collector<SM, SMs...>> {
        typedef typename state_in_sms<CSTATE, Collector<STATEs...>, Collector<SM, SMs...>>::type type;
    };
    template <typename CSTATE, typename ...STATEs, typename SM, typename ...SMs>
    struct state_in_sms<CSTATE, Collector<CSTATE, STATEs...>, Collector<SM, SMs...>> {
        typedef SM type;
    };
    template <typename CSTATE, typename SM, typename CSM, typename ...SMs>
    struct state_in_sms<CSTATE, Collector<>, Collector<SM, CSM, SMs...>> {
        typedef typename state_in_sms<CSTATE, typename CSM::StatesT, Collector<CSM, SMs...>>::type type;
    };
    template <typename CSTATE> struct state_in_sms_not_found{ enum { value = false }; };
    template <typename CSTATE, typename SM>
    struct state_in_sms<CSTATE, Collector<>, Collector<SM>> {
        static_assert(state_in_sms_not_found<CSTATE>::value, "Event not found ");
        typedef SM type;
    };

    template <typename ...> struct state_in_sms_start;
    template <typename CSTATE, typename SM, typename ...SMs>
    struct state_in_sms_start<CSTATE, Collector<SM, SMs...>> {
        typedef typename state_in_sms<CSTATE, typename SM::StatesT, Collector<SM, SMs...>>::type type;
    };

//    template <typename ...> struct state_in_sms_get_log;
//    template <typename CSTATE, typename StatesCollector, typename SMCollector>
//    struct state_in_sms_get_log<CSTATE, StatesCollector, SMCollector> {
//        static bool get(sm_helper::StateMachineImpl<StatesCollector, SMCollector> *sms) {
//            typedef typename state_in_sms<CSTATE, StatesCollector>::type ltype;
//            return static_cast<ltype>(sms)->doLog;
//        }
//    };
    template <typename ...> struct state_in_sms_get_all;
    template <typename CSTATE, typename ...STATEs, typename StatesCollector, typename SMCollector>
    struct state_in_sms_get_all<Collector<CSTATE, STATEs...>, StatesCollector, SMCollector> {
        static bool get(uint16_t stateId, sm_helper::StateMachineImpl<StatesCollector, SMCollector> *sms) {
            typedef typename sys_detail::helper::Id_Impl<CSTATE, std::integral_constant<uint16_t, 0>, StatesCollector,
                    sys_detail::helper::id_helper::ReturnFFFFOnVoid>::type Id_local;
            if(stateId == Id_local::value) {
                typedef typename state_in_sms_start<CSTATE, SMCollector>::type CUR_SM;
                static_assert(detail::is_one_of_collection<CUR_SM, SMCollector>::value, "CTC getStateMachine<>(): StateMachine is not part of systems statemachine list.");
                CUR_SM* sm = static_cast<CUR_SM*>(sms);
                return sm->doLog;
            }
            return state_in_sms_get_all<Collector<STATEs...>, StatesCollector, SMCollector>::get(stateId, sms);
        }
    };
    template <typename StatesCollector, typename SMCollector>
    struct state_in_sms_get_all<Collector<>, StatesCollector, SMCollector> {
        static bool get(uint16_t stateId, sm_helper::StateMachineImpl<StatesCollector, SMCollector> *sms) {
            return true;
        }
    };

}
// *****
#define MAKE_TIMER(TNAME, EVENT, DEFAULTTIME, ...) \
struct TNAME : SMTimer<Collector<__VA_ARGS__>, EVENT, DEFAULTTIME> { \
    typedef Collector<__VA_ARGS__> TimerStates; typedef EVENT TimerEvent; \
    static constexpr uint32_t value = DEFAULTTIME; };

namespace sys_detail {
    struct SMTimer_Base { };
}

template<typename STATEList, typename EVENT=E_Timer, uint32_t DefaultTime=0>
struct SMTimer : sys_detail::SMTimer_Base {
    typedef STATEList TimerStates;
    typedef EVENT TimerEvent;
    static constexpr uint32_t value = DefaultTime;
};
template<typename EventListT, typename ... SMTIMERs>
struct SMTimerListTmpl {
    // @todo Check that Events are in EventListT
    typedef Collector<SMTIMERs ...> TimersT;

    template<typename TIMER>
    using TimerId = typename sys_detail::helper::Id_Impl<TIMER, std::integral_constant<uint16_t, 0>, Collector<SMTIMERs...>,
            sys_detail::helper::id_helper::ErrorOnVoid>::type;

    static constexpr uint16_t size = sizeof...(SMTIMERs);
    uint32_t timerCounter[size];
    uint32_t timerCounterRepeat[size];
    uint16_t timerOwner[size];
    uint16_t timerInitiator[size];
    uint16_t timerEvents[size];

    template<typename TIMER, typename STATE>
    static void timerCheckSender() {
        static_assert(detail::is_one_of_collection<TIMER, TimersT>::value, "Timer has to be in TimerList.");
        static_assert(detail::is_one_of_collection<STATE, typename TIMER::TimerStates>::value, "Timer is not allowed for state.");
    }

    SMTimerListTmpl() : timerCounter {}, timerEvents { EventListT::template EventId<typename SMTIMERs::TimerEvent>::value ... }
    { }
};

// *****
//template< typename ... >
//class System;

template<typename ...>
class SMSystem;

//template<typename LALA, typename ...SMs>
template<typename EVL, typename ...SMs, typename SMTimerList_, typename HWAL_T, typename ...EventsNotToPrint>
class SMSystem<EVL, Collector<SMs...>, SMTimerList_, HWAL_T, Collector<EventsNotToPrint...>> : public SystemBase {

public:
    typedef SMSystem<SMs...> This;

    typedef Collector<SMs...> SMsT;
    typedef EVL EventListT;

    typedef SMTimerList_ SMTimerListT;

    static constexpr uint16_t ID_S_Undefined = SystemBase::ID_S_Undefined;

    // ***
    //typedef Collector<typename SMs::States ...> StatesT;
    typedef typename detail::JoinCollectors<typename SMs::StatesT ...>::type StatesT;
    typedef typename detail::JoinCollectors<typename SMs::StatesWithInitialFlagT ...>::type StatesWithInitialFlagT;
    typedef sys_detail::StateList<EventListT, StatesT, StatesWithInitialFlagT> StateListT;

    static_assert(detail::no_duplicates_in_collection<StatesT>::value, "CTC: No duplicates are allowed in state list of all state machine in the system.");

    static_assert(statemachine_detail::CheckStateTransitions_Events<StatesT, EventListT>::value, "");

    template<typename CSTATE>
    using StateId = typename StateListT::template StateId<CSTATE>;
    template<typename CEVENT>
    using EventId = typename EventListT::template EventId<CEVENT>;
    template<typename CTIMER>
    using TimerId = typename SMTimerListT::template TimerId<CTIMER>;

    typedef std::integral_constant<uint16_t, StatesT::size> numberOfStatesT;

    typedef typename sys_detail::MaxStateLevelC<StatesT>::type MaxStateLevelC_T;

    virtual const char *getEventName(uint16_t id) {
        if(id==SystemBase::ID_E_Undefined)
            return "E_Undefined";
        return event_details::getEventName<EventListT>(id);
    }

    template<typename EVENT>
    uint8_t eventOptsGet() {
        return eventOpts[EventId<EVENT>()];
    }
    uint8_t eventOptsGetById(uint16_t event_id) {
        return eventOpts[event_id];
    }

protected:
    sm_helper::StateMachineImpl<StatesT, Collector<SMs...>> sms;
    sys_detail::StateList<EventListT, StatesT, StatesWithInitialFlagT> statesImpl;
    uint16_t _stateListTemp[ MaxStateLevelC_T::value ]{};

    typedef std::integral_constant<uint16_t, sizeof...(EventsNotToPrint)> numberOfEventsNotToPrint;
    uint16_t eventsNotToPrint[ numberOfEventsNotToPrint::value ]{};

    EventListT eventList;
    SMTimerListT smTimerList;

    HWAL_T *hwalt;

public:
    SMSystem(HWAL_T *_hwalt) :
        SystemBase(_hwalt), statesImpl(), eventsNotToPrint{EventId<EventsNotToPrint>() ...}, hwalt(_hwalt) {
        numberOfStates = numberOfStatesT::value;

        stateListTemp = _stateListTemp;

        statesBP = statesImpl.statesBP;
        stateFlags = statesImpl.stateFlags;
        stateParents = statesImpl.stateParents;
        stateLevels = statesImpl.stateLevels;
        eventOpts = eventList.events.eventOpts;

        maxLevel = MaxStateLevelC_T::value;

        transitionsNumberPerState = statesImpl.transitionsNumber;
        transitions = statesImpl.transitions;

        timerNum = SMTimerListT::size;
        timerCounter = smTimerList.timerCounter;
        timerCounterRepeat = smTimerList.timerCounterRepeat;
        timerEvents = smTimerList.timerEvents;
        timerInitiator = smTimerList.timerInitiator;
        timerOwner = smTimerList.timerOwner;

        //initialSetup();
    }
    virtual ~SMSystem() { }

    StateBase *getStateById(uint16_t id) { return statesImpl.getById(id); }
    template<typename STATE> STATE *getState() {
        static_assert(detail::is_one_of_collection<STATE, StatesT>::value, "CTC getState<>(): State is not part of systems state list.");
        return statesImpl.template get<STATE>();
    }
    template<typename STATE> STATE &getStateRef() {
        static_assert(detail::is_one_of_collection<STATE, StatesT>::value, "CTC getStateRef<>(): State is not part of systems state list.");
        return statesImpl.template getRef<STATE>();
    }
    template<typename STATE> StateBase *getStateBase() {
        static_assert(detail::is_one_of_collection<STATE, StatesT>::value, "CTC getStateBase<>(): State is not part of systems state list.");
        return statesImpl.template getBase<STATE>();
    }

    template<typename STATE> bool isStateActive() {
        static_assert(detail::is_one_of_collection<STATE, StatesT>::value, "CTC isStateActive<>(): State is not part of systems state list.");
        return isStateActiveBI(StateId<STATE>::value);
    }
    bool isStateActive(uint16_t state_id) {
        return isStateActiveBI(state_id);
    }
//    template<typename STATE> bool hasInitialTransition()
//    { return hasInitialTransitionBI(StateId<STATE>::value); }

    template<typename EVENT, typename STATE> void raiseEvent(typename EVENT::payload_type *payload=0) {
        static_assert(detail::is_one_of_collection<EVENT, typename EventListT::AllEvents::type>::value, "CTC raiseEvent<>(): Event is not part of systems event list.");
        static_assert(detail::is_one_of_collection<STATE, StatesT>::value, "CTC raiseEvent<>(): State is not part of systems state list.");
        raiseEventIdByIds(EventListT::template EventId<EVENT>::value, StateId<STATE>::value, false, payload);
    }
    template<typename EVENT> void raiseEvent_noSender(typename EVENT::payload_type *payload=0, bool preventLog=true) {
        static_assert(detail::is_one_of_collection<EVENT, typename EventListT::AllEvents::type>::value, "CTC raiseEvent<>(): Event is not part of systems event list.");
        raiseEventIdByIds(EventListT::template EventId<EVENT>::value, ID_S_Undefined, preventLog, payload);
    }
    template<typename EVENT> void raiseEvent(uint16_t sender, typename EVENT::payload_type *payload=0) {
        static_assert(detail::is_one_of_collection<EVENT, typename EventListT::AllEvents::type>::value, "CTC raiseEvent<>(): Event is not part of systems event list.");
        raiseEventIdByIds(EventListT::template EventId<EVENT>::value, sender, false, payload);
    }

    template<typename STATE> sys_detail::TransitionsForState *getStateTransitions() {
        static_assert(detail::is_one_of_collection<STATE, StatesT>::value, "CTC raiseEvent<>(): State is not part of systems state list.");
        return this->transitionsForStateGetBI(StateId<STATE>::value);
    }

    template<typename TIMER, typename OWNER, typename INITIATOR=OWNER>
    void startTimer(bool repeat = true, uint32_t time=TIMER::value) {
        SMTimerListT::template timerCheckSender<TIMER, INITIATOR>();

        uint16_t id = SMTimerListT::template TimerId<TIMER>::value;
        smTimerList.timerCounter[id] = time+1;
        if(repeat)
            smTimerList.timerCounterRepeat[id] = smTimerList.timerCounter[id];
        else
            smTimerList.timerCounterRepeat[id] = 0;
        smTimerList.timerOwner[id] = StateId<OWNER>::value;
        smTimerList.timerInitiator[id] = StateId<INITIATOR>::value;
    }
    template<typename TIMER>
    uint32_t getCurrentTimerCounter() {
        uint16_t id = SMTimerListT::template TimerId<TIMER>::value;
        return smTimerList.timerCounter[id];
    }
    template<typename TIMER>
    void disableTimer() {
        uint16_t id = SMTimerListT::template TimerId<TIMER>::value;
        smTimerList.timerCounter[id] = 0;
        smTimerList.timerCounterRepeat[id] = 0;
    }

    HWAL_T *hwaltGet() {
        return hwalt;
    }

    template<typename SM> SM *getStateMachine() {
        static_assert(detail::is_one_of_collection<SM, SMsT>::value, "CTC getStateMachine<>(): StateMachine is not part of systems statemachine list.");
        return static_cast<SM*>(&sms);
    }
    virtual bool getLogForStateInStateMachine(uint16_t stateId) final {
        return sm_helper::state_in_sms_get_all<StatesT, StatesT, SMsT>::get(stateId, &sms);
    }
    void print_sm_do_logs() {
        sm_helper::print_sms<SMsT, sm_helper::StateMachineImpl<StatesT, Collector<SMs...>>>::print(&sms);
    }

    bool doPrintEvent(uint16_t eventId) final {
        for(uint16_t i=0; i!=numberOfEventsNotToPrint::value; i++)
            if(eventsNotToPrint[i] == eventId)
                return false;
        return true;
    }

    template<typename EVENT>
    static bool isEvent(uint16_t event) {
        return event == EventId<EVENT>();
    }
};


//template<typename ...>
//class SMSystem;
//
//template<typename EVL, typename ...SMs, typename SMTimerList_, typename HWAL_T>
//class SMSystem<EVL, Collector<SMs...>, SMTimerList_, HWAL_T> : public SMSystem<EVL, Collector<SMs...>, SMTimerList_> {
//    typedef Collector<SMs...> SMsT;
//
//    HWAL_T *hwalt;
//
//public:
//    SMSystem(HWAL_T *_hwalt) : SMSystem<EVL, Collector<SMs...>, SMTimerList_>(_hwalt), hwalt(_hwalt) { }
//
//    virtual ~SMSystem() { }
//
//    HWAL_T *hwaltGet() {
//        return hwalt;
//    }
//};

//#endif // SM2_H


