﻿#include "FileMainWindow.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QTextEdit>
#include <QInputDialog>
#include <FileSystem.h>
#include <sstream>
#include "FileContentView.h"

FileMainWindow::FileMainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    Init();
    updateDirectoryView();

    // 初始化文件系统监控器
    fileSystemWatcher = new QFileSystemWatcher(this);
    connect(fileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &FileMainWindow::onDirectoryChanged);
    // 使用实际路径进行监控
    std::string actualPath = getFullPath(config.currentPath);
    fileSystemWatcher->addPath(QString::fromStdString(actualPath));
}


void FileMainWindow::Init()
{
    // 设置窗口标题
    this->setWindowTitle("File System Simulator");
    // 设置窗口大小
    this->resize(WINDOW_WIDTH, WINDOW_HEIGHT);
    // 初始化界面
    InitWidget();
}

void FileMainWindow::InitWidget()
{
    // 创建一个中心部件
    QWidget* centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);

    // 界面分为两部分，左侧显示文件目录结构，右侧为另一部分。右侧的最上方一行显示当前路径，
    // 下面主要部分显示当前目录的文件(类似windows文件管理器)，最下方可以输入代码(就像linux一样输入makedir等等操作)
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    // 设置主布局的间距和边距
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // 左侧目录树
    QTreeWidget* directoryTree = new QTreeWidget(centralWidget);
    directoryTree->setHeaderLabel("文件目录");
    directoryTree->setObjectName("directoryTree");
    mainLayout->addWidget(directoryTree);

    // 右侧
    QVBoxLayout* rightLayout = new QVBoxLayout();
    // 设置右侧布局的间距和边距
    rightLayout->setSpacing(10);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    // 右侧文件列表
    QTreeWidget* fileList = new QTreeWidget(centralWidget);
    fileList->setHeaderLabels(QStringList() << "名称" << "类型" << "大小(Byte)" << "创建时间" << "修改时间");
    fileList->setObjectName("fileList");
    rightLayout->addWidget(fileList);

    // 当前路径显示
    currentPathEdit = new QLineEdit(centralWidget);
    currentPathEdit->setReadOnly(true);
    currentPathEdit->setText(QString::fromStdString(config.currentPath));
    // 大一点
	currentPathEdit->setMinimumHeight(40);
    rightLayout->addWidget(currentPathEdit);

    // 命令输入框
    commandInput = new QLineEdit(centralWidget);
    commandInput->setPlaceholderText("输入命令...");
	commandInput->setMinimumHeight(40);
    connect(commandInput, &QLineEdit::returnPressed, this, &FileMainWindow::on_commandInput_returnPressed);
    rightLayout->addWidget(commandInput);

    // 快捷按钮区域
    QHBoxLayout* shortcutButtonLayout = new QHBoxLayout();
    shortcutButtonLayout->setSpacing(10);
    shortcutButtonLayout->setContentsMargins(0, 0, 0, 0);

    QPushButton* mkdirButton = new QPushButton("创建文件夹", centralWidget);
    QPushButton* touchButton = new QPushButton("创建文件", centralWidget);
	QPushButton* cdButton = new QPushButton("切换路径", centralWidget);
    QPushButton* rmButton = new QPushButton("删除", centralWidget);
    QPushButton* writeButton = new QPushButton("编辑文件", centralWidget);
    QPushButton* readButton = new QPushButton("读文件", centralWidget);
    QPushButton* renameButton = new QPushButton("重命名", centralWidget);
	QPushButton* clearButton = new QPushButton("清空输入", centralWidget);

    connect(mkdirButton, &QPushButton::clicked, [=]() {
        commandInput->setText("mkdir new_dir");
    });
    connect(touchButton, &QPushButton::clicked, [=]() {
        commandInput->setText("touch new_file");
    });
	connect(cdButton, &QPushButton::clicked, [=]() {
		commandInput->setText("cd file_path");
	});
    connect(rmButton, &QPushButton::clicked, [=]() {
        commandInput->setText("rm file_to_delete");
    });
    connect(writeButton, &QPushButton::clicked, [=]() {
        commandInput->setText("write file_to_edit");
    });
    connect(readButton, &QPushButton::clicked, [=]() {
        commandInput->setText("read file_to_read");
    });
    connect(renameButton, &QPushButton::clicked, [=]() {
        commandInput->setText("rename o_name n_name");
    });
	connect(clearButton, &QPushButton::clicked, [=]() {
		commandInput->clear();
	});

    shortcutButtonLayout->addWidget(mkdirButton);
    shortcutButtonLayout->addWidget(touchButton);
	shortcutButtonLayout->addWidget(cdButton);
    shortcutButtonLayout->addWidget(rmButton);
    shortcutButtonLayout->addWidget(writeButton);
    shortcutButtonLayout->addWidget(readButton);
    shortcutButtonLayout->addWidget(renameButton);
	shortcutButtonLayout->addWidget(clearButton);

    rightLayout->addLayout(shortcutButtonLayout);

    // 确认和退出按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    // 设置按钮布局的间距和边距
    buttonLayout->setSpacing(10);
    buttonLayout->setContentsMargins(0, 0, 0, 0);

    QPushButton* confirmButton = new QPushButton("确认", centralWidget);
    QPushButton* exitButton = new QPushButton("退出", centralWidget);
    connect(confirmButton, &QPushButton::clicked, this, &FileMainWindow::on_confirmButton_clicked);
    connect(exitButton, &QPushButton::clicked, this, &FileMainWindow::on_exitButton_clicked);
    buttonLayout->addWidget(confirmButton);
    buttonLayout->addWidget(exitButton);

    rightLayout->addLayout(buttonLayout);
    mainLayout->addLayout(rightLayout);

    // 调整布局比例
    mainLayout->setStretchFactor(directoryTree, 1);
    mainLayout->setStretchFactor(rightLayout, 3);
}

