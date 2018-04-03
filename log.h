//
// Created by wlanjie on 2018/4/3.
//

#ifndef MP4_LOG_H
#define MP4_LOG_H

#define DEBUG

#ifdef __ANDROID__
#include <android/log.h>
#define TAG "mp4"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#else
#include <printf.h>
#define LOGV(...) printf(__VA_ARGS__)
#endif

#endif //MP4_LOG_H
