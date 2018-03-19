//
// Created by wlanjie on 2018/3/10.
//

#include "debug.h"

void debug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[1024];
    FormatStringVN(buffer, sizeof(buffer), format, args);
    printf("%s", buffer);
    va_end(args);
}