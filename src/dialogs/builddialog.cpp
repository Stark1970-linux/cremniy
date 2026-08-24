#include "builddialog.h"
#include "core/buildman/buildmanager.h"
#include "logView/logview.h"
#include <qboxlayout.h>
#include <qlabel.h>


buildDialog::buildDialog(const ProjectInfo &projInfo, QWidget *parent)
    : QDialog(parent)
{

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Build"));

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(40, 40, 40, 40);
    root->setSpacing(0);

    /* Title */
    auto* title = new QLabel(tr("Build"));
    title->setStyleSheet("color:#e0e0e0; font-size:16px; font-weight:bold;");
    root->addWidget(title);
    root->addSpacing(24);

    /* Log */
    auto* logLayout = new QVBoxLayout();
    auto* logViewWidg = new logView();

    logLayout->addWidget(logViewWidg);

    root->addLayout(logLayout);

    layout()->setSizeConstraint(QLayout::SetMinimumSize);
    resize(480, sizeHint().height());

    BuildManager *bman = new BuildManager(projInfo.path, projInfo.buildCommand, logViewWidg);
    bman->build();


}
