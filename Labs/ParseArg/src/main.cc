/**
* @File main.cc
* @Date 2023-04-03
* @Description 
* @Author Ticks
* @Email ticks.cc\@gmail.com
*
* Copyright 2023 Ticks, Inc. All rights reserved. 
**/

#include <iostream>
#include "parse_args.h"

class Args : public ParseArgs
{
protected:
    void onUsage() const override
    {
        std::cout << "Usage ./parse_args_test [options]\n"
               << "-h    --help               show the usage\n"
               << "-f    --config-file=str    set the config file\n"
               << "-t    --test=int           test the number";
    }
    std::vector<Option> onOptions() override
    {
        return {
                {"help",        kNoArg,  'h', true },
                {"config-file", kReqArg, 'f', true },
                {"test",        kOptArg, 't', true }
        };
    }
    std::pair<std::string, AnyType> onParseArg(int code, std::string arg) override
    {
        switch (code) {
            case 'h':
                return { "help", {} };
            case 'f':
                return { "config-file", std::move(arg) };
            case 't':
                return { "test", std::stoi(arg) };
            default:
                return {"", {} };
        }
    }
};

int main(int argc, char* const* argv)
{
    auto args = ParseArgs::Init<Args>(argc, argv);
    if (args->has("help")) {
        args->showHelp();
        return 0;
    }
    if (args->has("config-file")) {
        printf("config file is %s\n", args->get<std::string>("config-file").data());
    }
    if (args->has("test")) {
        printf("test the number %d\n", args->get<int>("test"));
    }

    AnyType type = 1;
    type = "Hello";

    std::cout << type.cast<const char*>() << std::endl;
    return 0;
}