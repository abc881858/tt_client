#include "customform.h"
#include "formcanvas.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QEnterEvent>
#include <QCursor>
#include <QMenu>
#include <QContextMenuEvent>
#include <QTextEdit>
#include <QVector>
#include <QLine>
#include <cmath>
#include <algorithm>

CustomForm::CustomForm(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_NoSystemBackground, false);
    setAutoFillBackground(true);
    // 注意：作为子部件时仍然允许 Frameless 外观，这样我们自绘边框并处理拖拽/缩放
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setMinimumSize(m_minw, m_minh);

    auto *vl = new QVBoxLayout(this);
    vl->setContentsMargins(1,1,1,1);
    vl->setSpacing(0);

    m_tabs = new QTabWidget(this);
    vl->addWidget(m_tabs);

    // Tab1：表格
    QWidget *page1 = new QWidget;
    auto *pageLay = new QVBoxLayout(page1);
    pageLay->setContentsMargins(6,6,6,6);
    m_table = new QTableView(page1);
    auto *model = new QStandardItemModel(15, 5, m_table);
    for (int r=0; r<model->rowCount(); ++r)
        for (int c=0; c<model->columnCount(); ++c)
            model->setData(model->index(r,c), QString("R%1C%2").arg(r).arg(c));
    m_table->setModel(model);
    m_table->horizontalHeader()->setStretchLastSection(true);
    pageLay->addWidget(m_table);
    m_tabs->addTab(page1, "表格");

    // Tab2：文本
    QWidget *page2 = new QWidget;
    auto *pageLay2 = new QVBoxLayout(page2);
    pageLay2->setContentsMargins(6,6,6,6);
    auto *te = new QTextEdit(page2);
    te->setPlainText("可拖动/可缩放的自定义组件。\n右键关闭。");
    pageLay2->addWidget(te);
    m_tabs->addTab(page2, "文本");

    // 鼠标跟踪与事件过滤
    setMouseTracking(true);
    setMouseTrackingRecursive(this, true);
    installCursorEventFilterRecursive(this);
}

void CustomForm::paintEvent(QPaintEvent *ev)
{
    Q_UNUSED(ev);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), QColor("#232324")); // 背景

    // 外边框
    QPen pen(QColor(255,255,255,40));
    pen.setWidth(1);
    p.setPen(pen);
    p.drawRect(rect().adjusted(0,0,-1,-1));
}

void CustomForm::enterEvent(QEnterEvent *e)
{
    QWidget::enterEvent(e);
    clearAncestorCursors();
    updateCursorByPos(mapFromGlobal(QCursor::pos()));
}

void CustomForm::leaveEvent(QEvent *e)
{
    QWidget::leaveEvent(e);
    unsetCursor();
    updateGuidelines({});
}

void CustomForm::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() != Qt::LeftButton) return;
    m_dragMode = hitTest(ev->pos());
    m_pressGlobalPos = ev->globalPosition().toPoint();
    m_pressGeometry = geometry();
    ev->accept();
}

void CustomForm::mouseMoveEvent(QMouseEvent *ev)
{
    const QPoint global = ev->globalPosition().toPoint();
    const QPoint delta  = global - m_pressGlobalPos;

    if (m_dragMode == None) {
        updateCursorByPos(ev->pos());
        updateGuidelines({});
        return;
    }

    QRect g = m_pressGeometry;
    switch (m_dragMode) {
    case Move: {
        QPoint p = g.topLeft() + delta;
        // 保持在父容器内部的简单约束（可按需增强）
        if (parentWidget()) {
            QRect pr = parentWidget()->rect();
            QSize sz = g.size();
            p.setX(std::max(pr.left(), std::min(p.x(), pr.right() - sz.width() + 1)));
            p.setY(std::max(pr.top(),  std::min(p.y(), pr.bottom()- sz.height() + 1)));
        }
        g.moveTopLeft(p);
        break;
    }
    case ResizeLeft:        g.setLeft(  g.left()   + delta.x()); break;
    case ResizeRight:       g.setRight( g.right()  + delta.x()); break;
    case ResizeTop:         g.setTop(   g.top()    + delta.y()); break;
    case ResizeBottom:      g.setBottom(g.bottom() + delta.y()); break;
    case ResizeTopLeft:     g.setTopLeft(    g.topLeft()    + delta); break;
    case ResizeTopRight:    g.setTopRight(   g.topRight()   + delta); break;
    case ResizeBottomLeft:  g.setBottomLeft( g.bottomLeft() + delta); break;
    case ResizeBottomRight: g.setBottomRight(g.bottomRight()+ delta); break;
    default: break;
    }

    // 最小尺寸
    g = g.normalized();
    if (g.width()  < m_minw) g.setWidth(m_minw);
    if (g.height() < m_minh) g.setHeight(m_minh);

    QVector<QLine> guides;
    QRect snapped = applySnapping(g, &guides);
    setGeometry(snapped);
    updateGuidelines(guides);

    // 通知父窗口更新滚动区域/容器大小
    emit moved(geometry());
}

