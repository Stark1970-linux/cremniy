#include "cremniytitlebar.h"

#include "core/theme/thememanager.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QToolButton>
#include <QWindow>
#include <QIcon>

namespace {
QString buttonStyle(const QColor& background, const QColor& foreground, const QColor& hover)
{
    return QStringLiteral(
        "QToolButton { border: none; border-radius: 4px; background: transparent; color: %1; font-size: 16px; font-weight: 500; }"
        "QToolButton:hover { background: %2; }"
        "QToolButton:pressed { background: %2; }"
    ).arg(foreground.name(QColor::HexRgb), hover.name(QColor::HexRgb));
}
}

CremniyTitleBar::CremniyTitleBar(QWidget* window, QWidget* parent)
    : QWidget(parent), m_window(window)
{
    setObjectName(QStringLiteral("CremniyTitleBar"));
    setFixedHeight(38);
    setMouseTracking(true);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 4, 0);
    layout->setSpacing(6);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(20, 20);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    if (m_window)
        m_iconLabel->setPixmap(m_window->windowIcon().pixmap(18, 18));
    layout->addWidget(m_iconLabel);

    m_titleLabel = new QLabel(m_window ? m_window->windowTitle() : QStringLiteral("Cremniy"), this);
    m_titleLabel->setObjectName(QStringLiteral("CremniyTitleBarTitle"));
    m_titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    layout->addWidget(m_titleLabel, 1);

    m_minimizeButton = new QToolButton(this);
    m_minimizeButton->setObjectName(QStringLiteral("CremniyTitleBarMinimize"));
    m_minimizeButton->setText(QStringLiteral("−"));
    m_minimizeButton->setFixedSize(44, 30);
    m_minimizeButton->setToolTip(tr("Minimize"));

    m_maximizeButton = new QToolButton(this);
    m_maximizeButton->setObjectName(QStringLiteral("CremniyTitleBarMaximize"));
    m_maximizeButton->setFixedSize(44, 30);
    m_maximizeButton->setToolTip(tr("Maximize"));

    m_closeButton = new QToolButton(this);
    m_closeButton->setObjectName(QStringLiteral("CremniyTitleBarClose"));
    m_closeButton->setText(QStringLiteral("×"));
    m_closeButton->setFixedSize(44, 30);
    m_closeButton->setToolTip(tr("Close"));

    layout->addWidget(m_minimizeButton);
    layout->addWidget(m_maximizeButton);
    layout->addWidget(m_closeButton);

    if (m_window) {
        connect(m_minimizeButton, &QToolButton::clicked, m_window, &QWidget::showMinimized);
        connect(m_maximizeButton, &QToolButton::clicked, this, &CremniyTitleBar::toggleMaximize);
        connect(m_closeButton, &QToolButton::clicked, m_window, &QWidget::close);
        connect(m_window, &QWidget::windowTitleChanged, this, &CremniyTitleBar::updateWindowTitle);
        connect(m_window, &QWidget::windowIconChanged, this, [this](const QIcon& icon) {
            if (m_iconLabel)
                m_iconLabel->setPixmap(icon.pixmap(18, 18));
        });
    }

    connect(&ThemeManager::instance(), &ThemeManager::currentThemeChanged,
            this, &CremniyTitleBar::updateTheme);
    connect(&ThemeManager::instance(), &ThemeManager::themePreviewChanged,
            this, &CremniyTitleBar::updateTheme);

    updateTheme();
    updateWindowState();
}

QSize CremniyTitleBar::sizeHint() const
{
    return QSize(400, 38);
}

void CremniyTitleBar::updateTheme()
{
    const QColor color = qApp->property("cremniyTitleBarColor").value<QColor>();
    applyColors(color.isValid() ? color : palette().color(QPalette::Window));
}

void CremniyTitleBar::applyColors(const QColor& color)
{
    m_backgroundColor = color;

    const bool dark = color.lightnessF() < 0.55;
    const QColor foreground = dark ? QColor(Qt::white) : QColor(Qt::black);
    const QColor hover = dark ? color.lighter(125) : color.darker(110);

    QPalette pal = palette();
    pal.setColor(QPalette::Window, color);
    pal.setColor(QPalette::WindowText, foreground);
    setPalette(pal);
    setAutoFillBackground(true);

    m_titleLabel->setPalette(pal);
    m_titleLabel->setStyleSheet(QStringLiteral("color: %1; background: transparent; font-weight: 600;").arg(foreground.name(QColor::HexRgb)));
    m_minimizeButton->setStyleSheet(buttonStyle(color, foreground, hover));
    m_maximizeButton->setStyleSheet(buttonStyle(color, foreground, hover));

    const QColor closeHover = QColor(196, 55, 55);
    m_closeButton->setStyleSheet(buttonStyle(color, foreground, closeHover));

    // The application-wide QSS contains a generic QWidget background rule.
    // Paint the title bar ourselves so that this color always wins over the
    // global stylesheet, including during live theme preview.
    setStyleSheet(QStringLiteral("#CremniyTitleBar { border-bottom: 1px solid %1; }")
                      .arg(hover.name(QColor::HexRgb)));
    update();
}

void CremniyTitleBar::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.fillRect(rect(), m_backgroundColor.isValid()
        ? m_backgroundColor
        : palette().color(QPalette::Window));
}

void CremniyTitleBar::updateWindowTitle(const QString& title)
{
    m_titleLabel->setText(title);
}

void CremniyTitleBar::updateWindowState()
{
    updateMaximizeButton();
}

void CremniyTitleBar::updateMaximizeButton()
{
    if (!m_window || !m_maximizeButton)
        return;

    const bool maximized = m_window->windowState() & Qt::WindowMaximized;
    m_maximizeButton->setText(maximized ? QStringLiteral("❐") : QStringLiteral("□"));
    m_maximizeButton->setToolTip(maximized ? tr("Restore") : tr("Maximize"));
}

void CremniyTitleBar::toggleMaximize()
{
    if (!m_window)
        return;

    if (m_window->isMaximized())
        m_window->showNormal();
    else
        m_window->showMaximized();

    updateMaximizeButton();
}

void CremniyTitleBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton || !m_window) {
        QWidget::mousePressEvent(event);
        return;
    }

    m_dragging = true;
    m_manualMove = false;
    m_dragOffset = event->globalPosition().toPoint() - m_window->frameGeometry().topLeft();

    if (!m_window->isMaximized()) {
        if (QWindow* handle = m_window->windowHandle())
            m_manualMove = !handle->startSystemMove();
        else
            m_manualMove = true;
    } else {
        // A maximized window is restored at the cursor position before starting
        // a manual drag. This makes double-click/drag feel like a native title bar.
        const QPoint cursor = event->globalPosition().toPoint();
        m_window->showNormal();
        const int x = cursor.x() - m_window->width() / 2;
        m_window->move(x, cursor.y() - height() / 2);
        m_dragOffset = QPoint(m_window->width() / 2, height() / 2);
        if (QWindow* handle = m_window->windowHandle())
            m_manualMove = !handle->startSystemMove();
        else
            m_manualMove = true;
    }

    event->accept();
}

void CremniyTitleBar::mouseMoveEvent(QMouseEvent* event)
{
    if (m_dragging && m_manualMove && m_window && !m_window->isMaximized()) {
        m_window->move(event->globalPosition().toPoint() - m_dragOffset);
        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void CremniyTitleBar::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        m_manualMove = false;
    }
    QWidget::mouseReleaseEvent(event);
}

void CremniyTitleBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_window) {
        toggleMaximize();
        event->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}
