#ifndef CREMNIYTITLEBAR_H
#define CREMNIYTITLEBAR_H

#include <QColor>
#include <QPoint>
#include <QWidget>

class QLabel;
class QToolButton;

class CremniyTitleBar final : public QWidget
{
    Q_OBJECT

public:
    explicit CremniyTitleBar(QWidget* window, QWidget* parent = nullptr);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private slots:
    void updateTheme();
    void updateWindowState();
    void updateWindowTitle(const QString& title);

private:
    void toggleMaximize();
    void updateMaximizeButton();
    void applyColors(const QColor& color);

    QWidget* m_window = nullptr;
    QLabel* m_iconLabel = nullptr;
    QLabel* m_titleLabel = nullptr;
    QToolButton* m_minimizeButton = nullptr;
    QToolButton* m_maximizeButton = nullptr;
    QToolButton* m_closeButton = nullptr;

    bool m_dragging = false;
    bool m_manualMove = false;
    QPoint m_dragOffset;
    QColor m_backgroundColor;
};

#endif // CREMNIYTITLEBAR_H
