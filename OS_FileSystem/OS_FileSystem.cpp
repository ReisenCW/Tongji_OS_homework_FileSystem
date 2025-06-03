#include "OS_FileSystem.h"
#include <QMessageBox>
#include <QDir>
#include <QRegularExpression>
#include <FileMainWindow.h>

OS_FileSystem::OS_FileSystem(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    Init();
}

void OS_FileSystem::Init()
{
    // 初始化根目录
    config.rootPath = "/home";
    config.realRootPath = QDir::currentPath().toStdString(); // 默认使用当前工作目录
    config.currentPath = "/home";
    // 设置窗口标题
    this->setWindowTitle("OS File System");
    // 设置窗口大小
    this->resize(600, 400);
    // 初始化界面
    InitWidget();
}

void OS_FileSystem::InitWidget()
{
    QLabel* titleLabel = new QLabel("OS File System Simulator", this);
    titleLabel->setGeometry(300 - 150, 50, 300, 40);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: black;");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFont(QFont("Arial", 16, QFont::Bold));

    QLabel* hintLabel = new QLabel("虚拟根路径：", this);
    hintLabel->setGeometry(50, 150, 120, 30);
    hintLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: black;");

    QLineEdit* rootPathEdit = new QLineEdit(this);
    rootPathEdit->setGeometry(200, 150, 350, 30);
    rootPathEdit->setText(QString::fromStdString(config.rootPath));
    // 设置不可修改,文本颜色灰色
	rootPathEdit->setReadOnly(true);
	rootPathEdit->setStyleSheet("color: gray; background-color: lightgray;");

    QLabel* realRootPathLabel = new QLabel("实际根路径：", this);
    realRootPathLabel->setGeometry(50, 200, 120, 30);
    realRootPathLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: black;");

    QLineEdit* realRootPathEdit = new QLineEdit(this);
    realRootPathEdit->setGeometry(200, 200, 350, 30);
    realRootPathEdit->setText(QString::fromStdString(config.realRootPath));

    QPushButton* confirmButton = new QPushButton("确认", this);
    confirmButton->setGeometry(300 - 75 - 100, 300, 150, 50);
    QPushButton* exitButton = new QPushButton("退出", this);
    exitButton->setGeometry(300 - 75 + 100, 300, 150, 50);

    connect(confirmButton, &QPushButton::clicked, [=]() {
        // 获取输入的虚拟路径和实际路径
        QString virtualPath = rootPathEdit->text();
        QString realPath = realRootPathEdit->text();

        // 检查虚拟路径合法性
        if (!IsValidPath(virtualPath)) {
            QMessageBox::warning(this, "错误", "虚拟路径包含非法字符，请使用合法的路径名。");
            return;
        }

        // 处理实际路径
        QDir dir(realPath);
        if (!dir.exists()) {
            // 尝试创建目录
            if (!dir.mkpath(".")) {
                QMessageBox::warning(this, "错误", "无法创建目录，请检查权限或路径名是否合法。");
                return;
            }
            QMessageBox::information(this, "提示", "目录不存在，已成功创建。");
        }

        // 规范化路径（解析相对路径、去除尾部斜杠等）
        config.rootPath = virtualPath.toStdString();
        config.realRootPath = dir.absolutePath().toStdString();

        QMessageBox::information(this, "成功", QString("配置已更新：\n虚拟根路径：%1\n实际根路径：%2")
            .arg(virtualPath)
            .arg(dir.absolutePath()));
        // 开始模拟
        // 关闭当前窗口，打开FileMainWindow
		this->close();
		FileMainWindow* fileWindow = new FileMainWindow();
		fileWindow->show();
    });
	connect(exitButton, &QPushButton::clicked, this, &OS_FileSystem::close);
}

bool OS_FileSystem::IsValidPath(const QString& path)
{
    // 路径合法性检查：
    // 1. 允许包含路径分隔符 / 或 \
    // 2. 不能包含其他非法字符：: * ? " < > |
    // 3. 不能以 . 或 .. 作为完整路径（但可以包含在路径中）
    // 4. 不能为空

    if (path.isEmpty()) return false;

    // 允许 / 和 \ 作为路径分隔符，但禁止其他非法字符
	QRegularExpression invalidChars("[*?:\"<>|]");
    if (invalidChars.match(path).hasMatch()) {
        return false;
    }

    // 检查是否为 . 或 ..
    if (path == "." || path == "..") {
        return false;
    }

    // 检查是否包含连续的路径分隔符（如 // 或 \\）
    QRegularExpression consecutiveSeparators("[/\\\\]{2,}");
    if (consecutiveSeparators.match(path).hasMatch()) {
        return false;
    }

    // 检查是否以路径分隔符结尾（除非是根路径）
    if (path.length() > 1 && (path.endsWith('/') || path.endsWith('\\'))) {
        return false;
    }

    return true;
}