#include <MemoryAudioFile.h>

#include <cstring>
#include <algorithm>
#include <stdexcept>

MemoryAudioFile::MemoryAudioFile(
    const void* data,
    size_t size)
{
    memory_.data = static_cast<const char*>(data);
    memory_.size = static_cast<sf_count_t>(size);
    memory_.pos = 0;

    io_.get_filelen = getFileLength;
    io_.seek = seek;
    io_.read = read;
    io_.write = write;
    io_.tell = tell;
}

SNDFILE* MemoryAudioFile::open(SF_INFO& info)
{
    SNDFILE* file =
        sf_open_virtual(
            &io_,
            SFM_READ,
            &info,
            &memory_);

    if (!file)
        throw std::runtime_error(sf_strerror(nullptr));

    return file;
}

sf_count_t MemoryAudioFile::getFileLength(void* userData)
{
    auto* mem =
        static_cast<MemoryFile*>(userData);

    return mem->size;
}

sf_count_t MemoryAudioFile::tell(void* userData)
{
    auto* mem =
        static_cast<MemoryFile*>(userData);

    return mem->pos;
}

sf_count_t MemoryAudioFile::read(
    void* ptr,
    sf_count_t count,
    void* userData)
{
    auto* mem =
        static_cast<MemoryFile*>(userData);

    sf_count_t remaining =
        mem->size - mem->pos;

    count = std::min(count, remaining);

    memcpy(
        ptr,
        mem->data + mem->pos,
        count);

    mem->pos += count;

    return count;
}

sf_count_t MemoryAudioFile::seek(
    sf_count_t offset,
    int whence,
    void* userData)
{
    auto* mem =
        static_cast<MemoryFile*>(userData);

    sf_count_t newPos = 0;

    switch (whence)
    {
        case SEEK_SET:
            newPos = offset;
            break;

        case SEEK_CUR:
            newPos = mem->pos + offset;
            break;

        case SEEK_END:
            newPos = mem->size + offset;
            break;

        default:
            return -1;
    }

    newPos =
        std::clamp(
            newPos,
            (sf_count_t)0,
            mem->size);

    mem->pos = newPos;

    return mem->pos;
}

sf_count_t MemoryAudioFile::write(
    const void*,
    sf_count_t,
    void*)
{
    return 0;
}