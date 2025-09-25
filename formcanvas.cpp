#include "formcanvas.h"

#include <QPainter>
#include <QPaintEvent>
#include <QColor>
#include <QPen>

FormCanvas::FormCanvas(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAutoFillBackground(true);
}

void FormCanvas::setGuidelines(const QVector<QLine> &lines)
{
    if (m_guidelines == lines)
        return;
    m_guidelines = lines;
    update();
}

void FormCanvas::clearGuidelines()
{
    if (m_guidelines.isEmpty())
        return;
    m_guidelines.clear();
    update();
}

void FormCanvas::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    // 背景网格
    painter.fillRect(rect(), QColor("#1e1e1f"));

    QPen gridPen(QColor(255, 255, 255, 30));
    gridPen.setWidth(1);
    painter.setPen(gridPen);

    for (int x = 0; x <= width(); x += m_gridSize)
        painter.drawLine(x, 0, x, height());
    for (int y = 0; y <= height(); y += m_gridSize)
        painter.drawLine(0, y, width(), y);

    if (!m_guidelines.isEmpty()) {
        QPen guidePen(QColor(66, 133, 244, 180));
        guidePen.setWidth(2);
        painter.setPen(guidePen);
        for (const QLine &line : m_guidelines)
            painter.drawLine(line);
    }
}
