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
    : hwal(_hwal), eventBuffer{}, doLogTransitions(true), doLogRaiseEvent(false),
        doLogEnterState(true), doLogExitState(true), doLogEventFromBuffer(false) {
    eventBufferReadPos = 0;
    eventBufferWritePos = 0;

    sysTime = 0;

//#ifdef DO_SIMULATION
//    sicaba = 0;
//#endif
}

void SystemBase::processEvents() {
    HWAL_Log *lol = this->hwal->logger_get();
    // lol->logf(HWAL_Log::Debug, HWAL_Log::NoColor, "processEvents(): ");

    // @todo add special treatment for E_Initial -> only process for sending state
    //smSystem_helper::TransitionBufferSys tr;

//    for(uint16_t csi=0; csi!=numberOfStates; csi++)
//        lol->logf(HWAL_Log::Details, "pe: transitionsNumberPerState[%d] = %d",
//                  csi, transitionsNumberPerState[csi]);

    raiseEvent_MutexLockOrWait();
    uint8_t readUntil = eventBufferWritePos;
    raiseEvent_MutexUnLock();
    if(eventBufferReadPos == readUntil)
        return;
    //lol->logf(HWAL_Log::Debug, -1, "processEvents() - new events.");

    for(; eventBufferReadPos != readUntil; eventBufferReadPos=(eventBufferReadPos+1)&((1<<EVENT_BUFFER_SIZE_V)-1)) {
    //for(uint8_t i=eventBufferReadPos; i != readUntil; i=(i+1)&((1<<EVENT_BUFFER_SIZE_V)-1)) {
        sys_detail::EventBuffer &cevent = eventBuffer[eventBufferReadPos];

        if(this->doLogEventFromBuffer)
            lol->logf(HWAL_Log::Debug, HWAL_Log::DGreen, "!! EBuffer %s | %s\n",
                   getEventName(cevent.event), getStateName(cevent.sender));

        for(uint16_t level = maxLevel; level!=0; level--)
            for(uint16_t csi=0; csi!=numberOfStates; csi++)
                if(isStateActiveBI(csi) && (stateLevels[csi]==level)) {
                    //BAHABase->logLine("active + level: ", (uint16_t) level, " ", StateIdT{csi}, " ", (uint8_t)csi);

                    for(uint16_t tii=0; tii!=transitionsNumberPerState[csi]; tii++) {
//                        lol->logf(HWAL_Log::Details, "pe: checking trns[%d][%d]", csi, tii);
//                        lol->logf(HWAL_Log::Details, "pe: transitionsNumberPerState[%d] = %d",
//                                  csi, transitionsNumberPerState[csi]);

                        TransitionImpl *tics = &(transitions[csi][tii]);

                        if(cevent.event == tics->eventId) {
                            if(checkEventProtection(cevent, csi)) {
                                uint16_t &cs = tics->stateId;
                                executeTransition(csi, cs, cevent.sender, cevent.event, true);
                            }
                        }
                    }

                    if(isStateActiveBI(csi) && !isStateBlockedBI(csi)) {
                        if(checkEventProtection(cevent, csi)) {
                            StateBase *sb = getStateById(csi);
                            sb->internalTransition(cevent.event, cevent.sender);
                            sb->internalTransition_withPayload(cevent.event, cevent.sender, cevent.payload);
                        }
                    }
        }
        if(cevent.payload!=0)
            delete cevent.payload;
    }
}

