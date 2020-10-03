#pragma once

#include <common/observer.h>
#include <core/hardware/memory.h>

#include <QWidget>
#include <QPoint>
#include <QLine>

#include <list>

using hw::Memory;

class SpectrumScreen : public QWidget, public Observer<Memory>
{
    Q_OBJECT

public:
    SpectrumScreen(QWidget* parent = nullptr);
    virtual ~SpectrumScreen();

    virtual void update(Memory* sender, const Memory::Message& msg) override;
    virtual void observableDies(const Memory* sender) override;
    void resizeEvent(QResizeEvent *event) override;

    void paintEvent(QPaintEvent *event) override;

    void setMemory(hw::Memory* new_memory);

private:
    void resizeImage(QImage *image, const QSize &newSize);
    hw::Memory* memory=nullptr;   // TODO should not be a shared_ptr (crashes expected later)
    QImage image;
};
