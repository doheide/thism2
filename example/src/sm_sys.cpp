//
// Created by dheide on 19.11.21.
//

#include "sm_sys.h"

// *****************************************************************************************
// *****************************************************************************************
void HWAL_Std_No_HW::set_led(uint8_t v) {
    if(v==0)
        smsys->hwalBaseGet()->logger_get()->logf(HWAL_Log::Always, "***** LED OFF");
    else
        smsys->hwalBaseGet()->logger_get()->logf(HWAL_Log::Always, "***** LED ON");
}


// *****************************************************************************************
// *****************************************************************************************
void S_LED_Off::onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering) {
    HWAL_Std_No_HW *chwal = reinterpret_cast<HWAL_Std_No_HW *>(smsys->hwalBaseGet());
    chwal->set_led(0);
}

void S_LED_On::onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering) {
    HWAL_Std_No_HW *chwal = reinterpret_cast<HWAL_Std_No_HW *>(smsys->hwalBaseGet());
    chwal->set_led(1);
}

void S_Off::onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering) {
    smsys->raiseEvent<E_LED_off, ThisState>();
}

void S_On::onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering) {
    smsys->raiseEvent<E_LED_on, ThisState>();
}

void S_Blink_On::onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering) {
    smsys->raiseEvent<E_LED_on, ThisState>();
    smsys->startTimer<SMT_Std, ThisState>(false, 1);
}

void S_Blink_Off::onEnter(uint16_t senderStateId, uint16_t event, bool isDestState, bool reentering) {
    smsys->raiseEvent<E_LED_off, ThisState>();
    smsys->startTimer<SMT_Std, ThisState>(false, 1);
}




