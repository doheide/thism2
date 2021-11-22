// *******************************************
#include "sm_sys.h"
#include "sm_diagrams.h"
//#include "diagram_info.cpp"

SMSys *smsys;

int main() {
    HWAL_Std_No_HW hwal(HWAL_Log::Debug);
    SMSys csmsys((HWAL*) &hwal);
    SMSys *smsys = &csmsys;

    make_treeuml_allSMs(&csmsys, "diagrams_out");
}
