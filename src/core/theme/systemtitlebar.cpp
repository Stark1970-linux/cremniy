#include "systemtitlebar.h"

#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QStandardPaths>
#include <QWindow>

#if defined(Q_OS_WIN)
#  include <windows.h>
#  include <dwmapi.h>
#endif

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
#  include <QNativeInterface>
#  include <xcb/xcb.h>
#endif

namespace {

QByteArray kdeColorSchemeFile(bool dark)
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(dir);
    const QString path = dir + QStringLiteral("/system-titlebar-%1.colors").arg(dark ? QStringLiteral("dark") : QStringLiteral("light"));

    const QByteArray content = dark
        ? QByteArrayLiteral("[Colors:Window]\nBackgroundNormal=35,38,39\n\n[WM]\nactiveBackground=35,38,39\ninactiveBackground=25,27,28\n")
        : QByteArrayLiteral("[Colors:Window]\nBackgroundNormal=239,240,241\n\n[WM]\nactiveBackground=239,240,241\ninactiveBackground=227,229,231\n");

    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(content);
        file.close();
    }
    return QFile::encodeName(path);
}

} // namespace

namespace SystemTitleBar {

void apply(QWidget* window, bool dark)
{
    if (!window)
        return;

    QWindow* native = window->windowHandle();
    if (!native) {
        window->winId();
        native = window->windowHandle();
    }
    if (!native)
        return;

#if defined(Q_OS_WIN)
    const HWND hwnd = reinterpret_cast<HWND>(native->winId());
    if (!hwnd)
        return;

    const COLORREF caption = dark ? RGB(35, 38, 39) : RGB(239, 240, 241);
    const COLORREF text = dark ? RGB(255, 255, 255) : RGB(30, 30, 30);
    DwmSetWindowAttribute(hwnd, 35 /* DWMWA_CAPTION_COLOR */, &caption, sizeof(caption));
    DwmSetWindowAttribute(hwnd, 36 /* DWMWA_TEXT_COLOR */, &text, sizeof(text));
#elif defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    // KWin supports the _KDE_NET_WM_COLOR_SCHEME X11 property. Other
    // window managers simply ignore it, while the application keeps the
    // normal native system decoration.
    if (QGuiApplication::platformName() != QStringLiteral("xcb"))
        return;

    auto* connection = qGuiApp->nativeInterface<QNativeInterface::QX11Application>()->connection();
    if (!connection)
        return;

    const xcb_window_t xwindow = static_cast<xcb_window_t>(native->winId());
    const xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, 0, 22, "_KDE_NET_WM_COLOR_SCHEME");
    xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(connection, cookie, nullptr);
    if (!reply)
        return;

    const xcb_atom_t property = reply->atom;
    free(reply);

    const QByteArray path = kdeColorSchemeFile(dark);
    xcb_change_property(connection, XCB_PROP_MODE_REPLACE, xwindow, property,
                        XCB_ATOM_STRING, 8, static_cast<uint32_t>(path.size()), path.constData());
    xcb_flush(connection);
#else
    Q_UNUSED(dark);
#endif
}

} // namespace SystemTitleBar
