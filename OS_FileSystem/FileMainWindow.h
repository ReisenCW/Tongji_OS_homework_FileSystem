#pragma once

#include <QMainWindow>
#include <QLineEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QFileSystemWatcher> 
#include "ui_FileMainWindow.h"
#include "Utilities.h"
#include <string>

class FileMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    FileMainWindow(QWidget* parent = nullptr);
    ~FileMainWindow() {}

private slots:
    void on_confirmButton_clicked();
    void on_exitButton_clicked();
    void on_commandInput_returnPressed();
    void onDirectoryChanged(const QString& path);
    bool writeFileContent(const std::string& path, const std::string& content);
private:
    Ui::FileMainWindowClass ui;
    void Init();
    void InitWidget();
    void updateDirectoryView();
    void addSubDirectories(QTreeWidgetItem* parentItem, const std::string& parentPath);
    const int WINDOW_WIDTH = 1280;
    const int WINDOW_HEIGHT = 720;
    void handleCommand(const std::string& command);
    QLineEdit* currentPathEdit; // 当前路径显示框
    QLineEdit* commandInput; // 命令输入框
    QFileSystemWatcher* fileSystemWatcher;
};