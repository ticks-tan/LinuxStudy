/**
* @File parse_args.h
* @Date 2023-04-03
* @Description 解析C++命令行参数
* @Author Ticks
* @Email ticks.cc\@gmail.com
*
* Copyright 2023 Ticks, Inc. All rights reserved. 
**/
#ifndef __LINUX_STUDY_FILE_TOOL_PARSE_ARGS_H
#define __LINUX_STUDY_FILE_TOOL_PARSE_ARGS_H

#include <getopt.h>
#include <type_traits>
#include <typeinfo>
#include <memory>
#include <vector>
#include <map>
#include <string>

// 支持任意类型的类简单实现
class AnyType {
public:

    AnyType() = default;
    ~AnyType() = default;

    AnyType(const AnyType &other)
    {
        if (other._data_ptr) {
            _data_ptr = other._data_ptr->clone();
        }
    }
    AnyType &operator=(const AnyType &other)
    {
        _data_ptr = std::move(AnyType(other)._data_ptr);
        return *this;
    }

    // move ctor and copy assignment
    AnyType(AnyType &&other) noexcept : _data_ptr(std::move(other._data_ptr)) {}

    AnyType &operator=(AnyType &&other) noexcept
    {
        _data_ptr = std::move(other._data_ptr);
        return *this;
    }

    template <typename T> using DecayType = typename std::decay<T>::type;
    template <typename T,typename std::enable_if<!std::is_same<DecayType<T>,AnyType>::value,bool>::type = true>
    AnyType(T &&data) noexcept {
        _data_ptr.reset(new AnyDataImpl<DecayType<T>>(std::forward<T>(data)));
    }
    template <typename T,
            typename std::enable_if<!std::is_same<DecayType<T>, AnyType>::value, bool>::type = true>
    AnyType &operator=(T &&data) noexcept{
        _data_ptr.reset(new AnyDataImpl<DecayType<T>>(std::forward<T>(data)));
        return *this;
    }

    bool empty() const {
        return _data_ptr == nullptr;
    }
    const std::type_info &getType() const {
        return (!empty()) ? _data_ptr->getType() : typeid(void);
    }

    template <typename T>
    void checkType() const {
        if (getType().hash_code() != typeid(T).hash_code()) {
            // TODO
        }
    }

    template <typename T>
    void checkBind() const {
        if (empty()) {
            // TODO
        }
    }
    template <typename T>
    const T& cast() const {
        checkType<T>();
        checkBind<T>();
        return static_cast<const AnyDataImpl<T> *>(_data_ptr.get())->data_;
    }
    template <typename T>
    T &cast() {
        checkType<T>();
        checkBind<T>();
        return static_cast<AnyDataImpl<T> *>(_data_ptr.get())->data_;
    }
    
private:
    struct AnyData {
        AnyData() = default;
        virtual ~AnyData() = default;
        virtual const std::type_info& getType() const = 0;
        virtual std::unique_ptr<AnyData> clone() const = 0;
    };

    template <typename T>
    struct AnyDataImpl : public AnyData {
        T data_;
        AnyDataImpl(const T &data) : data_(data) { }
        AnyDataImpl(T &&data) noexcept : data_(std::move(data)) { }
        const std::type_info& getType() const override { return typeid(T); }
        std::unique_ptr<AnyData> clone() const override {
            return std::unique_ptr<AnyDataImpl>(new AnyDataImpl<T>(data_));
        }
    };

private:
    std::unique_ptr<AnyData> _data_ptr;

}; // AnyType

// 解析参数的基类
class ParseArgs
{
public:
    virtual ~ParseArgs() = default;

    // 禁用拷贝构造
    ParseArgs(const ParseArgs&) = delete;
    ParseArgs& operator = (const ParseArgs&) noexcept = delete;

    // 判断参数是否解析成功，目前来说没用
    explicit operator bool() const {
        return _status;
    }

    // 初始化，也需要传递一个子类类型方便分配内存
    template<typename T, typename std::enable_if<std::is_base_of<ParseArgs, T>::value, bool>::type = true>
    static std::unique_ptr<ParseArgs> Init(int argc, char* const* argv)
    {
        std::unique_ptr<ParseArgs> args{new T};
        args->_init(argc, argv);
        return args;
    }

    // 显示帮助程序
    void showHelp() const
    {
        this->onUsage();
    }

    // 获取指定参数的值，需要提供类型并确保参数存在
    template<typename T>
    T get(const std::string& name) const
    {
        return _data.find(name)->second.cast<T>();
    }
    // 如果解析参数中有 name ，则返回 true
    bool has(const std::string& name) const
    {
        return _status && (_data.find(name) != _data.cend());
    }
    // 获取其他参数
    std::vector<std::string>& otherArgs() {
        return _other_args;
    }
    const std::vector<std::string>& otherArgs() const
    {
        return _other_args;
    }

public:
    // 几个参数选项
    static const int kNoArg = no_argument;          // 没有参数
    static const int kReqArg = required_argument;   // 需要参数
    static const int kOptArg = optional_argument;   // 可选参数

    // 参数结构，对外隐藏 option ，提供Option
    struct Option
    {
        std::string long_name;   // 参数名称
        int args;   // 参数要求
        char short_name;    // 短参数
        bool have_short;    // 是否有短参数
        Option(std::string _long_name, int _args, char _short_name, bool _have_short)
            : long_name(std::move(_long_name))
            , args(_args)
            , short_name(_short_name)
            , have_short(_have_short)
        {}
    };

protected:
    // 默认不允许公开构造
    ParseArgs() = default;

    // 子类实现命令行选项
    virtual std::vector<Option> onOptions() = 0;
    // 子类实现解析参数
    virtual std::pair<std::string, AnyType> onParseArg(int code, std::string arg) = 0;
    // 子类实现 help 用法
    virtual void onUsage() const = 0;

private:
    // 内部初始化
    void _init(int argc, char *const *argv)
    {
        // 开启错误信息输出
        opterr = 1;
        // 设置短参数
        std::string ss;

        for (auto& opt : onOptions()) {
            _options.push_back({
                opt.long_name.data(),
                opt.args,
                nullptr,
                opt.short_name
            });
            // 设置短参数
            if (opt.have_short) {
                ss += opt.short_name;
                if (opt.args != kNoArg) {
                    ss += ':';
                }
            }
        }
        _options.push_back({nullptr, 0, nullptr, 0});
        int optIndex = -1, c;
        while ((c = getopt_long(argc, argv, ss.data(), _options.data(), &optIndex)) != -1) {
            // 参数错误
            if ('?' == c) {
                showHelp();
                exit(1);
            }
            // 解析参数
            auto pair = this->onParseArg(c, optarg);
            if (!pair.first.empty()) {
                _data[pair.first] = pair.second;
            }
            optIndex = -1;
        }
        for (optIndex = optind; optIndex < argc; ++optIndex) {
            _other_args.emplace_back(argv[optIndex]);
        }
    }

private:
    std::map<std::string , AnyType> _data;  // 存储解析到的参数
    std::vector<option> _options;           // 传递给 getopt_long 函数的长参数选项
    std::vector<std::string> _other_args;
    bool _status = true;

}; // ParseArgs


#endif // __LINUX_STUDY_FILE_TOOL_PARSE_ARGS_H