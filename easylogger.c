#ifdef NAMESPACE_EASYLOGGER

#include <stdarg.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

typedef enum
{
    elLogDev_e = 0,
    elLogDbg_e,
    elLogInf_e,
    elLogWrn_e,
    elLogFtl_e,
}
elSeverityLevel_t;

typedef struct
{
    int line;
    const char* file;
    const char* function;
    const char* format;
    va_list arguments;
} elLogMessage_t;

typedef enum
{
    elFalse_e = 0,
    elTrue_e = 1
}
elBool_t;

#define EL_MESSAGE_BUFFER_LENGTH 1024

typedef struct
{
    elBool_t isInitialized;
    elSeverityLevel_t currentSeverity;
    pthread_mutex_t bufferLock;
    uint16_t threadsWaitingForWrite;
    elLogMessage_t* bufferTop;
    elLogMessage_t* bufferLimit;
    elLogMessage_t messageBuffer[EL_MESSAGE_BUFFER_LENGTH];
}
elLoggerData_t;

static elLoggerData_t loggerData;

void elLoggerDestroy_f(void)
{
    if(loggerData.isInitialized != elTrue_e)
    {
        return;
    }
}

int elInitialize_f(void)
{
    memset(&loggerData, 0, sizeof(elLoggerData_t));

    if(atexit(elLoggerDestroy_f))
    {
    }

    if(pthread_mutex_init(&loggerData.bufferLock, NULL))
    {
    }

    loggerData.bufferTop = loggerData.messageBuffer;
    loggerData.bufferLimit = loggerData.messageBuffer + EL_MESSAGE_BUFFER_LENGTH;


    loggerData.isInitialized = elTrue_e;
    return 0;
}


int elWriteLogMessage_f(elSeverityLevel_t severity,
                        int line,
                        const char* file,
                        const char* function,
                        const char* format,
                        ...)
{
    // lock all

    if(loggerData.isInitialized != elTrue_e)
    {
        // unlock
        return 1;
    }

    if(severity < loggerData.currentSeverity)
    {
        // unlock
        return 2;
    }

    if (loggerData.bufferTop >= loggerData.bufferLimit)
    {
        loggerData.threadsWaitingForWrite++;
        // and wait for empty space in buffer
    }

    // some underflow
    if (loggerData.bufferTop < loggerData.messageBuffer)
    {
        loggerData.bufferTop = loggerData.messageBuffer;
    }

    va_list currentArguments;

    va_start(currentArguments, format);
    va_copy(loggerData.bufferTop->arguments, currentArguments);
    va_end(currentArguments);

    loggerData.bufferTop->line = line;
    loggerData.bufferTop->file = file;
    loggerData.bufferTop->function = function;
    loggerData.bufferTop->format = format;

    loggerData.bufferTop++;

    // unlock

    return 0;

}


int main(void)
{
    return 0;
}

#endif // NAMESPACE_EASYLOGGER