bool SystemBase::checkEventProtection(sys_detail::EventBuffer &cevent, uint16_t cStateId) {
    if(eventOpts[cevent.event]==0)
        return true;
    if((eventOpts[cevent.event] & EOPT_ONLY_FROM_SELF) && (cevent.sender==cStateId))
        return true;
    if(eventOpts[cevent.event] & EOPT_ONLY_FROM_SELF_OR_PARENT) {
        uint16_t cn = cStateId;
        while((cn=getParentIdBI(cn)) != ID_S_Undefined)
            if(cevent.sender==cn)
                return true;
    }
    if(eventOpts[cevent.event] & EOPT_ONLY_FROM_SELF_OR_PARENT_OR_CHILD) {
        uint16_t cn = cevent.sender;
        while((cn=getParentIdBI(cn)) != ID_S_Undefined)
            if(cevent.sender==cStateId)
                return true;
    }
    return false;
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


void SystemBase::sysTickCallback() {
    hwal->sysTick_MutexLockOrWait();

    sysTime++;

    for(uint16_t i=0; i!=timerNum; i++) {
        if(timerCounter[i]>1)
            timerCounter[i]--;

        if(timerCounter[i]==1) {
            raiseEventIdByIds(timerEvents[i], timerInitiator[i], true);
            timerCounter[i] = timerCounterRepeat[i];
        }
    }
    hwal->sysTick_MutexUnLock();
}

void SystemBase::raiseEventIdByIds(uint16_t eventId, uint16_t senderStateId, bool preventLog, EventPayloadBase *payload) {
    if(!preventLog) {
        HWAL_Log *lol = this->hwal->logger_get();
        if(this->doLogRaiseEvent)
            lol->logf(HWAL_Log::Info, HWAL_Log::IGreen, "!! R %s | %s", getEventName(eventId), getStateName(senderStateId));
    }

    raiseEvent_MutexLockOrWait();
    eventBuffer[eventBufferWritePos++] = {eventId, senderStateId, payload };
    eventBufferWritePos &= (1<<EVENT_BUFFER_SIZE_V) - 1;
    raiseEvent_MutexUnLock();
}

void SystemBase::executeTransition(uint16_t startState, uint16_t destState, uint16_t senderState, uint16_t event,
                                   bool blockActivatedStates) {
    HWAL_Log *lol = this->hwal->logger_get();

    if(!isStateActiveBI(startState)) {
        // @todo Add error message
        raiseEventIdByIds(ID_E_FatalError, senderState, false);
    }

    if(this->doLogTransitions)
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
        deactivateStateFullById(lastActiveChild);
        lastActiveChild = getParentIdBI(lastActiveChild);
    }
    // activate from common state to destState
    activateStateAndParentsByIds(destState, senderState, event, blockActivatedStates);
}

void SystemBase::activateStateFullByIds(uint16_t curStateId, uint16_t destStateId, uint16_t senderStateId,
                                        uint16_t event, bool blockActivatedStates, bool initMode) {
    HWAL_Log *lol = this->hwal->logger_get();

    bool isCStateActive = isStateActiveBI(curStateId);
    if(!isCStateActive || initMode || (isCStateActive && curStateId == destStateId)) {
        if(isCStateActive && !initMode)
            //getStateById(curStateId)->onExit(isCStateActive);
            deactivateStateFullById(curStateId);

        if(this->doLogEnterState)
            lol->logfll(HWAL_Log::Info, getStateById(curStateId)->llstate_get(), HWAL_Log::DGreen,
                        "!! EN %s (dest: %s) | %s", getStateName(curStateId), getStateName(destStateId),
                        getStateName(senderStateId));

        isStateActiveSetBI(curStateId, true);
        isStateBlockedSetBI(curStateId, blockActivatedStates);

        bool isDestState = curStateId == destStateId;

        getStateById(curStateId)->onEnter(senderStateId, event, isDestState, isCStateActive);

        if(isDestState) {
            bool hasInitialEventTransition = false;
            bool forceRaiseInitial = getStateById(curStateId)->emitInitialEventOnEnter();
            if(!forceRaiseInitial)
            for(uint16_t tii=0; tii!=transitionsNumberPerState[curStateId]; tii++) {
                TransitionImpl *tics = &(transitions[curStateId][tii]);
                if(tics->eventId == ID_E_Initial) {
                    hasInitialEventTransition = true;
                    break;
                }
            }
            if(hasInitialEventTransition || forceRaiseInitial)
                raiseEventIdByIds(ID_E_Initial, curStateId, LOG_INITIAL_EVENT);
        }
    }
}

void SystemBase::deactivateStateFullById(uint16_t curStateId) {
    HWAL_Log *lol = this->hwal->logger_get();

    if(this->doLogExitState)
        lol->logfll(HWAL_Log::Info, getStateById(curStateId)->llstate_get(), HWAL_Log::DOrange, "!! EX %s",
                    getStateName(curStateId));

    getStateById(curStateId)->onExit(false);
    isStateActiveSetBI(curStateId, false);

    for(uint16_t i=0; i!=timerNum; i++) {
        if(timerOwner[i] == curStateId)
            timerCounter[i] = 0;
    }
}

void SystemBase::activateStateAndParentsByIds(uint16_t destState, uint16_t senderState, uint16_t event,
                                              bool blockActivatedStates, bool initMode) {
    int counter = 0;
    uint16_t cstate = destState;

    // make list of all parent state
    do {
        stateListTemp[counter++] = cstate;
        cstate = getParentIdBI(cstate);
    } while((cstate!=SystemBase::ID_S_Undefined) && !isStateActiveBI(cstate));

    // activate all states top down
    while(counter>0) {
        counter--;
        activateStateFullByIds(stateListTemp[counter], destState, senderState,
                               event, blockActivatedStates, initMode);
    }
}

void SystemBase::initialSetup() {
    for(uint16_t i=0; i!=numberOfStates; i++) {
        this->getStateById(i)->set_hwal(this->hwal);
        if (isStateActiveBI(i))
            activateStateAndParentsByIds(i, SystemBase::ID_S_Initialization, SystemBase::ID_E_Undefined,
                                         false, true);
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


