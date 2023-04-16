/**
* @File process.h
* @Date 2023-04-14
* @Description 
* @Author Ticks
* @Email ticks.cc\@gmail.com
*
* Copyright 2023 Ticks, Inc. All rights reserved. 
**/
#ifndef __LINUX_STUDY_PROCESS_PROCESS_H
#define __LINUX_STUDY_PROCESS_PROCESS_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string>
#include <vector>

namespace Process
{
    // 命令结果
    class Result
    {
    public:
        friend class Command;
        ~Result() = default;

        Result(const Result&) = default;
        Result& operator = (const Result&) = default;
        Result(Result&&) noexcept = default;
        Result& operator = (Result&&) = default;

        explicit operator bool() const {
            return _ret_code == 0;
        }

    public:
        // 获取标准输出
        const std::string& output() const {
            return _output_buf;
        }
        size_t output(char* buf, size_t len, size_t start = 0) const
        {
            size_t i = start;
            for (; i < len && i < _output_buf.size(); ++i) {
                buf[i] = _output_buf[i];
            }
            return i;
        }
        // 获取错误输出
        const std::string& error() const {
            return _error_buf;
        }

        size_t error(char* buf, size_t len, size_t start) const
        {
            size_t i = start;
            for (; i < len && i < _output_buf.size(); ++i) {
                buf[i] = _output_buf[i];
            }
            return i;
        }

        // 获取返回代码
        int retCode() const {
            return _ret_code;
        }

    protected:
        // 读取子进程输出到字符串中
        void readToBuf();
        // 初始化管道
        void initPipe();
        // 初始化父进程管道
        void initInParent() const;
        // 初始化子进程管道
        void initInChild() const;
        // 写入输入到子进程标准输入
        void writeToInput(std::string& input) const;

        explicit Result()
                : _input_pipe{-1, -1}
                , _output_pipe{-1, -1}
                , _error_pipe{-1, -1}
        {  }

        // 关闭管道
        void closePipe() const
        {
            if (_status > 1) {
                // 不确定标准输入父进程写入端是否关闭，选择关闭，已关闭会返回错误，选择忽略
                ::close(_input_pipe[1]);
                if (_status > 2) {
                    ::close(_output_pipe[0]);
                    if (_status > 3) {
                        ::close(_error_pipe[0]);
                    }
                }
            }
        }

    private:
        int _ret_code{0};           // 程序返回值
        int _status {0};            // 管道创建情况
        int _input_pipe[2];         // 标准输入管道
        int _output_pipe[2];        // 标准输出管道
        int _error_pipe[2];         // 标准错误管道
        std::string _output_buf;    // 标准输出缓冲区
        std::string _error_buf;     // 标准错误缓冲区

    }; // Result

    // 命令
    class Command
    {
    public:
        // 静态函数，用于构造命令
        static Command New(std::string& cmd) {
            return std::move(Command{cmd});
        }
        static Command New(const char* cmd) {
            return std::move(Command{cmd});
        }

        Command(const Command&) = delete;
        Command& operator = (const Command&) = delete;

        Command(Command&& c) noexcept = default;
        Command& operator = (Command&&) = default;

        ~Command() = default;

    public:
        // 添加一个参数
        Command& arg(std::string& arg)
        {
            if (!arg.empty()) {
                _args.emplace_back(arg);
            }
            return *this;
        }
        Command& arg(const char* arg)
        {
            if (arg != nullptr) {
                _args.emplace_back(arg);
            }
            return *this;
        }
        // 添加一组参数
        Command& args(const std::vector<std::string>& args)
        {
            for (auto& it : args) {
                if (!it.empty()) {
                    _args.emplace_back(it);
                }
            }
            return *this;
        }

        // 添加一个环境变量
        Command& env(std::string& env)
        {
            if (!env.empty()) {
                _envs.emplace_back(env);
            }
            return *this;
        }
        Command& env(const char* env)
        {
            if (env != nullptr) {
                _envs.emplace_back(env);
            }
            return *this;
        }
        // 添加一组环境变量
        Command& envs(const std::vector<std::string>& envs)
        {
            for (auto& it : envs) {
                if (!envs.empty()) {
                    _envs.emplace_back(it);
                }
            }
            return *this;
        }

        // 添加一个输入
        Command& input(const std::string& input)
        {
            _input_buf += input;
            return *this;
        }
        // 添加一组输入
        Command& input(const std::initializer_list<std::string>& list)
        {
            for (auto& it : list) {
                _input_buf += it;
            }
            return *this;
        }

        // 阻塞执行，子进程 + wait 阻塞
        Result& run();
        // 异步执行，子进程
        Command& start();
        // 等待子进程执行完成
        Result& wait();

    protected:
        explicit Command(std::string& cmd)
            : _command(cmd)
        {
            _args.emplace_back(_command);
        }
        explicit Command(const char* cmd)
            : _command(cmd)
        {
            _args.emplace_back(cmd);
        }

    private:
        Result _result;                 // 执行结果
        pid_t _pid {0};                 // 子进程 pid
        std::string _command;           // 命令
        std::string _input_buf;         // 输入缓冲区
        std::vector<std::string> _args; // 命令行参数
        std::vector<std::string> _envs; // 环境变量

    }; // Command

}

#endif // __LINUX_STUDY_PROCESS_PROCESS_H
