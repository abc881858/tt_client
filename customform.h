#pragma once
#include <QWidget>
#include <QPoint>
#include <QRect>

class QTabWidget;
class QTableView;

class CustomForm : public QWidget
{
    Q_OBJECT
public:
    explicit CustomForm(QWidget *parent = nullptr);
    ~CustomForm() override = default;

signals:
    void moved(const QRect &geom);
    void requestClose(CustomForm *self);

protected:
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void enterEvent(QEnterEvent*) override;
    void leaveEvent(QEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void paintEvent(QPaintEvent*) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    enum DragMode {
        None, Move,
        ResizeLeft, ResizeRight, ResizeTop, ResizeBottom,
        ResizeTopLeft, ResizeTopRight, ResizeBottomLeft, ResizeBottomRight
    };

    DragMode hitTest(const QPoint &localPos) const;
    void updateCursorByPos(const QPoint &localPos);
    void clearAncestorCursors();
    void setMouseTrackingRecursive(QWidget *w, bool on = true);
    void installCursorEventFilterRecursive(QWidget *w);

private:
    const int m_margin = 8;
    const int m_minw   = 260;
    const int m_minh   = 160;

    DragMode m_dragMode = None;
    QPoint   m_pressGlobalPos;
    QRect    m_pressGeometry;

    QTabWidget *m_tabs = nullptr;
    QTableView *m_table = nullptr;
};