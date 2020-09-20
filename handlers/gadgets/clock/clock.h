#pragma once

#include <QWidget>

class Clock : public QWidget
{
    Q_OBJECT

public:
    Clock(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

};
