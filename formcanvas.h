#pragma once

#include <QWidget>
#include <QVector>
#include <QLine>

class FormCanvas : public QWidget
{
    Q_OBJECT
public:
    explicit FormCanvas(QWidget *parent = nullptr);

    void setGuidelines(const QVector<QLine> &lines);
    void clearGuidelines();

    int gridSize() const { return m_gridSize; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<QLine> m_guidelines;
    const int m_gridSize = 20;
};
