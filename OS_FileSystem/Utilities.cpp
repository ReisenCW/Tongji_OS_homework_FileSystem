#include "Utilities.h"
#include <sstream>
#include <vector>
#include <fstream>
#include <QDir>
#include <iostream>

// 定义并初始化全局的 config 变量
Config config;

std::string getFullPath(const std::string& relativePath) {
    std::string actualPath;
    if (relativePath.empty()) {
        actualPath = config.realRootPath + "/" + config.currentPath.substr(config.rootPath.length());
    }
    else if (relativePath[0] == '/') {
        actualPath = config.realRootPath + relativePath;
    }
    else {
        std::string currentActualPath = config.realRootPath + config.currentPath.substr(config.rootPath.length());
        if (!currentActualPath.empty() && currentActualPath.back() != '/') {
            currentActualPath += '/';
        }
        actualPath = currentActualPath + relativePath;
    }
    // 确保路径不以 '/' 结尾
    if (!actualPath.empty() && actualPath.back() == '/') {
        actualPath.pop_back();
    }
    return actualPath;
}

std::string simplifyPath(const std::string& path) {
    std::vector<std::string> stack;
    std::istringstream iss(path);
    std::string part;

    while (std::getline(iss, part, '/')) {
        if (part == "..") {
            if (!stack.empty()) {
                stack.pop_back();
            }
        }
        else if (!part.empty() && part != ".") {
            stack.push_back(part);
        }
    }

    std::string simplifiedPath;
    for (const auto& p : stack) {
        simplifiedPath += "/" + p;
    }

    return simplifiedPath.empty() ? "/" : simplifiedPath;
}

// 分配一个空闲块
int allocateBlock() {
    for (int i = 0; i < BLOCK_COUNT; ++i) {
        if (!bitmap.test(i)) {
            return i;
        }
    }
    return -1;
}

// 保存索引节点信息到磁盘
void saveInode(const std::string& path, const Inode& inode) {
    std::string inodePath = config.realRootPath + "/inode/" + getFullPath(path).substr(config.realRootPath.length()) + ".inode";
    QDir().mkpath(QFileInfo(QString::fromStdString(inodePath)).absolutePath());
    std::ofstream inodeFile(inodePath, std::ios::binary);
    if (inodeFile) {
        inodeFile.write(reinterpret_cast<const char*>(&inode), sizeof(Inode));
        if (!inodeFile) {
            std::cerr << "Failed to write inode file: " << inodePath << std::endl;
        }
        inodeFile.close();
    }
    else {
        std::cerr << "Failed to open inode file for writing: " << inodePath << std::endl;
    }
}

// 从磁盘加载索引节点信息
Inode loadInode(const std::string& path) {
    Inode inode = { -1, 0, QDateTime(), QDateTime() }; // 默认初始化
    std::string inodePath = config.realRootPath + "/inode/" + getFullPath(path).substr(config.realRootPath.length()) + ".inode";
    std::ifstream inodeFile(inodePath, std::ios::binary);
    if (inodeFile) {
        // 检查文件大小
        inodeFile.seekg(0, std::ios::end);
        std::streamsize size = inodeFile.tellg();
        inodeFile.seekg(0, std::ios::beg);

        if (size == sizeof(Inode)) {
            inodeFile.read(reinterpret_cast<char*>(&inode), sizeof(Inode));
            if (inodeFile.gcount() != sizeof(Inode)) {
                std::cerr << "Failed to read inode file: " << inodePath << std::endl;
            }
        }
        else {
            std::cerr << "Inode file size is incorrect: " << inodePath << std::endl;
        }
        inodeFile.close();
    }
    else {
        std::cerr << "Failed to open inode file: " << inodePath << std::endl;
    }
    return inode;
}

