//
// Created by wlanjie on 2018/2/7.
//

#ifndef MP4_INTERFACES_H
#define MP4_INTERFACES_H

#include "types.h"

#define ADD_REFERENCE(o) do { if (o) (o)->addReference(); } while (0)
#define RELEASE(o) do { if (o) (o)->release(); (o) = NULL; } while (0)

//#define DEBUG(fmt, ...) \
//va_list args; \
//va_start(args, fmt); \
//char buffer[1024]; \
//FormatStringVN(buffer, sizeof(buffer), fmt, args); \
//printf("%s", buffer); \
//va_end(args); \

class Exception {
public:
    Exception(Result error) : error(error) {}
    Result error;
};

class Referenceable {
public:
    virtual ~Referenceable() {}
    virtual void addReference() = 0;
    virtual void release() = 0;
};

#endif //MP4_INTERFACES_H
