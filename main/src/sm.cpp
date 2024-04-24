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

#include <thism2/sm.h>


SystemBase::SystemBase(HWAL *_hwal)
    : hwal(_hwal), eventBuffer{}, doLogTransitions(true), doLogRaiseEvent(true),
        doLogEnterState(true), doLogExitState(true), doLogEventFromBuffer(true) {
    eventBufferReadPos = 0;
    eventBufferWritePos = 0;

    sysTime = 0;

//#ifdef DO_SIMULATION
//    sicaba = 0;
//#endif
}

#include <string.h>
void SystemBase::processEvents() {
    HWAL_Log *lol = this->hwal->logger_get();
    //lol->logf(HWAL_Log::Debug, HWAL_Log::Color::Orange, "processEvents() start: ");

    // @todo add special treatment for E_Initial -> only process for sending state
    //smSystem_helper::TransitionBufferSys tr;

//    for(uint16_t csi=0; csi!=numberOfStates; csi++)
//        lol->logf(HWAL_Log::Details, "pe: transitionsNumberPerState[%d] = %d",
//                  csi, transitionsNumberPerState[csi]);

    sysTickProcess();

    raiseEvent_MutexLockOrWait();
    uint8_t readUntil = eventBufferWritePos;
    raiseEvent_MutexUnLock();
    if(eventBufferReadPos == readUntil)
        return;

    // ***
/*    int log_num = 0;
    for(int ebrp = eventBufferReadPos;  ebrp!= readUntil; ebrp=(ebrp+1)&((1<<EVENT_BUFFER_SIZE_V)-1)) {
        sys_detail::EventBuffer &cevent = eventBuffer[ebrp];
        if(this->doLogEventFromBuffer && this->getLogForStateInStateMachine(cevent.sender) &&
            (strcmp(getEventName(cevent.event), "E_Timer")!=0)) log_num++;

        for(uint16_t csi=0; csi!=numberOfStates; csi++)
            if(isStateActiveBI(csi))
                for(uint16_t tii=0; tii!=transitionsNumberPerState[csi]; tii++) {
                    TransitionImpl *tics = &(transitions[csi][tii]);
                    if(cevent.event == tics->eventId && this->getLogForStateInStateMachine(csi))
                        log_num++;
                }
    }
    if(log_num>0)
        lol->logf(HWAL_Log::Debug,HWAL_Log::Color::Pink, "processEvents() - new events s/e: %d/%d.", eventBufferReadPos, readUntil);
*/
    auto eventBufferReadPos_buffer = eventBufferReadPos;
    for(; eventBufferReadPos != readUntil; eventBufferReadPos=(eventBufferReadPos+1)&((1<<EVENT_BUFFER_SIZE_V)-1)) {
        //for(uint8_t i=eventBufferReadPos; i != readUntil; i=(i+1)&((1<<EVENT_BUFFER_SIZE_V)-1)) {
        sys_detail::EventBuffer &cevent = eventBuffer[eventBufferReadPos];

/*        if(this->doLogEventFromBuffer) { // && (strcmp(getEventName(cevent.event), "E_Timer")!=0)) {
            if (this->getLogForStateInStateMachine(cevent.sender))
                lol->logf(HWAL_Log::Debug, HWAL_Log::Pink, "!! EBuffer %s (%d) | %s (%d) - rp: %d/%d\n",
                          getEventName(cevent.event), cevent.event, getStateName(cevent.sender), cevent.sender,
                          eventBufferReadPos, readUntil);
        }*/

        for (uint16_t level = maxLevel; level != 0; level--)
            for (uint16_t csi = 0; csi != numberOfStates; csi++)
                if (isStateActiveBI(csi) && (stateLevels[csi] == level)) {
                    if (isStateActiveBI(csi) && !isStateBlockedBI(csi)) {
                        if (checkEventProtection(cevent, csi, ID_S_Undefined)) {
                            StateBase *sb = getStateById(csi);
                            sb->internalTransition(cevent.event, cevent.sender, cevent.payload);
                            //sb->internalTransition_withPayload(cevent.event, cevent.sender, cevent.payload);
                        }
                    }
                }
    }

    for(eventBufferReadPos = eventBufferReadPos_buffer; eventBufferReadPos != readUntil; eventBufferReadPos=(eventBufferReadPos+1)&((1<<EVENT_BUFFER_SIZE_V)-1)) {
        sys_detail::EventBuffer &cevent = eventBuffer[eventBufferReadPos];

        for (uint16_t level = maxLevel; level != 0; level--)
            for (uint16_t csi = 0; csi != numberOfStates; csi++)
                if (isStateActiveBI(csi) && (stateLevels[csi] == level)) {
                    //BAHABase->logLine("active + level: ", (uint16_t) level, " ", StateIdT{csi}, " ", (uint8_t)csi);

                    for (uint16_t tii = 0; tii != transitionsNumberPerState[csi]; tii++) {
//                        lol->logf(HWAL_Log::Details, "pe: checking trns[%d][%d]", csi, tii);
//                        lol->logf(HWAL_Log::Details, "pe: transitionsNumberPerState[%d] = %d",
//                                  csi, transitionsNumberPerState[csi]);

                        TransitionImpl *tics = &(transitions[csi][tii]);

                        bool doLogT = this->getLogForStateInStateMachine(csi);
                        if (cevent.event == tics->eventId) {
                            uint16_t &cs = tics->stateId;
                            if (checkEventProtection(cevent, csi, cs)) {

                                // @todo Describe arguments: What does block activated States mean?
                                executeTransition(csi, cs, cevent, true, doLogT);
                                //executeTransition(csi, cs, cevent.sender, cevent.event, true, doLogT);
                            } else {
                                if (doLogT)
                                    lol->logf(HWAL_Log::Debug, HWAL_Log::Color::Pink,
                                              "Transition %s (%d) -> %s (%d) (because of %s) not executed because of protection.",
                                              getStateName(csi), csi, getStateName(tics->stateId),
                                              tics->stateId, getEventName(cevent.event));
                            }
                        }
                    }
                }

        if(cevent.payload != 0) {
            delete cevent.payload;
            cevent.payload = 0;
        }
    }
}