void FileMainWindow::updateDirectoryView() {
    // 更新左侧目录树和右侧文件列表
    Directory dirInfo = getDirectoryInfo(config.currentPath);

    // 更新左侧目录树
    QTreeWidget* directoryTree = findChild<QTreeWidget*>("directoryTree");
    if (directoryTree) {
        directoryTree->clear();
        QTreeWidgetItem* rootItem = new QTreeWidgetItem(directoryTree);
        rootItem->setText(0, QString::fromStdString(config.rootPath));
        addSubDirectories(rootItem, config.rootPath);
    }

    // 更新右侧文件列表
    QTreeWidget* fileList = findChild<QTreeWidget*>("fileList");
    if (fileList) {
        fileList->clear();
        for (const auto& item : dirInfo.items) {
            QTreeWidgetItem* treeItem = new QTreeWidgetItem();
            treeItem->setText(0, QString::fromStdString(item.name));
            treeItem->setText(1, item.type == FileType::File ? "文件" : "目录");
            treeItem->setText(2, QString::number(item.size));
            treeItem->setText(3, item.createTime.toString());
            treeItem->setText(4, item.modifyTime.toString());
            fileList->addTopLevelItem(treeItem);
        }
    }

    // 更新当前路径显示
    if (currentPathEdit) {
        currentPathEdit->setText(QString::fromStdString(config.currentPath));
    }

    // 更新文件系统监控器的监控路径
    if (fileSystemWatcher) {
        QStringList paths = fileSystemWatcher->directories();
        if (!paths.isEmpty()) {
            fileSystemWatcher->removePaths(paths);
        }
        // 使用实际路径进行监控
        std::string actualPath = getFullPath(config.currentPath);
        fileSystemWatcher->addPath(QString::fromStdString(actualPath));
    }
}

void FileMainWindow::addSubDirectories(QTreeWidgetItem* parentItem, const std::string& parentPath) {
    Directory subDirInfo = getDirectoryInfo(parentPath);
    for (const auto& item : subDirInfo.items) {
        if (item.type == FileType::Directory) {
            QTreeWidgetItem* subItem = new QTreeWidgetItem(parentItem);
            subItem->setText(0, QString::fromStdString(item.name));
            std::string subVirtualPath = parentPath + "/" + item.name;
            addSubDirectories(subItem, subVirtualPath);
        }
    }
}

void FileMainWindow::on_confirmButton_clicked() {
    if (commandInput) {
        std::string command = commandInput->text().toStdString();
        handleCommand(command);
        commandInput->clear();
        updateDirectoryView();
    }
}

void FileMainWindow::on_exitButton_clicked() {
    // 保存文件系统到磁盘
    std::ofstream fatFile("fat.bin", std::ios::binary);
    fatFile.write(reinterpret_cast<const char*>(fat.data()), fat.size() * sizeof(int));
    fatFile.close();

    std::ofstream bitmapFile("bitmap.bin", std::ios::binary);
    bitmapFile.write(reinterpret_cast<const char*>(&bitmap), sizeof(bitmap));
    bitmapFile.close();

    this->close();
}

void FileMainWindow::on_commandInput_returnPressed() {
    if (commandInput) {
        std::string command = commandInput->text().toStdString();
        handleCommand(command);
        commandInput->clear();
        updateDirectoryView();
    }
}

