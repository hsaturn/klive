#pragma once

#include <QWidget>
#include <QPoint>
#include <QLine>

#include "linebouncer.h"
#include <list>

class Screen : public QWidget
{
    Q_OBJECT

public:
    Screen(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    LineBouncer bouncer;
    std::list<LineBouncer*> lines;
};