bool SystemBase::checkEventProtection(sys_detail::EventBuffer &cevent, uint16_t startStateId, uint16_t destStateId) {
    if(eventOpts[cevent.event]==0)
        return true;

    if(eventOpts[cevent.event] & EOPT_ONLY_FROM_SELF_OR_PARENT_OR_CHILD) {
        bool or_result = false;
        if ((eventOpts[cevent.event] & EOPT_ONLY_FROM_SELF) && (cevent.sender == startStateId))
            or_result = true;
        if (eventOpts[cevent.event] & EOPT_ONLY_FROM_SELF_OR_PARENT) {
            uint16_t cn = startStateId;
            while ((cn = getParentIdBI(cn)) != ID_S_Undefined)
                if (cevent.sender == cn)
                    or_result = true;
        }
        if (eventOpts[cevent.event] & EOPT_ONLY_FROM_SELF_OR_PARENT_OR_CHILD) {
            uint16_t cn = cevent.sender;
            while ((cn = getParentIdBI(cn)) != ID_S_Undefined)
                if (cevent.sender == startStateId)
                    or_result = true;
        }
        if (!or_result)
            return false;
    }
    if(eventOpts[cevent.event] & EOPT_IGNORE_IF_DEST_STATE_IS_ACTIVE) {
//        if(destStateId==ID_S_Undefined) what is this???
//            return true;
        if(isStateActiveBI(destStateId))
            return false;
    }
    return true;
}

bool SystemBase::isStateActiveBI(uint16_t cstate) {
    if(cstate>=SystemBase::ID_S_Undefined) return false;
    return (stateFlags[cstate]&1)==1;
}
void SystemBase::isStateActiveSetBI(uint16_t cstate, bool v) {
    if(cstate>=SystemBase::ID_S_Undefined) return; // @todo raise fatal error
    if(v) stateFlags[cstate] |= 1;
    else  stateFlags[cstate] &= ~1;
}
bool SystemBase::isStateBlockedBI(uint16_t cstate) {
    if(cstate>=SystemBase::ID_S_Undefined) return false;
    return (stateFlags[cstate]&3)==2;
}
void SystemBase::isStateBlockedSetBI(uint16_t cstate, bool v) {
    if(cstate>=SystemBase::ID_S_Undefined) return; // @todo raise fatal error
    if(v) stateFlags[cstate] |= 2;
    else  stateFlags[cstate] &= ~2;
}

uint16_t SystemBase::getParentIdBI(uint16_t cstate) {
    if(cstate >= numberOfStates) return ID_S_Undefined;
    return stateParents[cstate];
}


/**
 * @brief Callback function for the system tick.
 *
 * This function is called by the system tick timer interrupt. It increments the system time (sysTime) by 1 and
 * decrements the timerCounter values for each active timer.
 */
void SystemBase::sysTickCallback() {
    sysTime++;
    for(uint16_t i=0; i!=timerNum; i++) {
        if(timerCounter[i]>1)
            timerCounter[i]--;
    }
}

