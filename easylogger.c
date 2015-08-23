#ifdef NAMESPACE_EASYLOGGER

#define _GNU_SOURCE

#include <stdarg.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

typedef enum
{
    elLogDev_e = 0,
    elLogDbg_e,
    elLogInf_e,
    elLogWrn_e,
    elLogErr_e,
    elLogFtl_e,
}
elSeverityLevel_t;

typedef enum
{
    elFalse_e,
    elTrue_e,
}
elBool_t;

typedef struct
{
    elBool_t isInitialized;
    pthread_mutex_t streamLock;
    elSeverityLevel_t level;
    FILE* stream;
}
elLoggerData_t;

static elLoggerData_t loggerData = { elFalse_e, PTHREAD_MUTEX_INITIALIZER, elLogInf_e, NULL };

void elLoggerDestroy_f(void)
{
    if(loggerData.isInitialized != elTrue_e)
    {
        return;
    }

    loggerData.isInitialized = elFalse_e;

    pthread_mutex_lock(&loggerData.streamLock);

    if(loggerData.stream != NULL)
    {
        if(fflush(loggerData.stream) != 0)
        {
            perror("[FTL] error during flushing file stream during exit");
        }

        if(fclose(loggerData.stream) != 0)
        {
            perror("[FTL] error during closing log stream");
        }
    }

    pthread_mutex_unlock(&loggerData.streamLock);

    pthread_mutex_destroy(&loggerData.streamLock);

}

int elLoggerInitialize_f(FILE* stream, elSeverityLevel_t level)
{
    if(stream == NULL)
    {
        return EXIT_FAILURE;
    }

    loggerData.stream = stream;
    loggerData.level = level;

    if(pthread_mutex_init(&loggerData.streamLock, NULL) != 0)
    {
        return EXIT_FAILURE;
    }

    loggerData.isInitialized = elTrue_e;

    return EXIT_SUCCESS;
}

int elLoggerWriteMessage_f(elSeverityLevel_t severity,
                        int line,
                        const char* file,
                        const char* function,
                        const char* format,
                        ...)
{
    if(loggerData.isInitialized != elTrue_e)
    {
        return EXIT_FAILURE;
    }

    if(severity < loggerData.level)
    {
        return EXIT_FAILURE;
    }


    int rc = EXIT_SUCCESS;
    char timeString[20] =
    {
        'Y', 'Y', 'Y', 'Y', '-', 'M', 'M', '-', 'D', 'D', '_',
        'H', 'H', ':', 'M', 'M', ':', 'S', 'S', '\0'
    };
    time_t rawtime = time(NULL);

    if(strftime (timeString, 20,"%Y-%m-%d_%H:%M:%S", localtime (&rawtime)) == 0)
    {
        timeString[0] = 'Y';
        timeString[1] = 'Y';
        timeString[2] = 'Y';
        timeString[3] = 'Y';
        timeString[4] = '-';
        timeString[5] = 'M';
        timeString[6] = 'M';
        timeString[7] = '-';
        timeString[8] = 'D';
        timeString[9] = 'D';
        timeString[10] = '_';
        timeString[11] = 'H';
        timeString[12] = 'H';
        timeString[13] = ':';
        timeString[14] = 'M';
        timeString[15] = 'M';
        timeString[16] = ':';
        timeString[17] = 'S';
        timeString[18] = 'S';
        timeString[19] = '\0';
    }

    if(pthread_mutex_lock(&loggerData.streamLock) != 0)
    {
        return EXIT_FAILURE;
    }

    switch(severity)
    {
        case elLogDev_e:
            rc = fprintf(stderr, "[DEV][%s][", timeString);
            break;
        case elLogDbg_e:
            rc = fprintf(stderr, "[DBG][%s][", timeString);
            break;
        case elLogInf_e:
            rc = fprintf(stderr, "[INF][%s][", timeString);
            break;
        case elLogWrn_e:
            rc = fprintf(stderr, "[WRN][%s][", timeString);
            break;
        case elLogErr_e:
            rc = fprintf(stderr, "[ERR][%s][", timeString);
            break;
        case elLogFtl_e:
            rc = fprintf(stderr, "[FTL][%s][", timeString);
            break;
    }

    if(rc < 0)
    {
        perror("[FTL] error during writing log message header");
        rc = EXIT_FAILURE;
    }

    va_list arguments;
    va_start(arguments, format);
    if(vfprintf(stderr, format, arguments) < 0)
    {
        perror("[FTL] error during writing log message");
        rc = EXIT_FAILURE;
    }
    va_end(arguments);

    if(fprintf(stderr, "][%u/%u/%ld][%s:%d:%s()]\n", getppid(), getpid(), syscall(SYS_gettid), file, line, function) < 0)
    {
        perror("[FTL] error during writing log message trace");
        rc = EXIT_FAILURE;
    }

    if(fflush(stderr) != 0)
    {
        perror("[FTL] error during flushing file stream");
        rc = EXIT_FAILURE;
    }

    if(pthread_mutex_unlock(&loggerData.streamLock) != 0)
    {
        return EXIT_FAILURE;
    }

    return rc;
}

