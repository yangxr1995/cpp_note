#include "Buffer.h"
#include <sys/uio.h>
#include <bits/types/struct_iovec.h>
#include <sys/types.h>
#include <unistd.h>

ssize_t Buffer::readFd(int fd, int *saveErrno) {
    char extrabuf[65536] = {0};
    struct iovec vec[2];
    const size_t writeable = writeableBytes();
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writeable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);
     
    const int iovcnt = writeable < sizeof(extrabuf) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0) {
        *saveErrno = errno;
    }
    else if (n <= writeable) {
        writerIndex_ += n;
    }
    else {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writeable);
    }

    return n;
}

ssize_t Buffer::writeFd(int fd, int *saveErrno) {
    ssize_t n = ::write(fd, peek(), readableBytes());
    if (n < 0) {
        *saveErrno = errno;
    }
    return n;
}
