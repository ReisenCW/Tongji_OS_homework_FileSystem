#pragma once
#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>
class FileContentView : public QWidget {
    Q_OBJECT
public:
    FileContentView(const std::string& fileName, const std::string& content, bool isEditable = true, QWidget* parent = nullptr);
    ~FileContentView();
    std::string getContent() const; // 获取编辑后的内容
private slots:
    void saveContent(); // 保存内容的槽函数
signals:
    void contentSaved(const std::string& content); // 文件内容保存信号
private:
    QLabel* fileNameLabel;
    QTextEdit* contentTextEdit;
    QPushButton* saveButton;
};