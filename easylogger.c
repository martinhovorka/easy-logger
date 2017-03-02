/*
 *   EasyLogger - Simple thread-safe logging library
 *   Copyright (C) 2016, 2017  Martim Hovorka
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "easylogger.h" 

#include <stdlib.h>

#ifdef NAMESPACE_EASYLOGGER

#define _GNU_SOURCE

#include <stdarg.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>

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

static elLoggerData_t loggerData = {elFalse_e, PTHREAD_MUTEX_INITIALIZER, elLogInf_e, NULL};

void elLoggerDestroy_f(void)
{
    if (loggerData.isInitialized != elTrue_e)
    {
        return;
    }

    if (pthread_mutex_lock(&loggerData.streamLock) != 0)
    {
        perror("[FTL] error during locking file stream mutex during exit");
        return;
    }

    if (loggerData.stream != NULL)
    {
        if (fflush(loggerData.stream) != 0)
        {
            perror("[FTL] error during flushing file stream during exit");
            return;
        }

        if (fclose(loggerData.stream) != 0)
        {
            perror("[FTL] error during closing log stream");
            return;
        }
    }

    if (pthread_mutex_unlock(&loggerData.streamLock) != 0)
    {
        perror("[FTL] error during unlocking file stream mutex during exit");
        return;
    }

    if (pthread_mutex_destroy(&loggerData.streamLock))
    {
        perror("[FTL] unable to destroy file stream mutex during exit");
        return;
    }

    loggerData.isInitialized = elFalse_e;
}

int elLoggerInitialize_f(FILE* stream, const elSeverityLevel_t level)
{
    if (stream == NULL)
    {
        loggerData.stream = stdout;
    }
    else
    {
        loggerData.stream = stream;
    }

    loggerData.level = level;

    if (pthread_mutex_init(&loggerData.streamLock, NULL) != 0)
    {
        perror("[FTL] unable to initialize file stream mutex");
        return EXIT_FAILURE;
    }

    loggerData.isInitialized = elTrue_e;

    return EXIT_SUCCESS;
}

int elLoggerWriteMessage_f(const elSeverityLevel_t severity,
                           const int line,
                           const char* file,
                           const char* function,
                           const char* format,
                           ...)
{
    if (loggerData.isInitialized != elTrue_e)
    {
        return EXIT_FAILURE;
    }

    if (severity < loggerData.level)
    {
        return EXIT_FAILURE;
    }

    int rc = EXIT_SUCCESS;

    char timeString[20] = {
        'Y', 'Y', 'Y', 'Y', '-', 'M', 'M', '-', 'D', 'D', '_',
        'H', 'H', ':', 'M', 'M', ':', 'S', 'S', '\0'
    };
    time_t rawtime = time(NULL);

    if (strftime(timeString, 20, "%Y-%m-%d_%H:%M:%S", localtime(&rawtime)) == 0)
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

    if (pthread_mutex_lock(&loggerData.streamLock) != 0)
    {
        perror("[FTL] unable to lock file stream mutex");
        return EXIT_FAILURE;
    }

    switch (severity)
    {
    case elLogDev_e:
        rc = fprintf(loggerData.stream, "[DEV][%s][", timeString);
        break;
    case elLogDbg_e:
        rc = fprintf(loggerData.stream, "[DBG][%s][", timeString);
        break;
    case elLogInf_e:
        rc = fprintf(loggerData.stream, "[INF][%s][", timeString);
        break;
    case elLogWrn_e:
        rc = fprintf(loggerData.stream, "[WRN][%s][", timeString);
        break;
    case elLogErr_e:
        rc = fprintf(loggerData.stream, "[ERR][%s][", timeString);
        break;
    case elLogFtl_e:
        rc = fprintf(loggerData.stream, "[FTL][%s][", timeString);
        break;
    }

    if (rc < 0)
    {
        perror("[FTL] error during writing log message header");
        rc = EXIT_FAILURE;
    }

    va_list arguments;
    va_start(arguments, format);
    if (vfprintf(loggerData.stream, format, arguments) < 0)
    {
        perror("[FTL] error during writing log message");
        rc = EXIT_FAILURE;
    }
    va_end(arguments);

    const char* trimmedFile = file;
    if (trimmedFile != NULL)
    {
        trimmedFile = strrchr(trimmedFile, '/');
        if (trimmedFile == NULL)
        {
            trimmedFile = file;
        }
        else
        {
            if (strlen(trimmedFile) != 1)
            {
                trimmedFile++;
            }
        }
    }

    if (fprintf(loggerData.stream, "][%u/%u/%ld][%s:%d:%s()]\n", getppid(), getpid(), syscall(SYS_gettid), trimmedFile, line, function) < 0)
    {
        perror("[FTL] error during writing log message trace");
        rc = EXIT_FAILURE;
    }

    if (fflush(loggerData.stream) != 0)
    {
        perror("[FTL] error during flushing file stream");
        rc = EXIT_FAILURE;
    }

    if (pthread_mutex_unlock(&loggerData.streamLock) != 0)
    {
        perror("[FTL] unable to unlock file stream mutex");
        return EXIT_FAILURE;
    }

    return rc;
}

#else /* NAMESPACE_EASYLOGGER */

void elLoggerDestroy_f(void)
{
}

#pragma GCC diagnostic ignored "-Wunused-parameter" /* temporarily disable unused parameter warnings */

int elLoggerInitialize_f(FILE* stream, const elSeverityLevel_t level)
{
    return EXIT_SUCCESS;
}

int elLoggerWriteMessage_f(const elSeverityLevel_t severity,
                           const int line,
                           const char* file,
                           const char* function,
                           const char* format,
                           ...)
{
    return EXIT_SUCCESS;
}

#pragma GCC diagnostic warning "-Wunused-parameter" /* restore warnings */

#endif /* NAMESPACE_EASYLOGGER */

