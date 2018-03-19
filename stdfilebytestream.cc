//
// Created by wlanjie on 2018/3/7.
//

#define _LARGEFILE_SOURCE
#define _LARGEFILE_SOURCE64
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <string.h>
#if !defined(_WIN32_WCE)
#include <errno.h>
#include <sys/stat.h>
#endif

#include "filebytestream.h"

/*----------------------------------------------------------------------
|   compatibility wrappers
+---------------------------------------------------------------------*/
#if !defined(ENOENT)
#define ENOENT 2
#endif
#if !defined(EACCES)
#define EACCES 13
#endif

#if !defined(CONFIG_HAVE_FOPEN_S)
static int fopen_s(FILE**      file,
                   const char* filename,
                   const char* mode)
{
    *file = fopen(filename, mode);
#if defined(UNDER_CE)
    if (*file == NULL) return ENOENT;
#else
    if (*file == NULL) return errno;
#endif
    return 0;
}
#endif // defined(CONFIG_HAVE_FOPEN_S

/*----------------------------------------------------------------------
|   StdcFileByteStream
+---------------------------------------------------------------------*/
class StdcFileByteStream: public ByteStream
{
public:
    // class methods
    static Result Create(FileByteStream*      delegator,
                             const char*              name,
                             FileByteStream::Mode mode,
                             ByteStream*&         stream);

    // methods
    StdcFileByteStream(FileByteStream* delegator,
                           FILE*               file,
                           LargeSize       size);

    ~StdcFileByteStream();

    // ByteStream methods
    Result readPartial(void*     buffer,
                           Size  bytesToRead,
                           Size& bytesRead);
    Result writePartial(const void* buffer,
                            Size    bytesToWrite,
                            Size&   bytesWritten);
    Result seek(Position position);
    Result tell(Position& position);
    Result getSize(LargeSize& size);

    // Referenceable methods
    void addReference();
    void release();

private:
    // members
    ByteStream* m_Delegator;
    Cardinal    m_ReferenceCount;
    FILE*           m_File;
    Position    m_Position;
    LargeSize   m_Size;
};

/*----------------------------------------------------------------------
|   StdcFileByteStream::Create
+---------------------------------------------------------------------*/
Result
StdcFileByteStream::Create(FileByteStream*      delegator,
                               const char*              name,
                               FileByteStream::Mode mode,
                               ByteStream*&         stream)
{
    // default value
    stream = NULL;

    // check arguments
    if (name == NULL) return ERROR_INVALID_PARAMETERS;

    // open the file
    FILE* file = NULL;
    Position size = 0;
    if (!strcmp(name, "-stdin")) {
        file = stdin;
    } else if (!strcmp(name, "-stdout")) {
        file = stdout;
    } else if (!strcmp(name, "-stderr")) {
        file = stderr;
    } else {
        int open_result;
        switch (mode) {
            case FileByteStream::STREAM_MODE_READ:
                open_result = fopen_s(&file, name, "rb");
                break;

            case FileByteStream::STREAM_MODE_WRITE:
                open_result = fopen_s(&file, name, "wb+");
                break;

            case FileByteStream::STREAM_MODE_READ_WRITE:
                open_result = fopen_s(&file, name, "r+b");
                break;

            default:
                return ERROR_INVALID_PARAMETERS;
        }

        if (open_result != 0) {
            if (open_result == ENOENT) {
                return ERROR_NO_SUCH_FILE;
            } else if (open_result == EACCES) {
                return ERROR_PERMISSION_DENIED;
            } else {
                return ERROR_CANNOT_OPEN_FILE;
            }
        }

        // get the size
        if (fseek(file, 0, SEEK_END) >= 0) {
            size = ftell(file);
            fseek(file, 0, SEEK_SET);
        }

    }

    stream = new StdcFileByteStream(delegator, file, size);
    return SUCCESS;
}

/*----------------------------------------------------------------------
|   StdcFileByteStream::StdcFileByteStream
+---------------------------------------------------------------------*/
StdcFileByteStream::StdcFileByteStream(FileByteStream* delegator,
                                               FILE*               file,
                                               LargeSize       size) :
        m_Delegator(delegator),
        m_ReferenceCount(1),
        m_File(file),
        m_Position(0),
        m_Size(size)
{
}