void FileMainWindow::handleCommand(const std::string& command) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(command);
    while (std::getline(tokenStream, token, ' ')) {
        tokens.push_back(token);
    }
    if (tokens.empty()) {
        return;
    }
    if (tokens[0] == "mkdir") {
        if (tokens.size() > 1) {
            std::string relativePath = tokens[1];
            std::string fullVirtualPath;
            if (relativePath[0] == '/') {
                fullVirtualPath = relativePath;
            }
            else {
                if (config.currentPath.back() == '/') {
                    fullVirtualPath = config.currentPath + relativePath;
                }
                else {
                    fullVirtualPath = config.currentPath + "/" + relativePath;
                }
            }
            // 查看是否存在同名文件夹，有的话提示用户，并不创建
			if (QDir(QString::fromStdString(getFullPath(fullVirtualPath))).exists()) {
				QMessageBox::warning(this, "提示", "该目录已存在");
			}
            else {
                if (createDirectory(fullVirtualPath)) {
                    QMessageBox::information(this, "成功", "目录创建成功");
                }
                else {
                    QMessageBox::warning(this, "错误", "目录创建失败");
                }
            }
        }
        else {
            QMessageBox::warning(this, "错误", "缺少目录名参数");
        }
    }
    else if (tokens[0] == "cd") {
        if (tokens.size() > 1) {
            std::string relativePath = tokens[1];
            std::string newVirtualPath;
            if (relativePath == ".") {
                // 输入 cd . 时路径不变
                newVirtualPath = config.currentPath;
            }
            else if (relativePath == "..") {
                // 输入 cd .. 时退回上一级
                size_t pos = config.currentPath.rfind('/');
                if (pos != std::string::npos) {
                    newVirtualPath = config.currentPath.substr(0, pos);
                }
                if (newVirtualPath.empty()) {
                    newVirtualPath = "/";
                }
                // 检查新路径是否在 /home 目录内
                if (newVirtualPath.length() < config.rootPath.length() || newVirtualPath.substr(0, config.rootPath.length()) != config.rootPath) {
                    QMessageBox::warning(this, "错误", "不能跳出 /home 目录");
                    return;
                }
            }
            else if (relativePath == "~") {
                // 输入 cd ~ 回到 /home
                newVirtualPath = "/home";
            }
            else {
                if (relativePath[0] == '/') {
                    newVirtualPath = relativePath;
                }
                else {
                    if (config.currentPath.back() == '/') {
                        newVirtualPath = config.currentPath + relativePath;
                    }
                    else {
                        newVirtualPath = config.currentPath + "/" + relativePath;
                    }
                }
            }
            // 简化路径
            newVirtualPath = simplifyPath(newVirtualPath);

            // 检查新路径是否在 /home 目录内
            if (newVirtualPath.length() < config.rootPath.length() || newVirtualPath.substr(0, config.rootPath.length()) != config.rootPath) {
                QMessageBox::warning(this, "错误", "不能跳出 /home 目录");
                return;
            }

            std::string newActualPath = getFullPath(newVirtualPath);
            QDir dir(QString::fromStdString(newActualPath));
            if (dir.exists()) {
                config.currentPath = newVirtualPath;
                if (currentPathEdit) {
                    currentPathEdit->setText(QString::fromStdString(config.currentPath));
                }
                updateDirectoryView();
            }
            else {
                QMessageBox::warning(this, "错误", "目录不存在");
            }
        }
        else {
            QMessageBox::warning(this, "错误", "缺少路径参数");
        }
    }
    else if (tokens[0] == "touch") {
        if (tokens.size() > 1) {
            std::string relativePath = tokens[1];
            std::string fullVirtualPath;
            if (relativePath[0] == '/') {
                fullVirtualPath = relativePath;
            }
            else {
                if (config.currentPath.back() == '/') {
                    fullVirtualPath = config.currentPath + relativePath;
                }
                else {
                    fullVirtualPath = config.currentPath + "/" + relativePath;
                }
            }
			// 查看是否存在同名文件，有的话提示用户，并不创建
			if (QFile::exists(QString::fromStdString(getFullPath(fullVirtualPath)))) {
				QMessageBox::warning(this, "提示", "该文件已存在");
			}
            else {
                if (createFile(fullVirtualPath)) {
                    QMessageBox::information(this, "成功", "文件创建成功");
                }
                else {
                    QMessageBox::warning(this, "错误", "文件创建失败");
                }
            }
        }
        else {
            QMessageBox::warning(this, "错误", "缺少文件名参数");
        }
    }
    else if (tokens[0] == "rm") {
        if (tokens.size() > 1) {
            std::string relativePath = tokens[1];
            std::string fullVirtualPath;
            if (relativePath[0] == '/') {
                fullVirtualPath = relativePath;
            }
            else {
                if (config.currentPath.back() == '/') {
                    fullVirtualPath = config.currentPath + relativePath;
                }
                else {
                    fullVirtualPath = config.currentPath + "/" + relativePath;
                }
            }
            if (deleteItem(fullVirtualPath)) {
                QMessageBox::information(this, "成功", "文件/目录删除成功");
            }
            else {
                QMessageBox::warning(this, "错误", "文件/目录删除失败");
            }
        }
        else {
            QMessageBox::warning(this, "错误", "缺少文件/目录名参数");
        }
    }
    else if (tokens[0] == "read") {
        if (tokens.size() > 1) {
            std::string relativePath = tokens[1];
            std::string fullVirtualPath;
            if (relativePath[0] == '/') {
                fullVirtualPath = relativePath;
            }
            else {
                if (config.currentPath.back() == '/') {
                    fullVirtualPath = config.currentPath + relativePath;
                }
                else {
                    fullVirtualPath = config.currentPath + "/" + relativePath;
                }
            }
            std::string content = readFileContent(fullVirtualPath);
            if (!content.empty()) {
                // 传递false参数，表示只读模式
                FileContentView* view = new FileContentView(relativePath, content, false);
                view->show();
            }
            else {
                QMessageBox::warning(this, "错误", "文件为空");
            }
        }
        else {
            QMessageBox::warning(this, "错误", "缺少文件名参数");
        }
    }
    else if (tokens[0] == "write") {
        if (tokens.size() > 1) {
            std::string relativePath = tokens[1];
            std::string fullVirtualPath;
            if (relativePath[0] == '/') {
                fullVirtualPath = relativePath;
            }
            else {
                fullVirtualPath = config.currentPath + (config.currentPath.back() == '/' ? "" : "/") + relativePath;
            }

            // 读取现有文件内容
            std::string content = readFileContent(fullVirtualPath);

            // 打开编辑窗口
            // 传递true参数，表示可编辑模式
            FileContentView* editor = new FileContentView(relativePath, content, true);
            connect(editor, &FileContentView::contentSaved, [=](const std::string& newContent) {
                // 保存修改后的内容
                if (writeFileContent(fullVirtualPath, newContent)) {
                    QMessageBox::information(this, "成功", "文件保存成功");
					// 更新 inode 信息
					Inode inode = loadInode(relativePath);
					inode.size = newContent.size();
					inode.modifyTime = QDateTime::currentDateTime();
					saveInode(relativePath, inode);
					updateDirectoryView(); // 更新目录视图
                }
                else {
                    QMessageBox::warning(this, "错误", "文件保存失败");
                }
                });
            editor->show();
        }
        else {
            QMessageBox::warning(this, "错误", "缺少文件名参数");
        }
    }
    else if (tokens[0] == "rename") {
        if (tokens.size() > 2) {
            std::string oldRelativePath = tokens[1];
            std::string newRelativePath = tokens[2];
            std::string oldFullVirtualPath;
            std::string newFullVirtualPath;

            if (oldRelativePath[0] == '/') {
                oldFullVirtualPath = oldRelativePath;
            }
            else {
                if (config.currentPath.back() == '/') {
                    oldFullVirtualPath = config.currentPath + oldRelativePath;
                }
                else {
                    oldFullVirtualPath = config.currentPath + "/" + oldRelativePath;
                }
            }

            if (newRelativePath[0] == '/') {
                newFullVirtualPath = newRelativePath;
            }
            else {
                if (config.currentPath.back() == '/') {
                    newFullVirtualPath = config.currentPath + newRelativePath;
                }
                else {
                    newFullVirtualPath = config.currentPath + "/" + newRelativePath;
                }
            }

            if (renameItem(oldFullVirtualPath, newFullVirtualPath)) {
                QMessageBox::information(this, "成功", "文件/目录重命名成功");
            }
            else {
                QMessageBox::warning(this, "错误", "文件/目录重命名失败");
            }
        }
        else {
            QMessageBox::warning(this, "错误", "缺少文件名参数");
        }
    }
}


// 处理文件系统变化事件
void FileMainWindow::onDirectoryChanged(const QString& path) {
    updateDirectoryView();
}

bool FileMainWindow::writeFileContent(const std::string& path, const std::string& content) {
    std::string fullPath = getFullPath(path);
    QFile file(QString::fromStdString(fullPath));
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << QString::fromStdString(content);
        file.close();

        // 更新 inode 中的文件大小和修改时间
        Inode inode = loadInode(path);
        inode.size = content.size();
        inode.modifyTime = QDateTime::currentDateTime();
        saveInode(path, inode);
        return true;
    }
    return false;
}