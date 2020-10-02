#pragma once

#include <QWidget>

#include "linebouncer.h"
#include <list>

class Lines : public QWidget
{
    Q_OBJECT

public:
    Lines(QWidget* parent = nullptr);
    ~Lines() = default;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    LineBouncer bouncer;
    std::list<LineBouncer*> lines;
    QImage image;

};
