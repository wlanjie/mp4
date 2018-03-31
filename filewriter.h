//
// Created by wlanjie on 2018/3/7.
//

#ifndef MP4_FILEWRITER_H
#define MP4_FILEWRITER_H

#include "types.h"
#include "file.h"

namespace mp4 {

class FileWriter {
public:
    typedef enum {
        INTERLEAVING_SEQUENTIAL
    } Interleaving;
    static Result write(File& file, ByteStream& stream, Interleaving interleaving = INTERLEAVING_SEQUENTIAL);

private:
    FileWriter() {}
};

}
#endif //MP4_FILEWRITER_H
