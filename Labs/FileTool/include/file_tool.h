/**
* @File file_tool.h
* @Date 2023-04-06
* @Description 文件操作工具，进行一些常见文件操作
* @Author Ticks
* @Email ticks.cc\@gmail.com
*
* Copyright 2023 Ticks, Inc. All rights reserved. 
**/
#ifndef __LINUX_STUDY_FILE_TOOL_FILE_TOOL_H
#define __LINUX_STUDY_FILE_TOOL_FILE_TOOL_H

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string>
#include <cstring>
#include <vector>

class File
{
public:
    typedef unsigned char byte;
    typedef byte* bytePtr;
    typedef const bytePtr constBytePtr;
    friend class FileReader;
    friend class FileWriter;
    enum : int{
        kCreateForce = O_CREAT, // 强制创建
        kCreateNotExist = O_CREAT | O_EXCL, // 不存在则创建
        kReadOnly = O_RDONLY,   // 仅读取
        kWriteOnly = O_WRONLY,  // 仅写入
        kReadWrite = O_RDWR,    // 读取加写入
        kTrunc = O_TRUNC,   // 截取
        kAppend = O_APPEND,     // 打开时移动到文件尾
        kNotLink = O_NOFOLLOW,  // 不能为链接文件
        kRequireDir = O_DIRECTORY,   // 要求打开文件为目录
        kUserRead = S_IRUSR,
        kUserWrite = S_IWUSR,
        kUserExec = S_IXUSR,
        kGroupRead = S_IRGRP,
        kGroupWrite = S_IWGRP,
        kGroupExec = S_IXGRP,
        kOtherRead = S_IROTH,
        kOtherWrite = S_IWOTH,
        kOtherExec = S_IXOTH,
    };

private:
    static const int kDefaultOpenFlags = (kReadWrite);
    static const int kMaxRetryCount = 3;

public:
    // 创建文件
    static File create(const std::string& _path, int _file_mode, bool _is_recursion = false, int _dir_mode = 0);
    static File createIfNotExist(const std::string& _path, int _file_mode, bool _is_recursion = false, int _dir_mode = 0);
    // 打开文件
    static File open(const std::string& _path, int _flags = 0);

    // 判断文件或目录是否存在
    static bool isExist(const char* _path) {
        return (access(_path, F_OK) == 0);
    }

    // 获取文件信息
    static struct stat64 fileInfo(const char* _name) {
        struct stat64 s {};
        stat64(_name, &s);
        return s;
    }
    static bool isRegularFile(const char* _name)  {
        auto file_info = File::fileInfo(_name);
        return S_ISREG(file_info.st_mode);
    }
    static bool isDirectoryFile(const char* _name)  {
        auto file_info = File::fileInfo(_name);
        return S_ISDIR(file_info.st_mode);
    }
    static bool isCharFile(const char* _name)  {
        auto file_info = File::fileInfo(_name);
        return S_ISCHR(file_info.st_mode);
    }
    static bool isBlockFile(const char* _name)  {
        auto file_info = File::fileInfo(_name);
        return S_ISBLK(file_info.st_mode);
    }
    static bool isFiFoFile(const char* _name)  {
        auto file_info = File::fileInfo(_name);
        return S_ISFIFO(file_info.st_mode);
    }
    static bool isLinkFile(const char* _name)  {
        auto file_info = File::fileInfo(_name);
        return S_ISLNK(file_info.st_mode);
    }
    static bool isSocketFile(const char* _name)  {
        auto file_info = File::fileInfo(_name);
        return S_ISSOCK(file_info.st_mode);
    }

protected:
    explicit File(std::string _name, int _flags)
        : _file_name(std::move(_name))
        , _open_flags(0)
    {
        this->_open(_flags, -1);
    }
    explicit File(std::string _name, int _flags, int _mode)
            : _file_name(std::move(_name))
            , _open_flags(0)
    {
        this->_open(_flags, _mode);
    }
    explicit File(const char* _name, int _flags)
        : _file_name(_name)
        , _open_flags(0)
    {
        this->_open(_flags, -1);
    }
    explicit File(const char* _name, int _flags, int _mode)
            : _file_name(_name)
            , _open_flags(0)
    {
        this->_open(_flags, _mode);
    }
    File(const File& _file)
        : _file_name(_file._file_name)
        , _open_flags(_file._open_flags)
    {
        this->_open(this->_open_flags, -1);
    }
    File& operator = (const File& _file)
    {
        if (this == &_file) {
            return *this;
        }
        if (this->_fd >= 0) {
            ::close(this->_fd);
        }
        this->_file_name = _file._file_name;
        this->_error = 0;
        this->_open(this->_open_flags, -1);
        return *this;
    }

