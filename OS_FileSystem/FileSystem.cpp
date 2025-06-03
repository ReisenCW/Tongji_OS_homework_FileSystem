#include "FileSystem.h"
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <iostream>

// 定义全局的FAT表和位图
FAT fat;
Bitmap bitmap;

void formatFileSystem() {
    fat.assign(BLOCK_COUNT, -1);
    bitmap.reset();
    // 初始化根目录
    Directory root;
    root.path = "/";
    // 保存到磁盘
    std::ofstream fatFile("fat.bin", std::ios::binary);
    fatFile.write(reinterpret_cast<const char*>(fat.data()), fat.size() * sizeof(int));
    fatFile.close();

    std::ofstream bitmapFile("bitmap.bin", std::ios::binary);
    bitmapFile.write(reinterpret_cast<const char*>(&bitmap), sizeof(bitmap));
    bitmapFile.close();
}

// 创建目录
bool createDirectory(const std::string& path) {
    std::string fullPath = getFullPath(path);
    QDir dir(QString::fromStdString(fullPath));
    if (dir.exists()) {
        return false;
    }
    return dir.mkpath(".");
}

// 创建文件
bool createFile(const std::string& path) {
    std::string fullPath = getFullPath(path);
    QFile file(QString::fromStdString(fullPath));
    return file.open(QIODevice::WriteOnly);
}

// 删除文件/目录
bool deleteItem(const std::string& path) {
    std::string fullPath = getFullPath(path);
    QFileInfo fileInfo(QString::fromStdString(fullPath));
    if (fileInfo.isDir()) {
        QDir dir(QString::fromStdString(fullPath));
        return dir.removeRecursively();
    }
    else {
        QFile file(QString::fromStdString(fullPath));
        return file.remove();
    }
}

// 获取目录信息
Directory getDirectoryInfo(const std::string& path) {
    Directory dirInfo;
    std::string actualPath = getFullPath(path);
    dirInfo.path = actualPath;
    QDir dir(QString::fromStdString(actualPath));
    QFileInfoList fileList = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    for (const auto& fileInfo : fileList) {
        std::string fullPath = fileInfo.absoluteFilePath().toStdString();
        // 检查文件或目录是否在自定义文件系统范围内
        if (fullPath.find(config.realRootPath) == 0) {
            FileItem item;
            item.name = fileInfo.fileName().toStdString();
            item.type = fileInfo.isDir() ? FileType::Directory : FileType::File;
            item.size = fileInfo.size();
            item.createTime = fileInfo.birthTime();
            item.modifyTime = fileInfo.lastModified();
            // 这里暂时不处理inode
            item.inode = -1;
            dirInfo.items.push_back(item);
        }
    }
    return dirInfo;
}

// 打开文件进行编辑
bool openFileForEdit(const std::string& path) {
    std::string fullPath = getFullPath(path);
    QFile file(QString::fromStdString(fullPath));
    if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        // 这里可以添加打开文件编辑器的逻辑，例如使用系统默认编辑器打开
        std::string command = "notepad.exe \"" + fullPath + "\""; // 适用于 Windows 系统
        int result = std::system(command.c_str());
        file.close();
        return result == 0;
    }
    return false;
}

// 读取文件内容
std::string readFileContent(const std::string& path) {
    std::string fullPath = getFullPath(path);
    QFile file(QString::fromStdString(fullPath));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString content = in.readAll();
        file.close();
        return content.toStdString();
    }
    return "";
}