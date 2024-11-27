#pragma once

#include <unistd.h>  // For close()

#include <stdexcept>  // For std::runtime_error

class NonCopyableNonMovable {
   public:
    NonCopyableNonMovable() = default;
    ~NonCopyableNonMovable() = default;

    NonCopyableNonMovable(const NonCopyableNonMovable&) = delete;
    NonCopyableNonMovable& operator=(const NonCopyableNonMovable&) = delete;

    NonCopyableNonMovable(NonCopyableNonMovable&&) = delete;
    NonCopyableNonMovable& operator=(NonCopyableNonMovable&&) = delete;
};

class FileDescriptor : private NonCopyableNonMovable {
   private:
    int fd;  // File descriptor

   public:
    explicit FileDescriptor(int fd) : fd(fd) {
        if (fd < 0) {
            throw std::runtime_error("Invalid file descriptor");
        }
    };

    ~FileDescriptor() {
        closeIfNeeded();
    };

    void close() {
        closeIfNeeded();
        fd = -1;
    };

    int getFd() const {
        return fd;
    };

   private:
    void closeIfNeeded() {
        if (fd >= 0) {
            ::close(fd);
        }
    };
};
