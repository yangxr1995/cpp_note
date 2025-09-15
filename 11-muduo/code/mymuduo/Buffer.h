#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

class Buffer {
    public:
        static const size_t kCheapPrepend = 8;
        static const size_t kInitialSize = 1024;

        explicit Buffer(size_t initialSize = kInitialSize)
            : buffer_(kCheapPrepend + initialSize)
              , readerIndex_(kCheapPrepend)
              , writerIndex_(kCheapPrepend)
        {}

        size_t readableBytes() const { return writerIndex_ - readerIndex_; }
        size_t writeableBytes() const { return buffer_.size() - writerIndex_; }
        size_t prependableBytes() const { return readerIndex_; }

        const char *peek() const {
            return begin() + readerIndex_;
        }

        void retrieve(size_t len) {
            if (len < readableBytes()) {
                readerIndex_ += len;
            }
            else {
                retrieveAll();
            }
        }

        void retrieveAll() {
            readerIndex_ = writerIndex_ = kCheapPrepend;
        }

        std::string retrieveAllAsString() {
            return retrieveAsString(readableBytes());
        }

        std::string retrieveAsString(size_t len) {
            std::string result(peek(), len);
            retrieve(len);
            return result;
        }

        void makeSpace(size_t len) {
            if (writeableBytes() + prependableBytes() < len + kCheapPrepend) {
                buffer_.resize(writerIndex_ + len);
            }
            else {
                size_t readable = readableBytes();
                std::copy(begin() + readerIndex_,
                        begin() + writerIndex_,
                        begin() + kCheapPrepend);
                readerIndex_ = kCheapPrepend;
                writerIndex_ = readerIndex_ + readable;
            }
        }

        void append(const char *data, size_t len) {
            ensureWriteableBytes(len);
            std::copy(data, data + len, beginWrite());
            hasWritten(len);
        }

        void hasWritten(size_t len) {
            writerIndex_ += len;
        }

        ssize_t readFd(int fd, int *saveErrno);
        ssize_t writeFd(int fd, int *saveErrno);

        char *beginWrite() {
            return begin() + writerIndex_;
        }

        const char *beginWrite() const {
            return begin() + writerIndex_;
        }

        void ensureWriteableBytes(size_t len) {
            if (writeableBytes() < len) {
                makeSpace(len);
            }
        }


    private:

        char *begin() {
            return &*buffer_.begin();
        }

        const char *begin() const {
            return &*buffer_.begin();
        }


        std::vector<char> buffer_;
        size_t readerIndex_;
        size_t writerIndex_;
};
