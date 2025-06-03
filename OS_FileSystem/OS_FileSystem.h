#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_OS_FileSystem.h"
#include <Utilities.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qstring.h>

class OS_FileSystem : public QMainWindow
{
    Q_OBJECT

public:
    OS_FileSystem(QWidget *parent = nullptr);
    ~OS_FileSystem() {}
private:
    void Init();
    void InitWidget();
    bool IsValidPath(const QString& path);

private:
    Ui::OS_FileSystemClass ui;
};