#define LOG_MSG_DEV( _MSG_ ) elLoggerWriteMessage_f(elLogDev_e, __LINE__, __FILE__, __func__, _MSG_)
#define LOG_MSG_DBG( _MSG_ ) elLoggerWriteMessage_f(elLogDbg_e, __LINE__, __FILE__, __func__, _MSG_)
#define LOG_MSG_INF( _MSG_ ) elLoggerWriteMessage_f(elLogInf_e, __LINE__, __FILE__, __func__, _MSG_)
#define LOG_MSG_WRN( _MSG_ ) elLoggerWriteMessage_f(elLogWrn_e, __LINE__, __FILE__, __func__, _MSG_)
#define LOG_MSG_ERR( _MSG_ ) elLoggerWriteMessage_f(elLogErr_e, __LINE__, __FILE__, __func__, _MSG_)
#define LOG_MSG_FTL( _MSG_ ) elLoggerWriteMessage_f(elLogFtl_e, __LINE__, __FILE__, __func__, _MSG_)

#define LOG_FMT_DEV( _MSG_ , ...) elLoggerWriteMessage_f(elLogDev_e, __LINE__, __FILE__, __func__, _MSG_, ## __VA_ARGS__)
#define LOG_FMT_DBG( _MSG_ , ...) elLoggerWriteMessage_f(elLogDbg_e, __LINE__, __FILE__, __func__, _MSG_, ## __VA_ARGS__)
#define LOG_FMT_INF( _MSG_ , ...) elLoggerWriteMessage_f(elLogInf_e, __LINE__, __FILE__, __func__, _MSG_, ## __VA_ARGS__)
#define LOG_FMT_WRN( _MSG_ , ...) elLoggerWriteMessage_f(elLogWrn_e, __LINE__, __FILE__, __func__, _MSG_, ## __VA_ARGS__)
#define LOG_FMT_ERR( _MSG_ , ...) elLoggerWriteMessage_f(elLogErr_e, __LINE__, __FILE__, __func__, _MSG_, ## __VA_ARGS__)
#define LOG_FMT_FTL( _MSG_ , ...) elLoggerWriteMessage_f(elLogFtl_e, __LINE__, __FILE__, __func__, _MSG_, ## __VA_ARGS__)


int main(void)
{
    elLoggerInitialize_f(stderr, elLogDev_e);
    elLoggerWriteMessage_f(elLogDev_e, __LINE__, __FILE__, __func__, "Hello %d. %s", 1, "World!");
    elLoggerWriteMessage_f(elLogDbg_e, __LINE__, __FILE__, __func__, "Hello %d. %s", 1, "World!");
    elLoggerWriteMessage_f(elLogInf_e, __LINE__, __FILE__, __func__, "Hello %d. %s", 1, "World!");
    elLoggerWriteMessage_f(elLogWrn_e, __LINE__, __FILE__, __func__, "Hello %d. %s", 1, "World!");
    elLoggerWriteMessage_f(elLogErr_e, __LINE__, __FILE__, __func__, "Hello %d. %s", 1, "World!");
    elLoggerWriteMessage_f(elLogFtl_e, __LINE__, __FILE__, __func__, "Hello %d. %s", 1, "World!");
    elLoggerWriteMessage_f(elLogFtl_e, __LINE__, NULL, NULL, "Hello %d. %s", 1, "World!");

    LOG_MSG_FTL("hello World");

    elLoggerDestroy_f();

    return 0;
}

#endif // NAMESPACE_EASYLOGGER
