//
// Created by dheide on 10.11.21.
//

#pragma once
//#ifndef CARA_FW_DEVICE_HWAL_X_H
//#define CARA_FW_DEVICE_HWAL_X_H

#include <cstdint>
#include <cstdarg>

// **********************************************************************************
// **********************************************************************************
#ifndef HWAL_LOG_BUFFER_SIZE
#define HWAL_LOG_BUFFER_SIZE 1024
#endif

class HWAL;
//template<typename LoggerT> class HWAL_LoggerT;

struct StateBase;

class HWAL_Log {
    friend HWAL;
    friend StateBase;

    char log_str[HWAL_LOG_BUFFER_SIZE];

protected:
    HWAL *hwal;

    virtual void write_to_log(const char *) = 0;
    virtual int write_to_mem(char */*dst*/, int /*len*/, const char */*format*/, va_list /*arg*/) = 0;
    virtual uint8_t write_time(uint64_t time) = 0;

    void hwal_set(HWAL *hwal) {
        this->hwal = hwal;
    }

public:
    enum LogLevel {Always, Error, Warning, Info, Details, Debug};
    enum Color {Reset=-2, NoColor, DDRed, DRed, Red, DGreen, IGreen, Yellow, Pink, LBlue, Orange, DOrange};

protected:
    LogLevel cloglevel;

    void write_color(int8_t c) {
        switch(c) {
            case (int) DDRed: write_to_log("\033[38;5;124m"); break;
            case (int) DRed: write_to_log("\033[38;5;1m"); break;
            case (int) Red: write_to_log("\033[38;5;9m"); break;
            case (int) IGreen: write_to_log("\033[38;5;2m"); break;
            case (int) DGreen: write_to_log("\033[38;5;10m"); break;
            case (int) Yellow: write_to_log("\033[38;5;11m"); break;
            case (int) Pink: write_to_log("\033[38;5;13m"); break;
            case (int) LBlue: write_to_log("\033[38;5;14m"); break;
            case (int) Orange: write_to_log("\033[38;5;208m"); break;
            case (int) DOrange: write_to_log("\033[38;5;208m"); break;
            default: write_to_log("\033[0m"); break;
        }

    }
    void write_loglevel(LogLevel ll) {
        switch(ll) {
            case Always: write_to_log("[-------]"); break;
            case Error: write_to_log("\033[41m[Error  ]\033[0m"); break;
            case Warning: write_to_log("\033[208m[Warning]\033[0m"); break;
            case Info: write_to_log("[Info   ]"); break;
            case Debug: write_to_log("[Debug  ]"); break;
            case Details: write_to_log("[Details]"); break;
        };
    }

public:
    explicit HWAL_Log(LogLevel ll = LogLevel::Always) : log_str{} {
        this->cloglevel = ll;
        this->hwal = nullptr;
    }

public:
    void logs(LogLevel ll, int8_t color, const char *str, const char *statename=nullptr,
              LogLevel ll_in=LogLevel::Always);

    virtual void logfll(LogLevel ll, LogLevel ll_display, int8_t color, const char *format, ...) {
        va_list args;
        va_start(args, format);

        write_to_mem(log_str, HWAL_LOG_BUFFER_SIZE, format, args);
        va_end(args);

        logs(ll, color, log_str, nullptr, ll_display);
    }

    virtual void logf(LogLevel ll, int8_t color, const char *format, ...) {
        va_list args;
        va_start(args, format);

        write_to_mem(log_str, HWAL_LOG_BUFFER_SIZE, format, args);
        va_end(args);

        logs(ll,  color, log_str);
    }

    virtual LogLevel loglevel_to_display_get() { return cloglevel; }

protected:
    virtual void log_args(LogLevel ll, int8_t color, const char *format, va_list args,
                          const char *statename= nullptr, LogLevel ll_display=LogLevel::Always) {
        write_to_mem(log_str, HWAL_LOG_BUFFER_SIZE, format, args);
        logs(ll, color, log_str, statename, ll_display);
    }
};

// *****************************************************************************************************
// *****************************************************************************************************
class HWAL {
protected:
    void set_this_to_logger(HWAL_Log *logger) {
        logger->hwal_set(this);
    }

public:
    // virtual void sleep_ms(uint64_t msecs) = 0;
    virtual void sleep_ticks(uint64_t ticks) = 0;

    virtual void sysTick_MutexLockOrWait() = 0;
    virtual void sysTick_MutexUnLock() = 0;

    virtual void eventProcess_MutexLockOrWait() = 0;
    virtual void eventProcess_MutexUnLock() = 0;

    virtual uint64_t get_time() = 0;
    virtual uint8_t write_logger_time(uint64_t) = 0;

    virtual HWAL_Log *logger_get() = 0;
};


template<typename LoggerT>
class HWAL_LoggerT : public HWAL {
protected:
    LoggerT *logger;

public:
    HWAL_LoggerT(HWAL_Log::LogLevel ll) {
        this->logger = new LoggerT(ll);
        this->set_this_to_logger(this->logger);
    }

    HWAL_Log *logger_get() { return logger; }
    LoggerT *logger_t_get() { return logger; }

    virtual uint8_t write_logger_time(uint64_t t)  {
        return logger->write_time(t);
    }
};


//#endif //CARA_FW_DEVICE_HWAL_X_H
