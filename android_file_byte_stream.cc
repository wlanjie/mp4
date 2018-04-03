//
// Created by wlanjie on 2018/4/3.
//

#include "android_file_byte_stream.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

namespace android {

ssize_t writePartial(int fd, const void *buffer, size_t size) {
    return write(fd, buffer, size);
}

ssize_t readPartial(int fd, void *buffer, size_t size) {
    return read(fd, buffer, size);
}

}

namespace mp4 {

Result AndroidFileByteStream::create(FileByteStream* delegator,
                                     const char* name,
                                     FileByteStream::Mode mode,
                                     ByteStream*& stream) {
    stream = NULL;

    if (name == NULL) return ERROR_INVALID_PARAMETERS;

    int fd = 0;
    Position size = 0;
    if (!strcmp(name, "-stdin")) {
        fd = STDIN_FILENO;
    } else if (!strcmp(name, "-stdout")) {
        fd = STDOUT_FILENO;
    } else if (!strcmp(name, "-stderr")) {
        fd = STDERR_FILENO;
    } else {
        int open_flags = 0;
        int create_perm = 0;
        switch (mode) {
            case FileByteStream::STREAM_MODE_READ:
                open_flags = O_RDONLY;
                break;

            case FileByteStream::STREAM_MODE_WRITE:
                open_flags = O_RDWR | O_CREAT | O_TRUNC;
                break;

            case FileByteStream::STREAM_MODE_READ_WRITE:
                open_flags = O_RDWR;
                break;

            default:
                return ERROR_INVALID_PARAMETERS;
        }

        fd = open(name, open_flags, create_perm);
        if (fd < 0) {
            if (errno == ENOENT) {
                return ERROR_NO_SUCH_FILE;
            } else if (errno == EACCES) {
                return ERROR_PERMISSION_DENIED;
            } else {
                return ERROR_CANNOT_OPEN_FILE;
            }
        }

        // get the size
        struct stat info;
        if (stat(name, &info) == 0) {
            size = info.st_size;
        }
    }

    stream = new AndroidFileByteStream(delegator, fd, size);
    return SUCCESS;
}

AndroidFileByteStream::AndroidFileByteStream(FileByteStream* delegator, int fd, LargeSize size) :
        delegator(delegator),
        referenceCount(1),
        fd(fd),
        position(0),
        size(size) { }

AndroidFileByteStream::~AndroidFileByteStream() {
    if (fd && fd != STDERR_FILENO && fd != STDOUT_FILENO && fd != STDERR_FILENO) {
        close(fd);
    }
}

void AndroidFileByteStream::addReference() {
    referenceCount++;
}

void AndroidFileByteStream::release() {
    if (--referenceCount == 0) {
        if (delegator) {
            delete delegator;
        } else {
            delete this;
        }
    }
}

Result AndroidFileByteStream::readPartial(void* buffer, Size bytes_to_read, Size& bytes_read) {
    ssize_t nb_read = android::readPartial(fd, buffer, bytes_to_read);
    if (nb_read > 0) {
        bytes_read = (Size)nb_read;
        position += nb_read;
        return SUCCESS;
    } else if (nb_read == 0) {
        bytes_read = 0;
        return ERROR_EOS;
    } else {
        bytes_read = 0;
        return ERROR_READ_FAILED;
    }
}

Result AndroidFileByteStream::writePartial(const void* buffer, Size bytes_to_write, Size& bytes_written) {
    if (bytes_to_write == 0) {
        bytes_written = 0;
        return SUCCESS;
    }
    ssize_t nb_written = android::writePartial(fd, buffer, bytes_to_write);

    if (nb_written > 0) {
        bytes_written = (Size)nb_written;
        position += nb_written;
        return SUCCESS;
    } else {
        bytes_written = 0;
        return ERROR_WRITE_FAILED;
    }
}

Result AndroidFileByteStream::seek(Position position) {
    // shortcut
    if (position == this->position) return SUCCESS;

    off64_t result = lseek64(fd, position, SEEK_SET);
    if (result >= 0) {
        this->position = position;
        return SUCCESS;
    } else {
        return FAILURE;
    }
}

Result AndroidFileByteStream::tell(Position& position) {
    position = position;
    return SUCCESS;
}

Result AndroidFileByteStream::getSize(LargeSize& size) {
    size = this->size;
    return SUCCESS;
}

Result AndroidFileByteStream::flush() {
    return SUCCESS;
}

Result FileByteStream::create(const char* name, FileByteStream::Mode mode, ByteStream*& stream) {
    return AndroidFileByteStream::create(NULL, name, mode, stream);
}

FileByteStream::FileByteStream(const char* name, FileByteStream::Mode mode) {
    ByteStream* stream = nullptr;
    Result result = AndroidFileByteStream::create(this, name, mode, stream);
    if (FAILED(result)) {
        return;
    }

    delegate = stream;
}

}