/**
* @File log_file.cc
* @Date 2023-04-19
* @Description 
* @Author Ticks
* @Email ticks.cc\@gmail.com
*
* Copyright 2023 Ticks, Inc. All rights reserved. 
**/

#include "log_file.h"

size_t LogFile::pushContent(const std::string &content)
{
    size_t write_len, total_len = 0;
    if (_buf.size() + content.size() >= _max_buf_size) {
        write_len = writeContent(_buf);
        total_len += write_len;
        if (write_len == _buf.size()) {
            _buf.clear();
            write_len = writeContent(content);
            total_len += write_len;
            if (write_len != content.size()) {
                _buf.append(content, write_len);
            }
        } else {
            _buf.erase(0, write_len);
            _buf += content;
        }
    }else {
        _buf += content;
        return 0;
    }
    return total_len;
}

size_t LogFile::writeContent(const std::string &content) const
{
    if (_fd < 0 || content.empty()) {
        return 0;
    }
    const char* data = content.c_str();
    int error_count = 0;
    size_t write_len = 0, content_size = content.size();
    ssize_t len;

    while (error_count < RETRY_COUNT && write_len < content_size) {
        len = ::write(_fd, (data + write_len), content_size - write_len);
        if (len > 0) {
            write_len += len;
        } else {
            if (errno != EINTR) {
                error_count += 1;
            }
        }
    }
    /*
    if (error_count == RETRY_COUNT) {
        perror("write file content error");
    }
    */

    return write_len;
}

bool LogFile::openFile()
{
    std::string::size_type index, start = 0;
    std::string tmp;
    mode_t u_mask = umask(0);
    int dir_mode = 0777;

    while ((index = _name.find_first_of('/', start)) != std::string::npos) {
        tmp = _name.substr(0, index + 1);
        if (::access(tmp.data(), F_OK) != 0) {
            if (mkdir(tmp.data(), dir_mode & (~u_mask)) != 0) {
                return false;
            }
        }
        start = index + 1;
    }

    _fd = ::open(_name.data(), O_APPEND | O_CREAT | O_WRONLY);

    return _fd >= 0;
}

void LogFile::reopenFile()
{
    closeLogFile();
    openFile();
}

size_t RollLogFile::rollFile()
{
    auto count = std::to_string(_roll_count);
    _roll_count += 1;
    auto pos = LogFile::_name.find_last_of(count);
    if (pos != std::string::npos) {
        LogFile::_name.replace(pos, count.size(), std::to_string(_roll_count));
    }else {
        LogFile::_name += std::to_string(_roll_count);
    }
    size_t len = LogFile::closeLogFile();
    LogFile::openFile();
    return len;
}

size_t RollLogFile::pushContent(const std::string &content)
{
    size_t len = LogFile::pushContent(content);
    if (LogFile::fileSize() >= _max_file_size) {
        len += rollFile();
    }
    return len;
}
