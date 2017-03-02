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

#ifndef EASYLOGGER_H
#define EASYLOGGER_H

#define _GNU_SOURCE

#include <stdio.h>

typedef enum {
    elLogDev_e = 0,
    elLogDbg_e,
    elLogInf_e,
    elLogWrn_e,
    elLogErr_e,
    elLogFtl_e,
}
elSeverityLevel_t;

void elLoggerDestroy_f(void);
int elLoggerInitialize_f(FILE* stream, const elSeverityLevel_t level);
int elLoggerWriteMessage_f(const elSeverityLevel_t severity,
	const int line,
	const char* file,
	const char* function,
	const char* format,
	...);

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

#undef _GNU_SOURCE

#endif /* EASYLOGGER_H */

