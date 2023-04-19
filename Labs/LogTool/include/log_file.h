/**
* @File log_file.h
* @Date 2023-04-19
* @Description 
* @Author Ticks
* @Email ticks.cc\@gmail.com
*
* Copyright 2023 Ticks, Inc. All rights reserved. 
**/
#ifndef __LINUX_STUDY_LOG_TOOL_LOG_FILE_H
#define __LINUX_STUDY_LOG_TOOL_LOG_FILE_H

#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>

class LogFile
{
public:
    static const int RETRY_COUNT = 3;

    explicit LogFile(const std::string& name) noexcept
        : _name(std::move(name + ".log"))
    {
        openFile();
    }

    LogFile(const LogFile&) = delete;
    LogFile& operator = (const LogFile&) = delete;

    LogFile(LogFile&&) = default;
    LogFile& operator = (LogFile&&) = default;

    virtual ~LogFile() {
        closeLogFile();
    }

public:
    void setMaxBufSize(size_t size) noexcept {
        if (size != _max_buf_size) {
            _max_buf_size = size;
        }
    }

    virtual size_t pushContent(const std::string& content);

    size_t fileSize() const {
        if (_fd < 0) {
            return 0;
        }
        struct stat64 s64{};
        ::fstat64(_fd, &s64);
        return s64.st_size;
    }

protected:
    size_t closeLogFile() {
        printf("close\n");
        if (_fd < 0) {
            return 0;
        }
        size_t len = 0;
        if (!_buf.empty()) {
            len = writeContent(_buf);
            _buf.erase(0, len);
        }
        ::syncfs(_fd);
        ::close(_fd);
        return len;
    }

    size_t writeContent(const std::string& content) const;
    bool openFile();
    void reopenFile();

protected:
    int _fd {-1};                   // 日志文件描述符
    size_t _max_buf_size{512};      // 缓冲区最大大小
    std::string _name;              // 日志文件名或者路径
    std::string _buf;               // 日志内容缓冲区

}; // LogFile

class RollLogFile final : protected LogFile
{
public:
    explicit RollLogFile(const std::string& base_name)
        : LogFile(base_name + std::to_string(1))
    {  }
    explicit RollLogFile(const std::string& base_name, size_t max_size)
        : LogFile(base_name + std::to_string(1))
        , _max_file_size(max_size)
    {  }
    explicit RollLogFile(const std::string& base_name, size_t roll_start, size_t max_size)
        : LogFile(base_name + std::to_string(roll_start))
        , _max_file_size(max_size)
    {  }

    ~RollLogFile() override = default;

public:
    size_t pushContent(const std::string& content) override;

    void setMaxFileSize(size_t size) {
        _max_file_size = size;
    }

private:
    size_t rollFile();

private:
    size_t _max_file_size{512};     // 日志文件最大大小
    size_t _roll_count{1};          // 滚动次数

}; // RollLogFile

#endif // __LINUX_STUDY_LOG_TOOL_LOG_FILE_H