    explicit operator bool() const {
        return (this->_error == 0);
    }

public:
    ~File() {
        this->flush();
        if (this->_fd >= 0) {
            ::close(this->_fd);
        }
    }

    off_t size() const {
        return this->getFileInfo().st_size;
    }
    uid_t ownerUid() const {
        return this->getFileInfo().st_uid;
    }
    gid_t ownerGid() const {
        return this->getFileInfo().st_gid;
    }

    // 判断文件类型
    bool isRegularFile() const {
        return S_ISREG(this->getFileInfo().st_mode);
    }
    bool isDirectory() const {
        return S_ISDIR(this->getFileInfo().st_mode);
    }
    bool isCharFile() const {
        return S_ISCHR(this->getFileInfo().st_mode);
    }
    bool isBlockFile() const {
        return S_ISBLK(this->getFileInfo().st_mode);
    }
    bool isFiFoFile() const {
        return S_ISBLK(this->getFileInfo().st_mode);
    }
    bool isLinkFile() const {
        return S_ISLNK(this->getFileInfo().st_mode);
    }
    bool isSocket() const {
        return S_ISSOCK(this->getFileInfo().st_mode);
    }

    // 返回当前错误信息
    const char* errorMsg() const
    {
        return std::strerror(_error);
    }

    // 刷新缓存
    void flush() const {
        if (this->_fd >= 0) {
            ::fsync(this->_fd);
        }
    }

public:
    size_t readBytes(bytePtr _buf, size_t _count) const;
    size_t writeBytes(constBytePtr _content, size_t _count) const;
    // 读取任意一块内容
    size_t multiReadBytes(bytePtr _buf, size_t _start, size_t _count) const;
    // 写入任意一块
    size_t multiWriteBytes(constBytePtr _content, size_t _start, size_t _count) const;

public:
    // 初始化文件属性信息
    struct stat64 getFileInfo() const
    {
        struct stat64 s{};
        ::fstat64(this->_fd, &s);
        return s;
    }
    // 内部打开文件
    bool _open(int _flags, int _mode);

private:
    int _open_flags;    // 文件打开标志
    int _fd {-1};       // 打开文件描述符
    int _error {0};     // 当前错误
    std::string _file_name;     // 文件名称

}; // File


// 读取文件
class FileReader
{
public:
    explicit FileReader(File&& file) noexcept
        : _file(file)
    {}
    ~FileReader() = default;

public:

    auto readString() const -> std::string;
    auto readVec() const -> std::vector<unsigned char>;
    auto readStringLine() const -> std::string;
    auto readVecLine() const -> std::vector<unsigned char>;
    auto readString(size_t _len) const -> std::string;
    auto readVec(size_t _len) const -> std::vector<unsigned char>;


    template<typename Tp, typename Rt = typename std::remove_cv<Tp>::type>
    auto readTo() const -> Rt*;

    explicit operator bool() const {
        return bool(this->_file);
    }

    File& file() {
        return this->_file;
    }

private:
    File _file;

}; // FileReader

template<typename Tp, typename Rt>
auto FileReader::readTo() const -> Rt*
{
    Rt* tp = new Rt;
    this->_file.readBytes((File::bytePtr)((void*)tp), sizeof(Rt));
    return tp;
}

class FileWriter
{
public:
    explicit FileWriter(File&& file) noexcept
        : _file(file)
    {}
    ~FileWriter() = default;

public:
    size_t write(const std::string& _content);
    size_t write(const std::string& _content, size_t _count);
    size_t write(const std::vector<File::byte>& _content);
    size_t write(const std::vector<File::byte>& _content, size_t _count);

    template<typename T, typename Tp = typename std::remove_cv<T>::type>
    size_t writeWith(const Tp& v);

private:
    File _file;

}; // FileWriter

template<typename T, typename Tp>
size_t FileWriter::writeWith(const Tp& v)
{
    return this->_file.writeBytes((File::constBytePtr)((void*)&v), sizeof(Tp));
}


#endif // __LINUX_STUDY_FILE_TOOL_FILE_TOOL_H