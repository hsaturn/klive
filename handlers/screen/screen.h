#pragma once

#include <QWidget>

class Screen : public QWidget
{
    Q_OBJECT

public:
    Screen(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

};
