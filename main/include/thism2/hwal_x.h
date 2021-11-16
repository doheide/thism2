//
// Created by dheide on 10.11.21.
//

#ifndef CARA_FW_DEVICE_HWAL_X_H
#define CARA_FW_DEVICE_HWAL_X_H

#include <cstdint>
#include <cstdarg>


// **********************************************************************************
// **********************************************************************************
#ifndef HWAL_LOG_BUFFER_SIZE
#define HWAL_LOG_BUFFER_SIZE 1024
#endif

class HWAL;

class HWAL_Log {
    friend HWAL;

protected:
    HWAL *hwal;

    virtual void write_to_log(const char *) = 0;
    virtual int write_to_mem(char */*dst*/, int /*len*/, const char */*format*/, va_list /*arg*/) = 0;
    virtual void write_time(uint64_t time) = 0;

    void hwal_set(HWAL *hwal) {
        this->hwal = hwal;
    }

public:
    enum LogLevel {Always, Error, Warning, Info, Details, Debug};

protected:
    LogLevel cloglevel;

    void write_loglevel(LogLevel ll) {
        switch(ll) {
            case Always: write_to_log("[-------]"); break;
            case Error: write_to_log("[Error  ]"); break;
            case Warning: write_to_log("[Warning]"); break;
            case Info: write_to_log("[Info   ]"); break;
            case Debug: write_to_log("[Debug  ]"); break;
            case Details: write_to_log("[Details]"); break;
        };
    }

public:
    HWAL_Log(LogLevel ll) {
        this->cloglevel = ll;
        this->hwal = 0;
    }


public:
    virtual void logs(LogLevel ll, const char *str);

    virtual void logf(LogLevel ll, const char *format, ...) {
        va_list args;
        va_start(args, format);

        char str[HWAL_LOG_BUFFER_SIZE];
        int r = write_to_mem(str, HWAL_LOG_BUFFER_SIZE, format, args);
        va_end(args);

        logs(ll, str);
    }

};

// *****************************************************************************************************
// *****************************************************************************************************
class HWAL {
    HWAL_Log *logger;

public:
    HWAL(HWAL_Log *hwal_log) {
        this->logger = hwal_log;
        this->logger->hwal_set(this);
    }

    virtual void print_chip_info() = 0;
    virtual void reboot() = 0;
    virtual void sleep_ms(uint64_t msecs) = 0;

    virtual uint64_t get_time() = 0;

    HWAL_Log *logger_get() { return logger; }
};



#endif //CARA_FW_DEVICE_HWAL_X_H
