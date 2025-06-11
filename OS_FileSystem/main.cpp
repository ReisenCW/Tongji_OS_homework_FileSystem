#include "OS_FileSystem.h"
#include <QtWidgets/QApplication>
#include <fstream>
#include <qfile.h>
#include <filesystem.h>
#include <iostream>

void loadFileSystem() {
    // 初始化 FAT 表和位图（若文件不存在）
    if (!QFile::exists("fat.bin") || !QFile::exists("bitmap.bin")) {
        fat.assign(BLOCK_COUNT, -1); // 初始化为全 -1
        bitmap.reset(); // 初始化为全 0（未使用）
        formatFileSystem(); // 调用格式化函数创建初始文件
    }
    else {
        // 从文件加载现有数据
        std::ifstream fatFile("fat.bin", std::ios::binary);
        if (fatFile) {
            fat.resize(BLOCK_COUNT); // 确保 FAT 表大小正确
            if (!fatFile.read(reinterpret_cast<char*>(fat.data()), fat.size() * sizeof(int))) {
                std::cerr << "Failed to read FAT table from file." << std::endl;
                fat.assign(BLOCK_COUNT, -1); // 初始化为全 -1
            }
            fatFile.close();
        }
        else {
            std::cerr << "Failed to open FAT table file." << std::endl;
            fat.assign(BLOCK_COUNT, -1); // 初始化为全 -1
        }
        // 同理加载 bitmap
        std::ifstream bitmapFile("bitmap.bin", std::ios::binary);
        if (bitmapFile) {
            if (!bitmapFile.read(reinterpret_cast<char*>(&bitmap), sizeof(bitmap))) {
                std::cerr << "Failed to read bitmap from file." << std::endl;
                bitmap.reset(); // 初始化为全 0
            }
            bitmapFile.close();
        }
        else {
            std::cerr << "Failed to open bitmap file." << std::endl;
            bitmap.reset(); // 初始化为全 0
        }
    }
}

int main(int argc, char* argv[])
{
    loadFileSystem();
    QApplication a(argc, argv);
    OS_FileSystem w;
    w.show();
    return a.exec();
}