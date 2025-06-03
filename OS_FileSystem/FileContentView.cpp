#include "FileContentView.h"

FileContentView::FileContentView(const std::string& fileName, const std::string& content, QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle("文件内容");
    setGeometry(200, 200, 600, 400);

    fileNameLabel = new QLabel(QString::fromStdString(fileName), this);
    contentTextEdit = new QTextEdit(this);
    contentTextEdit->setReadOnly(true);
    contentTextEdit->setPlainText(QString::fromStdString(content));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(fileNameLabel);
    layout->addWidget(contentTextEdit);

    setLayout(layout);
}

