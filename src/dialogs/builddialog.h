#ifndef BUILDDIALOG_H
#define BUILDDIALOG_H

#include "project_info_manager.h"
#include <QDialog>

class buildDialog : public QDialog {
public:
    buildDialog(const ProjectInfo &projInfo, QWidget *parent);
};

#endif// BUILDDIALOG_H