/**
 * @brief Process the sysTick interrupt and handle timer events.
 *
 * This function is called in the sysTick interrupt handler. It checks if any timer has reached its
 * counting value and raises the corresponding timer event. Then, it resets the timer counter to its
 * repeat value.
 */
void SystemBase::sysTickProcess() {
    for(uint16_t i=0; i!=timerNum; i++) {
        if(timerCounter[i]==1) {
            raiseEventIdByIds(timerEvents[i], timerInitiator[i], true, 0, false);
            timerCounter[i] = timerCounterRepeat[i];
        }
    }
}

void SystemBase::raiseEventIdByIds(uint16_t eventId, uint16_t senderStateId, bool preventLog, EventPayloadBase *payload,
                                   bool do_filter) {
    raiseEvent_MutexLockOrWait();
    if(do_filter) {
        int c_read_pos = eventBufferReadPos;
        while(c_read_pos != eventBufferWritePos) {
            auto &ce = eventBuffer[c_read_pos];
            if(ce.event==eventId && ce.sender == senderStateId && ce.payload == payload) {
                raiseEvent_MutexUnLock();
                return;
            }
            c_read_pos++; c_read_pos &= (1<<EVENT_BUFFER_SIZE_V) - 1;
        }
    }

    eventBuffer[eventBufferWritePos++] = {eventId, senderStateId, payload };
    eventBufferWritePos &= (1<<EVENT_BUFFER_SIZE_V) - 1;
    if(eventBufferWritePos==eventBufferReadPos) {
        eventBufferReadPos = 0;
        eventBufferWritePos = 0;
        eventBuffer[eventBufferWritePos++] = {ID_E_FatalError, ID_S_Undefined, new E_FatalError::payload_type("EventRingBuffer full") };
    }
    raiseEvent_MutexUnLock();

    if(!preventLog) {
        HWAL_Log *lol = this->hwal->logger_get();
        if(this->doLogRaiseEvent) {
            if(this->getLogForStateInStateMachine(senderStateId) && this->doPrintEvent(eventId))
                lol->logf(HWAL_Log::Info, HWAL_Log::IGreen, "!! R %s | %s (rp / wp: %d / %d)",
                          getEventName(eventId), getStateName(senderStateId), eventBufferReadPos, eventBufferWritePos);
        }
    }
}
void SystemBase::clearEvents() {
    HWAL_Log *lol = this->hwal->logger_get();
    lol->logf(HWAL_Log::Error, HWAL_Log::IGreen, "Clear events called - but disabled");

//    HWAL_Log *lol = this->hwal->logger_get();
//    lol->logf(HWAL_Log::Warning, HWAL_Log::IGreen, "Clear events called");
//    eventBufferWritePos = 0; eventBufferReadPos = 0;
}


void SystemBase::executeTransition(uint16_t startState, uint16_t destState, sys_detail::EventBuffer &cevent,
                                   bool blockActivatedStates, bool doLog) {
    HWAL_Log *lol = this->hwal->logger_get();

    uint16_t senderState = cevent.sender;
    uint16_t event = cevent.event;

    if(!isStateActiveBI(startState)) {
        // @todo Add error message
        raiseEventIdByIds(ID_E_FatalError, senderState, false);
    }

    if(this->doLogTransitions && doLog)
        lol->logfll(HWAL_Log::Info, getStateById(destState)->llstate_get(), HWAL_Log::IGreen, "!! T %s: %s -> %s | %s",
                    getEventName(event), getStateName(startState), getStateName(destState), getStateName(senderState));

    uint16_t temp, i;
    uint16_t commonState;
    uint16_t lastActiveChild;

    // find common state
    commonState = destState;
    while( ((temp=getParentIdBI(commonState))!=SystemBase::ID_S_Undefined) && (!isStateActiveBI(commonState)) )
        commonState = temp;

    // find last active child state
    lastActiveChild = startState;
    do {
        for(i=0; i!=numberOfStates; i++) {
            if( (getParentIdBI(i) == lastActiveChild) && isStateActiveBI(i) ) {
                lastActiveChild = i;
                break;
            }
        }
    } while(i!=numberOfStates);

    // make list of states to disable
    while ((lastActiveChild != ID_S_Undefined) && (lastActiveChild != commonState)) {
        deactivateStateFullById(lastActiveChild, cevent, doLog);
        lastActiveChild = getParentIdBI(lastActiveChild);
    }
    // activate from common state to destState
    activateStateAndParentsByIds(destState, cevent, doLog, blockActivatedStates);
    //activateStateAndParentsByIds(destState, senderState, event, doLog, blockActivatedStates);
}

