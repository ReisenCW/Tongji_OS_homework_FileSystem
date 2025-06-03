#pragma once
#include "Utilities.h"
#include <fstream>

// 全局的FAT表和位图
extern FAT fat;
extern Bitmap bitmap;

// 格式化文件系统
void formatFileSystem();

// 创建目录
bool createDirectory(const std::string& path);

// 创建文件
bool createFile(const std::string& path);

// 删除文件/目录
bool deleteItem(const std::string& path);

// 获取目录信息
Directory getDirectoryInfo(const std::string& path);

// 打开文件进行编辑
bool openFileForEdit(const std::string& path);

// 读取文件内容
std::string readFileContent(const std::string& path);