#pragma once

#include <sndfile.h>
#include <cstddef>

class MemoryAudioFile
{
public:

    MemoryAudioFile(const void* data, size_t size);

    SNDFILE* open(SF_INFO& info);

private:

    struct MemoryFile
    {
        const char* data;
        sf_count_t size;
        sf_count_t pos;
    };

    MemoryFile memory_;
    SF_VIRTUAL_IO io_;
    static sf_count_t getFileLength(void* userData);
    static sf_count_t seek(
        sf_count_t offset,
        int whence,
        void* userData);

    static sf_count_t read(
        void* ptr,
        sf_count_t count,
        void* userData);

    static sf_count_t write(
        const void*,
        sf_count_t,
        void*);

    static sf_count_t tell(void* userData);
};