void CustomForm::mouseReleaseEvent(QMouseEvent *ev)
{
    if (ev->button() != Qt::LeftButton) return;
    m_dragMode = None;
    unsetCursor();
    updateGuidelines({});
    emit moved(geometry());
}

void CustomForm::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    updateCursorByPos(mapFromGlobal(QCursor::pos()));
    emit moved(geometry());
}

bool CustomForm::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseMove:
    case QEvent::HoverMove:
    case QEvent::Enter:
    case QEvent::Leave: {
        const QPoint localPos = mapFromGlobal(QCursor::pos());
        if (!rect().contains(localPos)) {
            unsetCursor();
            return QWidget::eventFilter(obj, event);
        }
        if (m_dragMode == None) {
            updateCursorByPos(localPos);
        }
        break;
    }
    default:
        break;
    }
    return QWidget::eventFilter(obj, event);
}

void CustomForm::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    QAction *closeAct = menu.addAction("关闭组件");
    connect(closeAct, &QAction::triggered, this, [this](){ emit requestClose(this); });
    menu.exec(event->globalPos());
}

QRect CustomForm::applySnapping(const QRect &rect, QVector<QLine> *guides) const
{
    QWidget *parent = parentWidget();
    if (!parent) {
        if (guides)
            guides->clear();
        return rect;
    }

    QRect result = rect;
    QVector<QLine> lines;

    const QRect parentRect = parent->rect();
    int gridSize = 20;
    if (auto *canvas = qobject_cast<FormCanvas*>(parent))
        gridSize = canvas->gridSize();
    if (gridSize <= 0)
        gridSize = 20;

    auto addUnique = [](QVector<int> &vec, int value) {
        if (!vec.contains(value))
            vec.append(value);
    };

    QVector<int> verticalLines;
    QVector<int> horizontalLines;

    addUnique(verticalLines, parentRect.left());
    addUnique(verticalLines, parentRect.right());
    addUnique(horizontalLines, parentRect.top());
    addUnique(horizontalLines, parentRect.bottom());

    if (gridSize > 0) {
        for (int x = parentRect.left(); x <= parentRect.right(); x += gridSize)
            addUnique(verticalLines, x);
        for (int y = parentRect.top(); y <= parentRect.bottom(); y += gridSize)
            addUnique(horizontalLines, y);
    }

    const auto siblings = parent->findChildren<CustomForm*>(QString(), Qt::FindDirectChildrenOnly);
    for (CustomForm *other : siblings) {
        if (other == this)
            continue;
        const QRect og = other->geometry();
        addUnique(verticalLines, og.left());
        addUnique(verticalLines, og.right());
        addUnique(horizontalLines, og.top());
        addUnique(horizontalLines, og.bottom());
    }

    struct SnapData {
        bool matched = false;
        int target = 0;
        int diff = 0;
    };

    auto evaluate = [&](int value, const QVector<int> &candidates) {
        SnapData data;
        for (int candidate : candidates) {
            int diff = std::abs(candidate - value);
            if (diff <= m_snapThreshold && (!data.matched || diff < data.diff)) {
                data.matched = true;
                data.target = candidate;
                data.diff = diff;
            }
        }
        return data;
    };

    const bool adjustLeft   = m_dragMode == Move || m_dragMode == ResizeLeft ||
                              m_dragMode == ResizeTopLeft || m_dragMode == ResizeBottomLeft;
    const bool adjustRight  = m_dragMode == Move || m_dragMode == ResizeRight ||
                              m_dragMode == ResizeTopRight || m_dragMode == ResizeBottomRight;
    const bool adjustTop    = m_dragMode == Move || m_dragMode == ResizeTop ||
                              m_dragMode == ResizeTopLeft || m_dragMode == ResizeTopRight;
    const bool adjustBottom = m_dragMode == Move || m_dragMode == ResizeBottom ||
                              m_dragMode == ResizeBottomLeft || m_dragMode == ResizeBottomRight;

    if (m_dragMode == Move) {
        SnapData leftSnap = evaluate(result.left(), verticalLines);
        SnapData rightSnap = evaluate(result.right(), verticalLines);
        if (leftSnap.matched || rightSnap.matched) {
            int shiftX = 0;
            if (leftSnap.matched && (!rightSnap.matched || leftSnap.diff <= rightSnap.diff)) {
                shiftX = leftSnap.target - result.left();
                lines.append(QLine(leftSnap.target, parentRect.top(), leftSnap.target, parentRect.bottom()));
            } else if (rightSnap.matched) {
                shiftX = rightSnap.target - result.right();
                lines.append(QLine(rightSnap.target, parentRect.top(), rightSnap.target, parentRect.bottom()));
            }
            result.translate(shiftX, 0);
        }

        SnapData topSnap = evaluate(result.top(), horizontalLines);
        SnapData bottomSnap = evaluate(result.bottom(), horizontalLines);
        if (topSnap.matched || bottomSnap.matched) {
            int shiftY = 0;
            if (topSnap.matched && (!bottomSnap.matched || topSnap.diff <= bottomSnap.diff)) {
                shiftY = topSnap.target - result.top();
                lines.append(QLine(parentRect.left(), topSnap.target, parentRect.right(), topSnap.target));
            } else if (bottomSnap.matched) {
                shiftY = bottomSnap.target - result.bottom();
                lines.append(QLine(parentRect.left(), bottomSnap.target, parentRect.right(), bottomSnap.target));
            }
            result.translate(0, shiftY);
        }
    } else {
        if (adjustLeft) {
            SnapData leftSnap = evaluate(result.left(), verticalLines);
            if (leftSnap.matched) {
                result.setLeft(leftSnap.target);
                lines.append(QLine(leftSnap.target, parentRect.top(), leftSnap.target, parentRect.bottom()));
            }
        }
        if (adjustRight) {
            SnapData rightSnap = evaluate(result.right(), verticalLines);
            if (rightSnap.matched) {
                result.setRight(rightSnap.target);
                lines.append(QLine(rightSnap.target, parentRect.top(), rightSnap.target, parentRect.bottom()));
            }
        }
        if (adjustTop) {
            SnapData topSnap = evaluate(result.top(), horizontalLines);
            if (topSnap.matched) {
                result.setTop(topSnap.target);
                lines.append(QLine(parentRect.left(), topSnap.target, parentRect.right(), topSnap.target));
            }
        }
        if (adjustBottom) {
            SnapData bottomSnap = evaluate(result.bottom(), horizontalLines);
            if (bottomSnap.matched) {
                result.setBottom(bottomSnap.target);
                lines.append(QLine(parentRect.left(), bottomSnap.target, parentRect.right(), bottomSnap.target));
            }
        }
    }

    if (result.left() < parentRect.left())
        result.moveLeft(parentRect.left());
    if (result.top() < parentRect.top())
        result.moveTop(parentRect.top());
    if (result.right() > parentRect.right())
        result.moveRight(parentRect.right());
    if (result.bottom() > parentRect.bottom())
        result.moveBottom(parentRect.bottom());

    if (guides)
        *guides = lines;
    return result;
}

