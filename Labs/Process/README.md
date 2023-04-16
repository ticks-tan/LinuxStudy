## 命令执行器 Process

### 功能描述

利用Linux相关函数，实现支持同步和异步的命令执行器。

### 实现思路

父进程使用 `fork` 或 `vfork` 产生一个子进程，子进程使用 `exec` 系列函数
执行命令，父进程可以使用 `waitpid`获取子进程返回结果，使用管道获取子进程输出。

### 相关函数

1. **创建子进程函数**

    ```cpp
    #include <sys/types.h>
    #include <unistd.h>
    
    // 函数返回两次，调用之后父子进程会执行相同代码，所以应该在之后对返回值进行判断，区别父进程和子进程
    // 大于0为父进程，返回值为子进程PID，应该用变量保存，用于之后获取子进程状态
    // 等于0为子进程，
    // 子进程会复制父进程数据、堆、栈、屏蔽的信号等
    pid_t fork(void);
    // 与 fork 类似，但是与父进程共享进程地址空间，确保子进程先被调用，期间父进程阻塞，
    // 直到子进程调用 exec 系列函数或者 exit 类函数退出，父进程才能被调度运行
    pid_t vfork(void);
    ```
    
    `vfork` 是实现同步执行需要的函数，而 `fork` 则是异步执行需要的函数。

2. **`exec`系列函数**
       
   `exec`系列函数用于执行需要执行的指令。
    ```cpp
   #include <unistd.h>
   
   int execl(const char* path, const char* args, ...);
   int execlp(const char* file, const char* args, ...);
   int execle(const char* path, const char* args, ..., char* const envp[]);
   int execv(const char* path, char* const argv[]);
   int execvp(const char* file, char* const argv[]);
   int execve(const char* path, char* const argv[], char* const envp[]);
    ```
   
   函数记忆有方法，公告部分是 `exec` ，后缀分为 `[v或l]` 和 `[p][e]` 。
   
   以 `v` 为 `exec` 后续的参数部分是 `NULL` 结尾的数组，而 `l` 作为后续
   的参数部分是可变参数，也需要以 `NULL` 结束。

   以 `p` 结尾的函数第一个参数为文件名，即可以不用输入完整路径，函数会在环境变量中寻找。
   以 `e` 结尾的函数需要指定环境变量数组，每个成员指定一个环境变量，也需要以 `NULL` 结尾。

3. **创建管道与重定向**

   由于需要获取子进程执行命令的输出，
   需要使用管道读取标准输出和错误或者标准输入需要使用重定向到管道方便父进程读取。

   ```cpp
   #include <unistd.h>
   
   // 创建一个管道，参数存储管道的文件描述符，返回值用于判断创建结果
   int pipe(int fds[2]);
   
   // 重定向参数指定的描述符，返回最小可用的新描述符，失败返回 -1。
   int dup(int oldfd);
   // 重定向 oldfd 到 newfd，返回重定向的文件描述符，失败返回 -1。
   int dup2(int oldfd, int newfd);
   ```
   
### 示例

执行简单带参数命令
```cpp
#include "process.h"
#include <iostream>

#define LF "\n"
#define show std::cout
using namespace Process;

int main(int argc, char* const* argv)
{
    Result res =
            Command::New("ls")
            .arg("-a")
            .args({"-v", "-hl"})
            .run();
    if (res) {
        show << res.output() << LF;
    }else {
        show << res.error() << LF;
    }
    return 0;
}
```

执行带参数和环境变量的命令

```cpp
#include "process.h"
#include <iostream>

#define LF "\n"
#define show std::cout
using namespace Process;

int main(int argc, char* const* argv)
{
    Result res =
            Command::New("/usr/bin/ls")
            .arg("-a")
            .args({"-v", "-hl"})
            .env("PATH=/usr/bin:/bin")
            .run();
    if (res) {
        show << res.output() << LF;
    }else {
        show << res.error() << LF;
    }
    return 0;
}
```

执行带有输入的命令

```cpp
#include "process.h"
#include <iostream>

#define LF "\n"
#define show std::cout
using namespace Process;

int main(int argc, char* const* argv)
{
    Result res =
            Command::New("grep")
            .arg("hello")
            .input("I am input text\n")
            .input({
                "hello, world\n",
                "oh, so cool!\n",
                "hello, c++"
            })
            .run();
    if (res) {
        show << res.output() << LF;
    }else {
        show << res.error() << LF;
    }
    return 0;
}
```

执行异步命令

```cpp
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
```

