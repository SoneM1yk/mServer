#include "Buffer.h"
#include <sys/uio.h>

ssize_t Buffer::readFd(int sock)
{
    char extrabuf[65536];
    iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;
    const ssize_t n = readv(sock, vec, 2);
    if (n < 0) {

    } else if (static_cast<size_t>(n) < writable) {
        writerIndex_ += n;
    } else {
        writerIndex_ = buffer_.size();
        append(extrabuf, n-writableBytes());
    }
    return n;
}