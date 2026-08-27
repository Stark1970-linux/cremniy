#include "systemtitlebar.h"

#include <QGuiApplication>
#include <QStyleHints>

namespace SystemTitleBar {

void apply(QWidget* window, bool dark)
{
    Q_UNUSED(window);

    if (!qGuiApp)
        return;

    // Qt 6.8+ provides a platform-independent color-scheme hint.
    // The application keeps the native system window decoration; the
    // platform/window manager decides how the requested scheme is rendered.
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    qGuiApp->styleHints()->setColorScheme(
        dark ? Qt::ColorScheme::Dark : Qt::ColorScheme::Light
    );
#else
    Q_UNUSED(dark);
#endif
}

} // namespace SystemTitleBar
