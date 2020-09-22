#pragma once

#include <QWidget>

#include "linebouncer.h"
#include <list>

class Screen : public QWidget
{
    Q_OBJECT

public:
    Screen(QWidget* parent = nullptr);
    ~Screen() = default;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    LineBouncer bouncer;
    std::list<LineBouncer*> lines;
    QImage image;

};
