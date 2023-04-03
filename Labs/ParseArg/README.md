## 程序命令行参数解析

### 功能描述：

利用Linux下相关函数实现C++程序 main 函数参数解析。

### 使用函数原型：

```cpp
// 包含头文件
#include <getopt.h>

// 一些特殊的变量
extern int opterr;      // 如果为非0值，则在参数缺失或错误时输出错误信息，否则不输出
extern int optopt;      // 遇到未知选项字符或者参数缺失时存储该选项字符
extern int optind;      // argv中参数开始下标，可在选项字符遍历完成后访问参数
extern char* optarg;    // 当选项字符需要参数时指向对应参数
/**
 * @param argc : 与main函数中argc相同
 * @param argv : 与main函数中argv相同
 * @param opts : 包含合法选项字符的字符串
 * @return     : 返回参数中的选项字符，错误返回 ? 
 *              ，全部遍历完成返回 -1
 **/
int getopt(int argc, char* const* argv, const chat* opts);

// 长参数描述的结构体
struct option
{
    const char *name;   // 长参数名称
    /* has_arg can't be an enum because some compilers complain about 
     * type mismatches in all the code that assumes it is an int.  */
    int has_arg;        // 选项后是否要求参数
    int *flag;          // 如果为NULL，则函数调用返回val值，不为空则返回0,flag执行变量存储val
    int val;            // 函数返回的值，可以是字符或者数字
};

/* option has_arg 成员使用下面三个宏 */
#define no_argument		0       // 不要求参数
#define required_argument	1   // 要求参数
#define optional_argument	2   // 可选参数

int getopt_long (int argc, char* const* argv, const char* opts, const struct option* longopts, int* longind);
int getopt_long_only (int argc, char* const* argv, const char* opts, const struct option* longopts, int* longind);

```

### 部分参数说明：

**opts**: 适用于短字符的选项字符串，形式类似 `"f:c::a"`，其中每个字符代表一个短选项，`:` 表示前一个字符有参数，必须为其提供参数，参数使用空格隔开，
`::` 也是表示前一个选项字符需要提供参数，且参数是紧挨着选项字符，不能用空格分开，没有`:` 或 `::` 修饰的字符没有参数要求。
也就是像这样使用：**`./xxx -f arg1 -carg2 -a`** 。参数应该唯一。

**longopts**: 说明长参数的结构体数组，数组最后一个成员必须全用0填充。`option` 结构说明在上面，长参数类似 `./xxx --config=xxx.conf` 形式，
使用单纯的字符串已经没法描述了，所以设计了一个新的结构体来描述。参数应该唯一。

**longind**: 非空的指针，存储当前解析符合 `longopts` 数组参数描述的下标，否则不对其设置。

----

`getopt` 函数只能解析类似 `./xxx -f xxx.conf` 这种单个 `-` 字符参数， 不适用于调用 `./xxx --config=xxx.conf` 这种双 `--` 字符参数。
`getopt_long` 可以处理长参数也可以处理短参数，所以可以只使用支持长参数的版本。

## 示例

通过继承实现简单的参数解析
```cpp
class Args : public ParseArgs
{
protected:
    // 打印用法时会用到
    void onUsage() const override
    {
        std::cout << "Usage ./parse_args_test [options]\n"
               << "-h    --help               show the usage\n"
               << "-f    --config-file=str    set the config file\n"
               << "-t    --test=int           test the number";
    }
    // 这个函数返回一系列参数
    std::vector<Option> onOptions() override
    {
        return {
            // | 长参数 | 参数选项，决定是否需要提供值 | 作为解析参数时传递的code，如果提供字符可作为短参数，确保唯一 | 如果不需要短参数则为 false
                {"help",        kNoArg,  'h', true },
                {"config-file", kReqArg, 'f', true },
                {"test",        kOptArg, 't', true }
        };
    }
    // 解析参数，code用于区分不同参数，arg是解析到的参数值，如果为可选参数，可能为空
    // 返回一个pair,第一个为参数标识，如果在程序中需要使用，则传递这里设置的值
    // 第二个参数是一个 "任意类型" 的值，只要你可以从 arg 转换到 AnyType,
    // 随便你用什么，不过这里返回的类型决定了你之后获取的类型，你可以返回空表示没有值
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
    // 静态构造方法，返回了基类指针，解析失败会调用 exit 结束程序
    auto args = ParseArgs::Init<Args>(argc, argv);
    // has 可以获取是否有指定的参数
    if (args->has("help")) {
        args->showHelp();
        return 0;
    }
    if (args->has("config-file")) {
        // get 是一个模板函数，需要提供获取类值的型，请确保传递的参数是有的，不然行为未定义。
        printf("config file is %s\n", args->get<std::string>("config-file").data());
    }
    if (args->has("test")) {
        printf("test the number %d\n", args->get<int>("test"));
    }
    return 0;
}
```
