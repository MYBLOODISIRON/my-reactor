#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>
#include <algorithm>
#include "Buffer.h"

Buffer::Buffer(size_t initialSize)
:   m_buffer    (sm_kCheapPrepend + initialSize)
{

}


size_t Buffer::readableBytes() const
{
    return m_writeIndex - m_readIndex;
}

size_t Buffer::writableBytes() const
{
    return m_buffer.size() - m_writeIndex;
}

size_t Buffer::prependableBytes() const
{
    return m_readIndex;
}

const char* Buffer::begin() const
{
    return m_buffer.data();
}


const char* Buffer::peek() const
{
    return begin() + m_readIndex;
}


void Buffer::retrieve(size_t len)
{
    if(len < readableBytes())
    {
        m_readIndex += len;
    }
    else
    {
        retrieveAll();
    }
}

void Buffer::retrieveAll()
{
    m_readIndex = m_writeIndex = sm_kCheapPrepend;
}

std::string Buffer::retrieveAllAsString()
{
    return retrieveAsString(readableBytes());
}

std::string Buffer::retrieveAsString(size_t len)
{
    std::string result {peek(), len};
    retrieve(len);
    return result;
}

void Buffer::ensureWritableBytes(size_t len)
{
    if(writableBytes() < len)
    {
        makeSpace(len);
    }
}

void Buffer::makeSpace(size_t len)
{
    if(readableBytes() + prependableBytes() < len + sm_kCheapPrepend)
    {
        m_buffer.resize(m_writeIndex + len);
    }
    else
    {
        size_t readable = readableBytes();
        std::copy(begin() + m_readIndex, begin()+ m_writeIndex, begin() + sm_kCheapPrepend);
        m_readIndex = sm_kCheapPrepend;
        m_writeIndex = m_readIndex + readable;
    }
}


const char* Buffer::beginWrite() const
{
    return begin() + m_writeIndex;
}

void Buffer::append(const char* data, size_t len)
{
    ensureWritableBytes(len);
    std::copy(data, data + len, beginWrite());
    m_writeIndex += len;
}

ssize_t Buffer::readFd(int fd, int* saveErrno)
{
    char extrabuf [65535];
    struct iovec vec [2];
    const size_t writable = writableBytes();
    vec[0].iov_base = (void*) (begin() + m_writeIndex);
    vec[1].iov_base = extrabuf;
    vec[0].iov_len = writable;
    vec[1].iov_len = sizeof extrabuf;

    const int iov_count = (writable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iov_count);
    if(n < 0)
    {
        *saveErrno = errno;
    }
    else if (n <= writable)
    {
        m_writeIndex += n;
    }
    else
    {
        m_writeIndex = m_buffer.size();
        append(extrabuf, n - writable);
    }
    return n; 
}

ssize_t Buffer::writeFd(int fd, int* saveErrno)
{
    ssize_t n = ::write(fd, peek(), readableBytes());
    if(n < 0)
    {
        *saveErrno = errno;
    }
    return n;
}