#include "FileContentView.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>

FileContentView::FileContentView(const std::string& fileName, const std::string& content, bool isEditable, QWidget* parent)
    : QWidget(parent) {
    setWindowTitle("编辑文件 - " + QString::fromStdString(fileName));
    setGeometry(200, 200, 800, 600);

    fileNameLabel = new QLabel(QString::fromStdString(fileName), this);
    contentTextEdit = new QTextEdit(this);
    contentTextEdit->setPlainText(QString::fromStdString(content));

    if (isEditable) {
        saveButton = new QPushButton("保存", this);
        connect(saveButton, &QPushButton::clicked, this, &FileContentView::saveContent);

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addWidget(fileNameLabel);
        layout->addWidget(contentTextEdit);
        layout->addWidget(saveButton);
        setLayout(layout);
    }
    else {
        // 只读模式，不显示保存按钮
        contentTextEdit->setReadOnly(true);
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addWidget(fileNameLabel);
        layout->addWidget(contentTextEdit);
        setLayout(layout);
    }
}

FileContentView::~FileContentView() {}

std::string FileContentView::getContent() const {
    return contentTextEdit->toPlainText().toStdString();
}

void FileContentView::saveContent() {
    emit contentSaved(getContent()); 
    close();
}