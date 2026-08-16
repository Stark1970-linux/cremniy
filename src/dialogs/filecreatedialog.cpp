#include "filecreatedialog.h"

#include "QIODevice"
#include "QFile"
#include "QFileInfo"
#include <qdir.h>

FileCreateDialog::FileCreateDialog(QWidget *parent, const QString& path, bool _is_dir): QDialog(parent) {

    this->dir_path = path;
    this->is_dir = _is_dir;

    lineEdit = new QLineEdit(this);

    if (is_dir) {
        setWindowTitle(tr("Create folder"));
        lineEdit->setPlaceholderText(tr("Enter folder name..."));
    }
    else {
        setWindowTitle(tr("Create file"));
        lineEdit->setPlaceholderText(tr("Enter file name..."));
    }

    setFixedSize(300, 100); // маленькое окно

    QVBoxLayout *layout = new QVBoxLayout(this);

    // текстовое поле
    layout->addWidget(lineEdit);

    // кнопка
    QPushButton *button = new QPushButton(tr("Create"), this);
    layout->addWidget(button);

    connect(button, &QPushButton::clicked, this, &FileCreateDialog::onCreateClicked);
}

void FileCreateDialog::onCreateClicked() {
    QString fileName = lineEdit->text();
    if(fileName.isEmpty()) {
        if (is_dir) QMessageBox::warning(this, tr("Error"), tr("Enter folder name!"));
        else QMessageBox::warning(this, tr("Error"), tr("Enter file name!"));
        return;
    }

    if (dir_path.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("No directory specified!"));
        return;
    }

    QDir dir(dir_path);
    if (!dir.exists()) {
        QMessageBox::critical(this, tr("Error"),
            tr("Directory does not exist: %1").arg(dir_path));
        return;
    }

    QString fullPath = dir.filePath(fileName);

    if (is_dir) {
        if (!dir.mkpath(fullPath)) {
            QMessageBox::critical(this, tr("Error"), tr("Failed to create directory!"));
            return;
        }
        accept();
    }
    else {
        QDir().mkpath(QFileInfo(fullPath).absolutePath());
        QFile file(fullPath);
        if(file.open(QIODevice::WriteOnly)) {
            file.close();
            accept();
        } else {
            QMessageBox::critical(this, tr("Error"),
                tr("Failed to create file: %1").arg(file.errorString()));
        }
    }
}