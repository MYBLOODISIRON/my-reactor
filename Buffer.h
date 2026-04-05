#pragma once
#include <vector>
#include <string>

class Buffer    // 网络缓冲区
{
    public:
        static const size_t sm_kCheapPrepend    {8};
        static const size_t sm_kInitialSize     {1024};
    private:
        std::vector< char > m_buffer;
        size_t              m_readIndex;
        size_t              m_writeIndex;
    public:

        explicit    Buffer  (size_t initialSize = sm_kInitialSize);
        ~Buffer () = default;

        size_t  readableBytes   () const;
        size_t  writableBytes   () const;
        size_t  prependableBytes    () const;
        const char* peek            () const;
        void        retrieve         (size_t len);
        void        retrieveAll      ();
        std::string retrieveAllAsString ();
        std::string retrieveAsString    (size_t len);
        void        ensureWritableBytes (size_t len);
        void        append              (const char* data, size_t len);
        char*       beginWrite          ();
        const char* beginWrite          () const;
        ssize_t     readFd              (int fd, int* saveErrno);   // 从fd上读取数据
        ssize_t     writeFd             (int fd, int* saveErrno);
    private:
        const char* begin   ();
        void        makeSpace   (size_t len);
};