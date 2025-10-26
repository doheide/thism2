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
// *****************************************************************************************
Make_State_Get_ID(SMSys, S_LED_Off)
void S_LED_Off::onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering, void*) {
    HWAL_Std_No_HW *chwal = reinterpret_cast<HWAL_Std_No_HW *>(smsys->hwalBaseGet());
    chwal->set_led(0);
}

Make_State_Get_ID(SMSys, S_LED_On)
void S_LED_On::onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering, void*) {
    HWAL_Std_No_HW *chwal = reinterpret_cast<HWAL_Std_No_HW *>(smsys->hwalBaseGet());
    chwal->set_led(1);
}

Make_State_Get_ID(SMSys, S_Off)
void S_Off::onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering, void*) {
    smsys->raiseEvent<E_LED_off, ThisState>();
}

Make_State_Get_ID(SMSys, S_On)
void S_On::onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering, void*) {
    smsys->raiseEvent<E_LED_on, ThisState>();
}

Make_State_Get_ID(SMSys, S_Blink)

Make_State_Get_ID(SMSys, S_Blink_On)
void S_Blink_On::onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering, void*) {
    smsys->raiseEvent<E_LED_on, ThisState>();
    smsys->startTimer<SMT_Main, ThisState>(false, 1);
}

Make_State_Get_ID(SMSys, S_Blink_Off)
void S_Blink_Off::onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering, void*) {
    smsys->raiseEvent<E_LED_off, ThisState>();
    smsys->startTimer<SMT_Main, ThisState>(false, 1);
}




