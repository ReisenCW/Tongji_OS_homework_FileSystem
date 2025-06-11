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
    if (file.open(QIODevice::WriteOnly)) {
        int block = allocateBlock();
        if (block != -1) {
            bitmap.set(block);
            if (block < fat.size()) {
                fat[block] = -1;
            }
            else {
                std::cerr << "FAT table out of range for block: " << block << std::endl;
                file.close();
                return false;
            }
            Inode inode;
            inode.firstBlock = block;
            inode.size = 0; // 初始大小为 0
            inode.createTime = QDateTime::currentDateTime();
            inode.modifyTime = inode.createTime;
            saveInode(path, inode);
            return true;
        }
        file.close();
    }
    return false;
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
        // 先加载 inode 信息
        Inode inode = loadInode(path);

		QFile file(QString::fromStdString(fullPath));
        if (file.remove()) {
            // 释放物理块
            int block = inode.firstBlock;
            while (block != -1 && block < BLOCK_COUNT) {
                int nextBlock = fat[block];
                bitmap.reset(block);
                fat[block] = -1;
                block = nextBlock;
            }

            // 删除对应的 inode 文件
            std::string inodePath = config.realRootPath + "/inode/" +
                getFullPath(path).substr(config.realRootPath.length()) + ".inode";
            QFile inodeFile(QString::fromStdString(inodePath));
            if (inodeFile.exists() && !inodeFile.remove()) {
                std::cerr << "Failed to remove inode file: " << inodePath << std::endl;
            }
            return true;
        }
        return false;
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
            if (item.type == FileType::Directory) {
                // 递归计算文件夹大小
                QDir subDir(fileInfo.absoluteFilePath());
                QFileInfoList subFileList = subDir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
                qint64 totalSize = 0;
                for (const auto& subFileInfo : subFileList) {
                    totalSize += subFileInfo.size();
                }
                item.size = totalSize;
            }
            else {
                item.size = fileInfo.size();
            }
            item.createTime = fileInfo.birthTime();
            item.modifyTime = fileInfo.lastModified();
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

bool renameItem(const std::string& oldPath, const std::string& newPath) {
    std::string oldFullPath = getFullPath(oldPath);
    std::string newFullPath = getFullPath(newPath);
    QFile file(QString::fromStdString(oldFullPath));
    if (file.rename(QString::fromStdString(newFullPath))) {
        // 判断是文件还是文件夹，如果是文件夹，则直接return true,无需处理inode
        QFileInfo fileInfo(QString::fromStdString(newFullPath));
        if (fileInfo.isDir()) {
            return true;
        }
        // 重命名inode文件
        std::string oldInodePath = config.realRootPath + "/inode/" + getFullPath(oldPath).substr(config.realRootPath.length()) + ".inode";
        std::string newInodePath = config.realRootPath + "/inode/" + getFullPath(newPath).substr(config.realRootPath.length()) + ".inode";
        QFile inodeFile(QString::fromStdString(oldInodePath));
        if (inodeFile.rename(QString::fromStdString(newInodePath))) {
            return true;
        }
    }
    return false;
}