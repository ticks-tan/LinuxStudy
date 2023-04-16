/**
* @File main.cc
* @Date 2023-04-14
* @Description 
* @Author Ticks
* @Email ticks.cc\@gmail.com
*
* Copyright 2023 Ticks, Inc. All rights reserved. 
**/

#include "process.h"
#include <iostream>

#define LF "\n"
#define show std::cout
using namespace Process;

int main(int argc, char* const* argv)
{
    auto& comm = Command::New("grep")
            .arg("hello")
            .input("I am input text\n")
            .input({
                "hello, world\n",
                "oh, so cool!\n",
                "hello, c++"
            })
            .start();
    show << "this is async command!" << LF;
    auto& res = comm.wait();
    if (res) {
        show << res.output() << LF;
    }else {
        show << res.error() << LF;
    }
    return 0;
}