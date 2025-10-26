//
// Created by dheide on 19.11.21.
//

#include "sm_sys.h"

// *****************************************************************************************
// *****************************************************************************************
void HWAL_Std_No_HW::set_led(uint8_t v) {
    if(v==0)
        smsys->hwalBaseGet()->logger_get()->logf(HWAL_Log::Always, HWAL_Log::NoColor, "***** LED OFF");
    else
        smsys->hwalBaseGet()->logger_get()->logf(HWAL_Log::Always, HWAL_Log::NoColor, "***** LED ON");
}


// *****************************************************************************************
Make_State_Get_ID(SMSys, S_LogTest_L1)
void S_LogTest_L1::onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering, void *p) {
    logf(HWAL_Log::Always, HWAL_Log::Yellow, "Always");
    logf(HWAL_Log::Info, HWAL_Log::Yellow, "Info");
    logf(HWAL_Log::Warning, HWAL_Log::Yellow, "Warning");
    logf(HWAL_Log::Error, HWAL_Log::Yellow, "Error");
    logf(HWAL_Log::Debug, HWAL_Log::Yellow, "Debug");
    logf(HWAL_Log::Details, HWAL_Log::Yellow, "Details");

    smsys->raiseEvent<E_Next, ThisState>();
}

// *****************************************************************************************
Make_State_Get_ID(SMSys, S_LogTest_L2)
void S_LogTest_L2::onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering, void *p) {
    logf(HWAL_Log::Always, HWAL_Log::LBlue, "Always");
    logf(HWAL_Log::Info, HWAL_Log::LBlue, "Info");
    logf(HWAL_Log::Warning, HWAL_Log::LBlue, "Warning");
    logf(HWAL_Log::Error, HWAL_Log::LBlue, "Error");
    logf(HWAL_Log::Debug, HWAL_Log::LBlue, "Debug");
    logf(HWAL_Log::Details, HWAL_Log::LBlue, "Details");

    smsys->raiseEvent<E_Next, ThisState>();
}

// *****************************************************************************************
Make_State_Get_ID(SMSys, S_LogTest_L3)

void S_LogTest_L3::onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering, void *p) {
    logf(HWAL_Log::Always, HWAL_Log::Red, "Always");
    logf(HWAL_Log::Info, HWAL_Log::Red, "Info");
    logf(HWAL_Log::Warning, HWAL_Log::Red, "Warning");
    logf(HWAL_Log::Error, HWAL_Log::Red, "Error");
    logf(HWAL_Log::Debug, HWAL_Log::Red, "Debug");
    logf(HWAL_Log::Details, HWAL_Log::Red, "Details");

    smsys->raiseEvent<E_Next, ThisState>();
}

// *****************************************************************************************
Make_State_Get_ID(SMSys, S_LogTest_L4)

void S_LogTest_L4::onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering, void *p) {
    logf(HWAL_Log::Always, HWAL_Log::IGreen, "Always");
    logf(HWAL_Log::Info, HWAL_Log::IGreen, "Info");
    logf(HWAL_Log::Warning, HWAL_Log::IGreen, "Warning");
    logf(HWAL_Log::Error, HWAL_Log::IGreen, "Error");
    logf(HWAL_Log::Debug, HWAL_Log::IGreen, "Debug");
    logf(HWAL_Log::Details, HWAL_Log::IGreen, "Details");

    smsys->raiseEvent<E_Next, ThisState>();
}