void SystemBase::activateStateFullByIds(uint16_t curStateId, uint16_t destStateId, sys_detail::EventBuffer &cevent,
                                        bool blockActivatedStates, bool doLog, bool initMode) {
    HWAL_Log *lol = this->hwal->logger_get();

    uint16_t senderStateId = cevent.sender;
    //uint16_t event = cevent.event;

    bool isCStateActive = isStateActiveBI(curStateId);
    if(!isCStateActive || initMode || (isCStateActive && curStateId == destStateId)) {
        if(isCStateActive && !initMode)
            //getStateById(curStateId)->onExit(isCStateActive);
            deactivateStateFullById(curStateId, cevent, doLog);

        if(this->doLogEnterState && doLog)
            lol->logfll(HWAL_Log::Info, getStateById(curStateId)->llstate_get(), HWAL_Log::DGreen,
                        "!! EN %s (dest: %s) | %s", getStateName(curStateId), getStateName(destStateId),
                        getStateName(senderStateId));

        isStateActiveSetBI(curStateId, true);
        isStateBlockedSetBI(curStateId, blockActivatedStates);

        bool isDestState = curStateId == destStateId;

        getStateById(curStateId)->onEnter(senderStateId, cevent.event, isDestState, isCStateActive, cevent.payload);

        if(isDestState) {
            bool hasInitialEventTransition = false;
            bool forceRaiseInitial = getStateById(curStateId)->emitInitialEventOnEnter;
            if(!forceRaiseInitial)
                for(uint16_t tii=0; tii!=transitionsNumberPerState[curStateId]; tii++) {
                    TransitionImpl *tics = &(transitions[curStateId][tii]);
                    if(tics->eventId == ID_E_Initial) {
                        hasInitialEventTransition = true;
                        break;
                    }
                }
            if(hasInitialEventTransition || forceRaiseInitial)
                raiseEventIdByIds(ID_E_Initial, curStateId, !LOG_INITIAL_EVENT);
        }
    }
}

//void SystemBase::deactivateStateFullById(uint16_t curStateId, uint16_t event, bool doLog) {
void SystemBase::deactivateStateFullById(uint16_t curStateId, sys_detail::EventBuffer &cevent, bool doLog) {
    HWAL_Log *lol = this->hwal->logger_get();

    if(this->doLogExitState && doLog)
        lol->logfll(HWAL_Log::Info, getStateById(curStateId)->llstate_get(), HWAL_Log::DGreen, "!! EX %s",
                    getStateName(curStateId));

    getStateById(curStateId)->onExit(cevent.event, cevent.payload);
    isStateActiveSetBI(curStateId, false);

    for(uint16_t i=0; i!=timerNum; i++) {
        if(timerOwner[i] == curStateId)
            timerCounter[i] = 0;
    }
}

void SystemBase::activateStateAndParentsByIds(uint16_t destState, sys_detail::EventBuffer &cevent, bool doLog,
                                              bool blockActivatedStates, bool initMode) {
    int counter = 0;
    uint16_t cstate = destState;

//    uint16_t senderState = cevent.sender;
//    uint16_t event = cevent.event;

    // make list of all parent state
    do {
        stateListTemp[counter++] = cstate;
        cstate = getParentIdBI(cstate);
    } while((cstate!=SystemBase::ID_S_Undefined) && !isStateActiveBI(cstate));

    // activate all states top down
    while(counter>0) {
        counter--;
        activateStateFullByIds(stateListTemp[counter], destState, cevent,
                               blockActivatedStates, doLog, initMode);
//        activateStateFullByIds(stateListTemp[counter], destState, senderState,
//                               event, blockActivatedStates, doLog, initMode);
    }
}

void SystemBase::initialSetup() {
    for(uint16_t i=0; i!=numberOfStates; i++) {
        this->getStateById(i)->set_hwal(this->hwal);
        if (isStateActiveBI(i)) {
            bool doLog = this->getLogForStateInStateMachine(i);
            sys_detail::EventBuffer cevent = {
                    .event = SystemBase::ID_E_Undefined, .sender = SystemBase::ID_S_Initialization, .payload=0};
            activateStateAndParentsByIds(i, cevent, doLog, false, true);
        }
    }
}

bool SystemBase::checkIfStateIsChildOfOrSame(uint16_t parentState, uint16_t childState)
{
    if((parentState >= this->numberOfStates) || (childState >= this->numberOfStates))
        return false;

    uint16_t cState = childState;
    while (cState != SystemBase::ID_S_Undefined) {
        if(cState == parentState)
            return true;
        cState = stateParents[cState];
    }
    return false;
}


