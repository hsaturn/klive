#ifndef GENS_H
#define GENS_H

#include <QWidget>

namespace Ui {
class Gens;
}

class Gens : public QWidget
{
    Q_OBJECT

public:
    explicit Gens(QWidget *parent = nullptr);
    ~Gens();

private:
    Ui::Gens *ui;
};

#endif // GENS_H
