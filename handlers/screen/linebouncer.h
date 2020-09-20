#pragma once#include <QWidget>
#include <QPaintEvent>
#include <QWidget>
#include <QPoint>

#include <list>

class LineBouncer
{
public:
    LineBouncer(QWidget*, int min_speed=5, int max_speed=9);
    void paintEvent(QPaintEvent* event);

private:
    void advance(QPoint& p, QPoint& speed);
    int delta();
    QPoint p1, p2;
    QPoint d1, d2;

    std::list<QLine> lines;
    QWidget* widget;
    int min_speed;
    int max_speed;
    QColor color;
};
