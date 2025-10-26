//
// Created by dheide on 19.11.21.
//

#include "sm_sys.h"

SMSys *smsys;

int main() {
    HWAL_Std_No_HW hwal(HWAL_Log::_MaxLogLevel);
    hwal.logger_get()->print_log_level();

    SMSys csmsys(&hwal);
    smsys = &csmsys;

    smsys->initialSetup();

    while(smsys->sysTimeGet() < 20) {
        smsys->processEvents();
//        smsys->processEvents();

        hwal.sleep_ticks(100);
        smsys->sysTickCallback();
    }
}
