/**
* @File main.cc
* @Date 2023-04-06
* @Description 
* @Author Ticks
* @Email ticks.cc\@gmail.com
*
* Copyright 2023 Ticks, Inc. All rights reserved. 
**/

#include "parse_args.h"
#include "file_tool.h"

int main()
{
    auto f = FileReader(File::open("../cmake_install.cmake", File::kReadOnly));
    if (!f) {
        printf("open file error: %s\n", f.file().errorMsg());
        return 1;
    }
    printf(">> 读取一行\n");
    auto line = f.readStringLine();
    printf(">> %s\n", line.data());

    int* i = f.readTo<int>();
    printf(">> read int data: %d\n", *i);
    delete i;

    printf(">> 读取全部数据\n");
    auto vec = f.readVec();
    for (auto& ch : vec) {
        printf("%c", ch);
    }
    return 0;
}