/*----------------------------------------------------------------------
|   StdcFileByteStream::~StdcFileByteStream
+---------------------------------------------------------------------*/
StdcFileByteStream::~StdcFileByteStream()
{
    if (m_File && m_File != stdin && m_File != stdout && m_File != stderr) {
        fclose(m_File);
    }
}

/*----------------------------------------------------------------------
|   StdcFileByteStream::AddReference
+---------------------------------------------------------------------*/
void
StdcFileByteStream::addReference()
{
    m_ReferenceCount++;
}

/*----------------------------------------------------------------------
|   StdcFileByteStream::Release
+---------------------------------------------------------------------*/
void
StdcFileByteStream::release()
{
    if (--m_ReferenceCount == 0) {
        if (m_Delegator) {
            delete m_Delegator;
        } else {
            delete this;
        }
    }
}

/*----------------------------------------------------------------------
|   StdcFileByteStream::ReadPartial
+---------------------------------------------------------------------*/
Result
StdcFileByteStream::readPartial(void*     buffer,
                                    Size  bytesToRead,
                                    Size& bytesRead)
{
    size_t nbRead;

    nbRead = fread(buffer, 1, bytesToRead, m_File);

    if (nbRead > 0) {
        bytesRead = (Size)nbRead;
        m_Position += nbRead;
        return SUCCESS;
    } else if (feof(m_File)) {
        bytesRead = 0;
        return ERROR_EOS;
    } else {
        bytesRead = 0;
        return ERROR_READ_FAILED;
    }
}

/*----------------------------------------------------------------------
|   StdcFileByteStream::WritePartial
+---------------------------------------------------------------------*/
Result
StdcFileByteStream::writePartial(const void* buffer,
                                     Size    bytesToWrite,
                                     Size&   bytesWritten)
{
    size_t nbWritten;

    if (bytesToWrite == 0) return SUCCESS;
    nbWritten = fwrite(buffer, 1, bytesToWrite, m_File);

    if (nbWritten > 0) {
        bytesWritten = (Size)nbWritten;
        m_Position += nbWritten;
        return SUCCESS;
    } else {
        bytesWritten = 0;
        return ERROR_WRITE_FAILED;
    }
}

/*----------------------------------------------------------------------
|   StdcFileByteStream::Seek
+---------------------------------------------------------------------*/
Result
StdcFileByteStream::seek(Position position)
{
    // shortcut
    if (position == m_Position) return SUCCESS;

    size_t result;
    result = fseek(m_File, position, SEEK_SET);
    if (result == 0) {
        m_Position = position;
        return SUCCESS;
    } else {
        return FAILURE;
    }
}

/*----------------------------------------------------------------------
|   StdcFileByteStream::Tell
+---------------------------------------------------------------------*/
Result
StdcFileByteStream::tell(Position& position)
{
    position = m_Position;
    return SUCCESS;
}

/*----------------------------------------------------------------------
|   StdcFileByteStream::GetSize
+---------------------------------------------------------------------*/
Result
StdcFileByteStream::getSize(LargeSize& size)
{
    size = m_Size;
    return SUCCESS;
}

/*----------------------------------------------------------------------
|   FileByteStream::Create
+---------------------------------------------------------------------*/
Result
FileByteStream::create(const char*              name,
                           FileByteStream::Mode mode,
                           ByteStream*&         stream)
{
    return StdcFileByteStream::Create(NULL, name, mode, stream);
}

#if !defined(CONFIG_NO_EXCEPTIONS)
/*----------------------------------------------------------------------
|   FileByteStream::FileByteStream
+---------------------------------------------------------------------*/
FileByteStream::FileByteStream(const char*              name,
                                       FileByteStream::Mode mode)
{
    ByteStream* stream = NULL;
    Result result = StdcFileByteStream::Create(this, name, mode, stream);
    if (FAILED(result)) throw Exception(result);

    delegate = stream;
}
#endif