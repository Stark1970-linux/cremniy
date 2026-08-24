#ifndef CONFIGUREBUILD_H
#define CONFIGUREBUILD_H

#include <QDialog>
#include <qlabel.h>
#include <qlineedit.h>
#include "project_info_manager.h"

class ConfigureBuild : public QDialog {

    public:
        ConfigureBuild(ProjectInfo &projInfo, QWidget *parrent);

    private:
        QLabel* m_buildCommandLabel;
        QLineEdit* m_buildCommandEdit;
        ProjectInfo* m_projectInfo;

        void saveConfigureClicked();
        void cancelClicked();

};

#endif// CONFIGUREBUILD_H
