/**
* @File file_tool.cc
* @Date 2023-04-06
* @Description 
* @Author Ticks
* @Email ticks.cc\@gmail.com
*
* Copyright 2023 Ticks, Inc. All rights reserved. 
**/

#include "file_tool.h"

bool File::_open(int _flags, int _mode) {
    // 设置默认打开模式
    _flags |= kDefaultOpenFlags;
    if ((_flags & kReadOnly) && (_flags & kWriteOnly)) {
        _flags &= ~kWriteOnly;
        _flags &= ~kReadWrite;
    }
    // append 和 trunc 同在是保留 trunc
    if ((_flags & kAppend) && (_flags & kTrunc)) {
        _flags &= ~kAppend;
    }

    int fd;
    if (_mode == -1) {
        fd = ::open(this->_file_name.data(), _flags);
    } else {
        fd = ::open(this->_file_name.data(), _flags, _mode);
    }
    if (fd < 0) {
        this->_error = errno;
        return false;
    }
    this->_open_flags = _flags;
    this->_fd = fd;
    return true;
}

// 按文件权限和目录权限创建文件
File File::create(const std::string &_path, int _file_mode, bool _is_recursion, int _dir_mode)
{
    if (_is_recursion) {
        std::string::size_type index, start = 0;
        std::string tmp;
        mode_t u_mask = 0;
        if (_dir_mode == 0) {
            u_mask = umask(0);
            _dir_mode = 0777;
        }
        while ((index = _path.find_first_of('/', start)) != std::string::npos) {
            tmp = _path.substr(0, index + 1);
            if (!File::isExist(tmp.data())) {
                if (mkdir(tmp.data(), _dir_mode & (~u_mask)) != 0) {
                    fprintf(stderr, "mkdir dir[%s] error!\n", tmp.data());
                }
            }
            start = index + 1;
        }
    }
    return File(_path, (kCreateForce | kDefaultOpenFlags), _file_mode);
}

File File::createIfNotExist(const std::string &_path, int _file_mode, bool _is_recursion, int _dir_mode)
{
    if (_is_recursion) {
        std::string::size_type index, start = 0;
        std::string tmp;
        mode_t u_mask = 0;
        if (_dir_mode == 0) {
            u_mask = umask(0);
            _dir_mode = 0777;
        }
        while ((index = _path.find_first_of('/', start)) != std::string::npos) {
            tmp = _path.substr(0, index + 1);
            if (!File::isExist(tmp.data())) {
                if (mkdir(tmp.data(), _dir_mode & (~u_mask)) != 0) {
                    fprintf(stderr, "mkdir dir[%s] error!\n", tmp.data());
                }
            }
            start = index + 1;
        }
    }
    return File(_path, (kCreateNotExist | kDefaultOpenFlags), _file_mode);
}

File File::open(const std::string& _path, int _flags) {
    return File(_path, _flags);
}



size_t File::readBytes(bytePtr _buf, size_t _count) const
{
    int retry_count = 0;
    size_t total_read_len = 0, read_len;
    while (retry_count < kMaxRetryCount && total_read_len < _count) {
        read_len = ::read(this->_fd, _buf + total_read_len, _count - total_read_len);
        if (read_len > 0) {
            total_read_len += read_len;
        }else {
            if (read_len == 0) {
                break;
            }
            // 信号中断
            if (errno != EINTR) {
                ++retry_count;
            }
        }
    }
    return total_read_len;
}

size_t File::writeBytes(constBytePtr _content, size_t _count) const
{
    int retry_count = 0;
    size_t total_read_len = 0, read_len;
    while (retry_count < kMaxRetryCount && total_read_len < _count) {
        read_len = ::write(this->_fd, _content + total_read_len, _count - total_read_len);
        if (read_len > 0) {
            total_read_len += read_len;
        }else {
            // 信号中断
            if (errno != EINTR) {
                ++retry_count;
            }
        }
    }
    return total_read_len;
}

// 非多线程安全！！
size_t File::multiReadBytes(bytePtr _buf, size_t _start, size_t _count) const
{
    auto seek = ::lseek64(this->_fd, 0, SEEK_CUR);
    ::lseek64(this->_fd, off64_t(_start), SEEK_SET);
    auto len = this->readBytes(_buf, _count);
    ::lseek64(this->_fd, seek, SEEK_SET);
    return len;
}

size_t File::multiWriteBytes(constBytePtr _content, size_t _start, size_t _count) const
{
    auto seek = ::lseek64(this->_fd, 0, SEEK_CUR);
    ::lseek64(this->_fd, off64_t(_start), SEEK_SET);
    auto len = this->writeBytes(_content, _count);
    ::lseek64(this->_fd, seek, SEEK_SET);
    return len;
}

auto FileReader::readString() const -> std::string
{
    return this->readString(this->_file.size());
}

auto FileReader::readVec() const -> std::vector<File::byte>
{
    return this->readVec(this->_file.size());
}

auto FileReader::readStringLine() const -> std::string
{
    File::byte buf[32];
    size_t read_len = 1;
    std::string content;
    content.reserve(32);
    while (read_len != 0) {
        read_len = this->_file.readBytes(buf, 32);
        for (int i = 0; i < read_len; ++i) {
            if (buf[i] != '\n') {
                content.push_back((char)buf[i]);
            }else {
                ::lseek64(this->_file._fd, -(31 - i), SEEK_CUR);
                return content;
            }
        }
    }
    return content;
}

auto FileReader::readVecLine() const -> std::vector<File::byte>
{
    File::byte buf[32];
    size_t read_len = 1;
    std::vector<File::byte> content;
    content.resize(32);
    while (read_len != 0) {
        read_len = this->_file.readBytes(buf, 32);
        for (int i = 0; i < 32; ++i) {
            if (buf[i] != '\n') {
                content.push_back((char)buf[i]);
            }else {
                ::lseek64(this->_file._fd, -(31 - i), SEEK_CUR);
                return content;
            }
        }
    }
    return content;
}

auto FileReader::readString(size_t _len) const -> std::string
{
    char buf[128];
    size_t read_len = 1, total_read_len = 0;
    std::string content;
    content.reserve(_len);
    while (read_len != 0) {
        read_len = this->_file.readBytes((File::bytePtr)buf, std::min((size_t)128, (_len - total_read_len)));
        content += buf;
    }
    return content;
}

auto FileReader::readVec(size_t _len) const -> std::vector<File::byte>
{
    size_t read_len = 1, total_read_len = 0;
    std::vector<File::byte> content;
    content.resize(_len);
    while (read_len != 0) {
        read_len = this->_file.readBytes(File::bytePtr(content.data() + total_read_len), _len - total_read_len);
        total_read_len += read_len;
    }
    return content;
}

size_t FileWriter::write(const std::string &_content)
{
    return this->write(_content, _content.size());
}

size_t FileWriter::write(const std::string &_content, size_t _count)
{
    return this->_file.writeBytes((File::constBytePtr)(_content.data()), _count);
}

size_t FileWriter::write(const std::vector<File::byte> &_content)
{
    return this->write(_content, _content.size());
}

size_t FileWriter::write(const std::vector<File::byte> &_content, size_t _count)
{
    return this->_file.writeBytes((File::constBytePtr)(_content.data()), _count);
}
