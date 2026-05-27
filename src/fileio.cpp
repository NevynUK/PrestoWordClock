/**
 * @file fileio.cpp
 * @brief FatFS-backed file I/O implementation for the af-file-io interface.
 *
 * Wraps the FatFS (ff.h) API to provide the opaque file-handle interface
 * expected by the audio/graphics library (af-file-io.h).
 */
#include "af-file-io.h"
#include "ff.h"
#include "stdint.h"
#include "string.h"
#include "stdio.h"
#include <stdlib.h>

/**
 * @brief Opens a file for reading.
 *
 * @param filename  Path to the file on the FatFS volume.
 * @return          Opaque file handle on success, @c NULL on failure.
 */
void *fileio_open(const char *filename)
{
    printf("IMPL: Opening %s\n", filename);
    FIL *fil = (FIL *) malloc(sizeof(FIL));
    FRESULT fr = f_open(fil, filename, FA_READ);
    if (fr == FR_OK)
    {
        return (void *) fil;
    }
    else
    {
        free(fil);
        return NULL;
    }
}

/**
 * @brief Closes a file and releases its handle.
 *
 * @param fhandle  Handle returned by fileio_open().
 */
void fileio_close(void *fhandle)
{
    f_close((FIL *) fhandle);
    free(fhandle);
}

/**
 * @brief Reads bytes from the current file position.
 *
 * @param fhandle  Handle returned by fileio_open().
 * @param buf      Destination buffer.
 * @param len      Number of bytes to read.
 * @return         Number of bytes actually read, or 0 on error.
 */
size_t fileio_read(void *fhandle, void *buf, size_t len)
{
    unsigned int bytes_read;
    FRESULT fr = f_read((FIL *) fhandle, buf, len, &bytes_read);
    return fr == FR_OK ? bytes_read : 0;
}

/**
 * @brief Reads a single byte from the current file position.
 *
 * @param fhandle  Handle returned by fileio_open().
 * @return         The byte read, cast to @c int.
 */
int fileio_getc(void *fhandle)
{
    unsigned char buf;
    (void *) fileio_read(fhandle, (void *) &buf, (size_t) 1);
    return (int) buf;
}

/**
 * @brief Returns the current file position.
 *
 * @param fhandle  Handle returned by fileio_open().
 * @return         Current byte offset from the start of the file.
 */
size_t fileio_tell(void *fhandle)
{
    return f_tell((FIL *) fhandle);
}

/**
 * @brief Moves the file position to an absolute byte offset.
 *
 * @param fhandle  Handle returned by fileio_open().
 * @param pos      Target byte offset from the start of the file.
 * @return         The resulting file position after seeking.
 */
size_t fileio_seek(void *fhandle, size_t pos)
{
    f_lseek((FIL *) fhandle, pos);
    return f_tell((FIL *) fhandle);
}