void CustomForm::updateGuidelines(const QVector<QLine> &guides)
{
    if (auto *canvas = qobject_cast<FormCanvas*>(parentWidget())) {
        if (guides.isEmpty())
            canvas->clearGuidelines();
        else
            canvas->setGuidelines(guides);
    }
}

CustomForm::DragMode CustomForm::hitTest(const QPoint &p) const
{
    const bool left   = p.x() < m_margin;
    const bool right  = p.x() > width() - m_margin;
    const bool top    = p.y() < m_margin;
    const bool bottom = p.y() > height() - m_margin;

    if (left && top)     return ResizeTopLeft;
    if (right && top)    return ResizeTopRight;
    if (left && bottom)  return ResizeBottomLeft;
    if (right && bottom) return ResizeBottomRight;
    if (left)            return ResizeLeft;
    if (right)           return ResizeRight;
    if (top)             return ResizeTop;
    if (bottom)          return ResizeBottom;

    return Move;
}

void CustomForm::updateCursorByPos(const QPoint &pos)
{
    switch (hitTest(pos)) {
    case ResizeTopLeft:
    case ResizeBottomRight:
        setCursor(Qt::SizeFDiagCursor); break;
    case ResizeTopRight:
    case ResizeBottomLeft:
        setCursor(Qt::SizeBDiagCursor); break;
    case ResizeLeft:
    case ResizeRight:
        setCursor(Qt::SizeHorCursor); break;
    case ResizeTop:
    case ResizeBottom:
        setCursor(Qt::SizeVerCursor); break;
    default:
        unsetCursor();
        break;
    }
}

void CustomForm::clearAncestorCursors()
{
    QWidget *p = parentWidget();
    while (p) {
        p->unsetCursor();
        p = p->parentWidget();
    }
}

void CustomForm::setMouseTrackingRecursive(QWidget *w, bool on)
{
    if (!w) return;
    w->setMouseTracking(on);
    const auto children = w->findChildren<QWidget*>();
    for (QWidget *c : children) c->setMouseTracking(on);
}

void CustomForm::installCursorEventFilterRecursive(QWidget *w)
{
    if (!w) return;
    this->installEventFilter(this);
    const auto children = w->findChildren<QWidget*>();
    for (QWidget *c : children) c->installEventFilter(this);
}
