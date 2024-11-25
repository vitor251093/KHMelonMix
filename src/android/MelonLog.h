#ifndef MELONLOG_H
#define MELONLOG_H

#include <android/log.h>

#define LOG_DEBUG(tag, message, ...) __android_log_print(ANDROID_LOG_DEBUG, tag, message, ##__VA_ARGS__)
#define LOG_INFO(tag, message, ...) __android_log_print(ANDROID_LOG_INFO, tag, message, ##__VA_ARGS__)
#define LOG_ERROR(tag, message, ...) __android_log_print(ANDROID_LOG_ERROR, tag, message, ##__VA_ARGS__)

#endif //MELONLOG_H
