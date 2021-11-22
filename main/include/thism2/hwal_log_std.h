
#ifndef CARA_FW_DEVICE_HWAL_LOG_STD_H
#define CARA_FW_DEVICE_HWAL_LOG_STD_H

#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <chrono>
#include <thread>

#include "hwal_x.h"


class HWAL_Log_Std : public HWAL_Log {
protected:
    virtual void write_to_log(const char *str){
        printf("%s", str);
    }

    virtual int write_to_mem(char *dst, int len, const char *format, va_list args) {
        return vsnprintf(dst, len, format, args);
    }

    virtual void write_time(uint64_t time_secs) {
        char buffer[16];
        time_t rawtime = time_secs;
        struct tm * timeinfo = localtime (&rawtime);

        strftime (buffer,16,"%y%m%d-%H:%M:%S",timeinfo);
        printf("%s", buffer);
    }

public:
    HWAL_Log_Std(HWAL_Log::LogLevel ll) : HWAL_Log(ll)
    { }
};


#endif // CARA_FW_DEVICE_HWAL_LOG_STD_H
