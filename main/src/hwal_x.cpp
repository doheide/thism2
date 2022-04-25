//
// Created by dheide on 15.11.21.
//
#include "thism2/hwal_x.h"


void HWAL_Log::logs(LogLevel ll, int8_t color, const char *str, const char *statename,
                    LogLevel ll_in) {
    if(ll_in == LogLevel::Always)
        ll_in = this->cloglevel;

    if((int)ll > 0)
        if((int) ll_in > (int) this->cloglevel)
            return;

//    int last_len;
    char *cstart = (char*) str, *cend = (char*) str;
    bool finish = false;
    uint8_t time_area = 13;
    while(true) {
        for (; (*cend != 0) && (*cend != '\n'); cend++);

        if(cstart==str) {
            if (this->hwal == nullptr)
                write_to_log("xxxxxx-xxxxxx ");
            else {
                uint64_t t = this->hwal->get_time();
                //write_time(t);
                time_area = this->hwal->write_logger_time(t);
                write_to_log(" ");
            }
            write_loglevel(ll);
        }
        else for(int i=0; i!=time_area+9; i++)
                write_to_log(" ");

        if(statename!=0) {
            write_to_log("[");
            write_to_log(statename);
            write_to_log("]");
        }

        if(cstart==str)
            write_to_log(" ");
        else
            write_to_log("| ");

        write_color(color);
        if(*cend==0)
            finish = true;
        *cend = 0;
        write_to_log(cstart);

        cstart = cend+1; cend = cstart;
        write_color(-1);
        write_to_log("\n");
        if((*cstart == 0) || finish)
            break;
    }
}
