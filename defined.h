//
// Created by wlanjie on 2018/2/7.
//

#ifndef MP4_DEFINED_H
#define MP4_DEFINED_H

#define DYNAMIC_CAST(_class,_object) dynamic_cast<_class*>(_object)

#if defined (CONFIG_HAVE_ASSERT_H)
#include <assert.h>
#define ASSERT assert
#else
#define ASSERT
#endif

#endif //MP4_DEFINED_H
