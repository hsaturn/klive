#ifndef MONS_H
#define MONS_H

#include <QWidget>

namespace Ui {
class Mons;
}

class Mons : public QWidget
{
    Q_OBJECT

public:
    explicit Mons(QWidget *parent = nullptr);
    ~Mons();

private:
    Ui::Mons *ui;
};

#endif // MONS_H
