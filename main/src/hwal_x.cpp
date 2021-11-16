//
// Created by dheide on 15.11.21.
//
#include "hwal_x.h"


void HWAL_Log::logs(LogLevel ll, const char *str) {
    if((int)ll > 0)
        if((int) ll > (int) this->cloglevel)
            return;

    int last_len;
    char *cstart = (char*) str, *cend = (char*) str;
    bool finish = false;
    while(true) {
        for (; (*cend != 0) && (*cend != '\n'); cend++);

        if(this->hwal == 0)
            write_to_log("xxxxxx-xxxxxx ");
        else {
            uint64_t t = this->hwal->get_time();
            write_time(t);
            write_to_log(" ");
        }
        write_loglevel(ll);
        if(cstart==str)
            write_to_log(" - ");
        else
            write_to_log(" | ");

        if(*cend==0)
            finish = true;
        *cend = 0;
        write_to_log(cstart);

        cstart = cend+1; cend = cstart;
        write_to_log("\n");
        if((*cstart == 0) || finish)
            break;
    }
}
