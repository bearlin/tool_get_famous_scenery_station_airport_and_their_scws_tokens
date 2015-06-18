#ifndef __TTLOG_H__
#define __TTLOG_H__

#include <stdio.h>

#define DEFINE_LOGGER(tag,string) static const char tag[] = string;

//#define DO_LOG(tag,fmt,...)  printf("[%s] " fmt "\n", tag, ##__VA_ARGS__);
#define DO_LOG(tag,fmt,...)  printf("[%s] " fmt, tag, ##__VA_ARGS__);

#define LOG_ERROR   DO_LOG
#define LOG_WARNING DO_LOG
#define LOG_INFO    DO_LOG
#define LOG_DEBUG   DO_LOG

#define LOG LOG_DEBUG

#endif // __TTLOG_H__

