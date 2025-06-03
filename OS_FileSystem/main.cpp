#include "OS_FileSystem.h"
#include <QtWidgets/QApplication>
#include <fstream>

void loadFileSystem() {
    std::ifstream fatFile("fat.bin", std::ios::binary);
    if (fatFile) {
        fatFile.read(reinterpret_cast<char*>(fat.data()), fat.size() * sizeof(int));
        fatFile.close();
    }

    std::ifstream bitmapFile("bitmap.bin", std::ios::binary);
    if (bitmapFile) {
        bitmapFile.read(reinterpret_cast<char*>(&bitmap), sizeof(bitmap));
        bitmapFile.close();
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