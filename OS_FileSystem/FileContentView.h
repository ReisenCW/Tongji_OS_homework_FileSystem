#pragma once

#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>

class FileContentView : public QWidget
{
    Q_OBJECT

public:
    FileContentView(const std::string& fileName, const std::string& content, QWidget* parent = nullptr);
    ~FileContentView() {}

private:
    QLabel* fileNameLabel;
    QTextEdit* contentTextEdit;
    QPushButton* closeButton;
};