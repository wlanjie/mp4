//
// Created by wlanjie on 2018/3/7.
//

#include "filewriter.h"

Result FileWriter::write(File &file, ByteStream &stream, FileWriter::Interleaving interleaving) {
    Ftyp* fileType = file.getFileType();
    if (fileType) {
        fileType->write(stream);
    }

    return SUCCESS;
}
