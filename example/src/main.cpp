//
// Created by dheide on 19.11.21.
//

#include "sm_sys.h"

SMSys *smsys;

int main() {
    HWAL_Std_No_HW hwal(HWAL_Log::Debug);
    SMSys csmsys(&hwal);
    smsys = &csmsys;

    smsys->initialSetup();

    while(smsys->sysTimeGet() < 20) {
        smsys->processEvents();
        smsys->processEvents();

        hwal.sleep_ticks(100);
        smsys->sysTickCallback();

        if(smsys->sysTimeGet() == 3) {
            hwal.logger_get()->logf(HWAL_Log::Info, HWAL_Log::NoColor, "* Switching LED on");
            smsys->raiseEvent_noSender<E_On>();
        }
        else if(smsys->sysTimeGet() == 5) {
            hwal.logger_get()->logf(HWAL_Log::Info, HWAL_Log::NoColor,"* Switching LED blink on");
            smsys->raiseEvent_noSender<E_On_Blink>();
        }
        else if(smsys->sysTimeGet() == 15) {
            hwal.logger_get()->logf(HWAL_Log::Info, HWAL_Log::NoColor,"* Switching LED off");
            smsys->raiseEvent_noSender<E_Off>();
        }

    }
}
