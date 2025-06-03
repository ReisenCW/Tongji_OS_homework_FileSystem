#pragma once
#include <string>
#include <vector>
#include <QDateTime> // Qt 时间类，用于记录文件信息
#include <bitset>

// 声明 Config 类
class Config {
public:
    Config() : rootPath("/home"), realRootPath("/"), currentPath("/home") {}
public:
    std::string rootPath; // 虚拟根目录
    std::string realRootPath; // 真实根目录
    std::string currentPath; // 当前路径
};

// 声明全局的 config 变量
extern Config config;

// 文件类型枚举
enum class FileType {
    File,       // 普通文件
    Directory   // 目录
};

// 文件/目录项基础结构
struct FileItem {
    std::string name;          // 名称（不含路径）
    FileType type;             // 类型（文件/目录）
    qint64 size;               // 大小（字节，目录为0）
    QDateTime createTime;      // 创建时间
    QDateTime modifyTime;      // 修改时间
    int inode;                 // 索引节点号
};

// 目录结构体（包含子项列表）
struct Directory {
    std::string path;          // 目录路径（全路径，如"/docs"）
    std::vector<FileItem> items; // 子文件/目录列表
};

// 索引节点结构
struct Inode {
    int firstBlock;            // 第一个物理块号
    qint64 size;               // 文件大小
    QDateTime createTime;      // 创建时间
    QDateTime modifyTime;      // 修改时间
};

// 假设文件系统总共有1024个物理块
const int BLOCK_COUNT = 1024;
using FAT = std::vector<int>;
using Bitmap = std::bitset<BLOCK_COUNT>;

// 全局的FAT表和位图
extern FAT fat;
extern Bitmap bitmap;

// 辅助函数：获取文件/目录的全路径
std::string getFullPath(const std::string& relativePath);

// 辅助函数：简化路径
std::string simplifyPath(const std::string& path);