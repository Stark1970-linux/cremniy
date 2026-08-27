#ifndef CREMNIY_SYSTEMTITLEBAR_H
#define CREMNIY_SYSTEMTITLEBAR_H

#include <QWidget>

namespace SystemTitleBar {
// Applies the requested light/dark appearance to the native window
// decoration where the platform/window manager exposes such an API.
void apply(QWidget* window, bool dark);
}

#endif // CREMNIY_SYSTEMTITLEBAR_H
