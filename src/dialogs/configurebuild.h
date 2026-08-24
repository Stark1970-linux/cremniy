#ifndef CONFIGUREBUILD_H
#define CONFIGUREBUILD_H

#include <QDialog>
#include <qlabel.h>
#include <qlineedit.h>

class ConfigureBuild : public QDialog {

    public:
        ConfigureBuild();

    private:
        QLabel* m_buildCommandLabel;
        QLineEdit* m_buildCommandEdit;

        void saveConfigureClicked();
        void cancelClicked();

};

#endif// CONFIGUREBUILD_H
