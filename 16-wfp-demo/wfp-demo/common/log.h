#pragma once

#include "file_help.h"
#include <wdm.h>
#include <ntstrsafe.h>

#define BUFF_SIZE 256

int log_init(wchar_t* plog_file);

int log_exit();

int write_log(char* format, va_list arg, BOOLEAN thread_safe);

void log_debug(char* format, ...);

void log_debug_safe(char* format, ...);

#if DBG 
#define kd_log_debug(_x_) log_debug _x_
#define kd_log_debug_safe(_x_) log_debug_safe _x_
#else
#define kd_log_debug(_x_)
#define kd_log_debug_safe(_x_)
#endif
