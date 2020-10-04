#include "linebouncer.h"

#include <QPainter>

int LineBouncer::delta()
{
    int d;

    do
    {
        d=(rand()%max_speed)-(rand()%max_speed);
    } while(d<min_speed);
    return d;
}

LineBouncer::LineBouncer(QWidget* widget, int min_speed_, int max_speed_)
    : widget(widget),
      min_speed(min_speed_),
      max_speed(max_speed_)
{
    if (min_speed > max_speed)
    {
        int speed=min_speed;
        min_speed=max_speed;
        min_speed=speed;
    }
    p1 = QPoint(rand()%100, rand()%100);
    p2 = QPoint(rand()%100, rand()%100);
    d1 = QPoint(delta(), delta());
    d2 = QPoint(delta(), delta());

    color = QColor(rand()%255, rand()%255, rand()%255);
}

void LineBouncer::advance(QPoint& p, QPoint& speed)
{
    p+=speed;
    if (p.x()>=widget->width())
        speed.setX(-abs(speed.x()));
    else if (p.x()<=0)
        speed.setX(abs(speed.x()));

    if (p.y()>=widget->height())
        speed.setY(-abs(speed.y()));
    else if (p.y()<=0)
        speed.setY(abs(speed.y()));
}

void LineBouncer::paintEvent(QPaintEvent*)
{
    QPainter painter(widget);

    if (lines.size()>30)
    {
        painter.setBrush(QColor(255,255,255));
        lines.pop_back();
    }

    advance(p1, d1);
    advance(p2, d2);

    painter.setPen(QPen(color, 1));
    lines.push_front(QLine(p1, p2));

    for(const auto& line: lines)
        painter.drawLine(line);
    painter.end();
}
