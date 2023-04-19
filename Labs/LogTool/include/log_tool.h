/**
* @File log_tool.h
* @Date 2023-04-19
* @Description 日志文件工具
* @Author Ticks
* @Email ticks.cc\@gmail.com
*
* Copyright 2023 Ticks, Inc. All rights reserved. 
**/
#ifndef __LINUX_STUDY_LOG_TOOL_LOG_TOOL_H
#define __LINUX_STUDY_LOG_TOOL_LOG_TOOL_H

#include <atomic>
#include "log_file.h"

struct LogLinkNode
{
    std::string msg;
    LogLinkNode* prv{nullptr};
    LogLinkNode* next{nullptr};

}; // LogLinkNode

class LogLink
{
public:
    static bool exchange(LogLink& link1, LogLink& lin2);

    LogLink() = default;
    ~LogLink() {
        clearNode();
    };

    void pushLogMsg(const std::string& msg);
    LogLinkNode* popLogMsg();

    const LogLinkNode* head() const noexcept {
        return this->_head;
    }
    const LogLinkNode* tail() const noexcept {
        return this->_tail;
    }

    size_t size() const noexcept {
        return this->_size;
    }

private:
    void clearNode();

private:
    std::atomic<LogLinkNode*> _head {nullptr};
    std::atomic<LogLinkNode*> _tail {nullptr};
    std::atomic<size_t> _size {0};
    std::atomic_bool _lock {false};

}; // LogLink

class Logger
{
private:
    RollLogFile* _log;

}; // Logger

#endif // __LINUX_STUDY_LOG_TOOL_LOG_TOOL_H
