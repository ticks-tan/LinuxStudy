/**
* @File process.cc
* @Date 2023-04-14
* @Description 
* @Author Ticks
* @Email ticks.cc\@gmail.com
*
* Copyright 2023 Ticks, Inc. All rights reserved. 
**/

#include "process.h"
#include <cstring>

Process::Result& Process::Command::run()
{
    // vector<string> 不能直接转换为 char* const char*
    char** args = new char*[_args.size() + 1];
    char** envs = new char*[_envs.size() + 1];
    // 数组最后一个元素为空
    args[_args.size()] = nullptr;
    envs[_envs.size()] = nullptr;
    // 填充参数
    for (int i = 0; i < _args.size(); ++i) {
        args[i] = new char[_args[i].size() + 1];
        std::strcpy(args[i], _args[i].data());
    }
    // 填充环境变量
    for (int i = 0; i < _envs.size(); ++i) {
        envs[i] = new char[_envs[i].size() + 1];
        std::strcpy(envs[i], _envs[i].data());
    }

    long res;
    _result.initPipe();

    // 创建子进程，子进程中使用 exec 系列函数执行
    _pid = ::fork();
    if (_pid == 0) {
        // 子进程
        _result.initInChild();
        if (!_envs.empty()) {
            res = ::execve(_command.data(), args, envs);
        }else {
            res = ::execvp(_command.data(), args);
        }
        if (res == -1) {
            ::perror("exec command error");
        }
        ::_exit(1);

    } else if (_pid < 0) {
        // 错误
        _result._ret_code = -1;
        ::perror("create child program error");

    }else {
        // 父进程
        _result.initInParent();
        // 通过写入管道向子进程标准输入输出内容
        if (!_input_buf.empty()) {
            _result.writeToInput(_input_buf);
        }
        // 阻塞等待子进程完成退出
        res = ::waitpid(_pid, &_result._ret_code, 0);
        if (res != _pid) {
            _result._ret_code = -1;
        } else {
            // 成功退出，通过管道读取子进程输出
            _result.readToBuf();
        }
        // 子进程执行完成，关闭管道
        _result.closePipe();
    }
    // 回收内存
    for (int i = 0; i < _envs.size(); ++i) {
        delete[] envs[i];
    }
    for (int i = 0; i < _args.size(); ++i) {
        delete[] args[i];
    }
    delete[] envs;
    delete[] args;
    return _result;
}

Process::Command& Process::Command::start()
{
    char** args = new char*[_args.size() + 1];
    char** envs = new char*[_envs.size() + 1];
    args[_args.size()] = nullptr;
    envs[_envs.size()] = nullptr;
    for (int i = 0; i < _args.size(); ++i) {
        args[i] = new char[_args[i].size() + 1];
        std::strcpy(args[i], _args[i].data());
    }
    for (int i = 0; i < _envs.size(); ++i) {
        envs[i] = new char[_envs[i].size() + 1];
        std::strcpy(envs[i], _envs[i].data());
    }

    long res;
    _result.initPipe();

    _pid = ::fork();
    if (_pid == 0) {
        // 子进程
        _result.initInChild();
        if (!_envs.empty()) {
            res = ::execve(_command.data(), args, envs);
        }else {
            res = ::execvp(_command.data(), args);
        }
        if (res == -1) {
            ::perror("exec command error");
        }
        ::_exit(1);

    } else if (_pid < 0) {
        // 错误
        ::perror("create child program error");
        _result._ret_code = -1;

    }else {
        // 父进程
        _result.initInParent();
        if (!_input_buf.empty()) {
            _result.writeToInput(_input_buf);
        }
        // 不用阻塞等待
    }
    // 回收内存，子进程已经克隆一份了，可以回收
    for (int i = 0; i < _envs.size(); ++i) {
        delete[] envs[i];
    }
    for (int i = 0; i < _args.size(); ++i) {
        delete[] args[i];
    }
    delete[] envs;
    delete[] args;
    return *this;
}

Process::Result& Process::Command::wait()
{
    // 阻塞等待
    int res = ::waitpid(_pid, &_result._ret_code, 0);
    if (res != _pid) {
        _result._ret_code = -1;
    }else {
        _result.readToBuf();
    }
    // 子进程执行完成，关闭管道
    _result.closePipe();
    return _result;
}

void Process::Result::readToBuf()
{
    std::vector<char> buf(65);
    long read_len;
    do {
        read_len = ::read(_output_pipe[0], buf.data(), 64);
        if (read_len > 0) {
            _output_buf.append(buf.begin(), buf.begin() + read_len);
        }
    } while (read_len > 0);
    do {
        read_len = ::read(_error_pipe[0], buf.data(), 64);
        if (read_len > 0) {
            _error_buf.append(buf.begin(), buf.begin() + read_len);
        }
    } while (read_len > 0);
}

void Process::Result::initPipe()
{
    // 初始化3组管道，分别作为子进程的标准输入、标准输出、标准错误
    int res = pipe(_input_pipe);
    if (res == -1) {
        _status = 1;
        perror("create input pipe error");
        return;
    }
    res = pipe(_output_pipe);
    if (res == -1) {
        _status = 2;
        perror("create output pipe error");
        return;
    }
    res = pipe(_error_pipe);
    if (res == -1) {
        _status = 3;
        perror("create error pipe error");
        return;
    }
    _status = 4;
}

void Process::Result::initInChild() const
{
    // 根据状态决定关闭哪些管道
    if (_status > 1) {
        // 子进程标准输入不需要写入，所以可以关闭写入端
        ::close(_input_pipe[1]);
        // 首先关闭标准输入，然后复制管道到标准输入实现重定向，下同
        ::close(0);
        dup(_input_pipe[0]);
        if (_status > 2) {
            // 标准输出和错误不需要读取，关闭读取端
            ::close(_output_pipe[0]);
            ::close(1);
            dup(_output_pipe[1]);
            if (_status > 3) {
                ::close(_error_pipe[0]);
                ::close(2);
                dup(_error_pipe[1]);
            }
        }
    }
}

void Process::Result::initInParent() const
{
    if (_status > 1) {
        // 父进程需要向子进程标准输入写入，关闭标准输入的读取端
        ::close(_input_pipe[0]);
        if (_status > 2) {
            // 需要读取子进程标准输出和错误，所以关闭写入端
            ::close(_output_pipe[1]);
            if (_status > 3) {
                ::close(_error_pipe[1]);
            }
        }
    }
}

void Process::Result::writeToInput(std::string &input) const
{
    size_t write_len = 0, total_len = input.length(), len;
    while (write_len < total_len) {
        // 写入标准输入管道的写入端，此函数在父进程执行
        len = ::write(_input_pipe[1], (input.data() + write_len), (total_len - write_len));
        if (len > 0) {
            write_len += len;
            continue;
        }
        break;
    }
    // 为了标记输入完成，这里需要关闭管道
    ::close(_input_pipe[1]);
}
