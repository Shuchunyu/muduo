#include "Buffer.h"

#include<errno.h>
#include<sys/uio.h>
#include<unistd.h>

const char Buffer::kCRLF[] = "\r\n";

//从fd上读取数据
ssize_t Buffer::readFd(int fd, int* savedErrno)
{
    char extrabuf[65536] = {0};
    struct iovec vec[2];
    const size_t writeable = writableBytes();
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writeable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;

    const int iovcnt = (writeable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = readv(fd, vec, iovcnt);

    if(n < 0)
    {
        *savedErrno = errno;
    }
    else if (n <= writeable)
    {
        writerIndex_ += n;
    }
    else
    {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writeable);
    }

    return n;
}

ssize_t Buffer::writeFd(int fd, int* savedErrno)
{
    ssize_t n = ::write(fd, peek(), readableBytes());

    if(n < 0)
    {
        *savedErrno = errno;
    }
    return n;
}