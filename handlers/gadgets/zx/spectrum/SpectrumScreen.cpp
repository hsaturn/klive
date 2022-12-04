#include "SpectrumScreen.h"
#include <stdlib.h>

#include <QPaintEvent>
#include <QPainter>
#include <QPalette>

#include <iostream>

static uint32_t colors[16]=
{   // 0        1           2           3           4           5           6           7
    0xFF000000, 0xFF000080, 0xFF800000, 0xFF800080, 0xFF008000, 0xFF008080, 0xFF808000, 0xFF808080,
    0xFF000000, 0xFF0000FF, 0xFFFF0000, 0xFFFF00FF, 0xFF00FF00, 0xFF00FFFF, 0xFFFFFF00, 0xFFFFFFFF
};

SpectrumScreen::SpectrumScreen(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StaticContents);
}

void SpectrumScreen::setMemory(hw::Memory *new_memory)
{
    if (memory) memory->detach(this);

    memory = new_memory;

    if (memory) memory->attach(this);
    // Todo redraw screen with this new memory
}

void SpectrumScreen::update(hw::Memory *mem, const Memory::Message& msg)
{
    constexpr int dx=20;
    constexpr int dy=20;
    if (msg.start<16384 || msg.start>=(16384+6912))
        return;

    for(auto addr = msg.start; addr < msg.start+msg.size; addr++)
    {
        if (addr < 16384+6144) // pixels
        {
            int start = addr-16384;
            int x = 8*(start%32);
            int y = 64*(start / 2048)+8*((start%256)/32) +(start%2048)/256;
            uint16_t pixels=mem->peek(addr);

            Memory::addr_t attr_ptr=16384+6144+int(x/8)+32*int(y/8);
            int attr=mem->peek(attr_ptr);
            int bright=attr&0x40 ? 8 : 0;   // Bright

            uint32_t on_color  = colors[(attr & 7) + bright];
            uint32_t off_color = colors[((attr>>3) & 7) + bright];

            x+=dx;
            y+=dy;  // Margin (todo, center)
            for(int z=0; z<8; z++)
            {
                // TODO use .bits() for performance
                // -> see setPixel help
                image.setPixel(x++,y, pixels & 0x80 ? on_color : off_color);
                pixels <<= 1;
            }
            //rect.moveCenter(QPoint(x, y));

        }
        else // Color attributes
        {
            int attr=mem->peek(addr);
            int start = addr-16384-6144;
            int y=8*(start/32);
            int x=8*(start%32);
            if (x+y==0)
                std::cout << (int)attr << std::endl;
            int bright=attr&0x40 ? 8 : 0;   // Bright

            uint32_t on_color  = colors[(attr & 7) + bright];
            uint32_t off_color = colors[((attr>>3) & 7) + bright];

            Memory::addr_t hl = 16384+2048*(y/64)+32*((y%64)/8)+256*(y%8)+x/8;

            x+=dx;
            y+=dy;  // Margin (todo, center)
            for(int yy=y; yy<y+8; yy++)
            {
                uint16_t pixels=mem->peek(hl);
                hl+=256;
                for(int xx=x; xx<x+8; xx++)
                {
                    image.setPixel(xx,yy, pixels & 0x80 ? on_color : off_color);
                    pixels <<= 1;
                }
            }
        }
    }
    QWidget::update();
    // QRectF rect(x, y, 8,1);
    // TODO paint what has been modified only...
    // QWidget::update(rect.toRect());
}

void SpectrumScreen::observableDies(const Memory *)
{
    memory=nullptr;
}

void SpectrumScreen::resizeImage(QImage *image, const QSize &newSize)
{
    if (image->size() == newSize)
        return;

    QImage newImage(newSize, QImage::Format_RGB32);
    newImage.fill(qRgb(192, 192, 192));
    QPainter painter(&newImage);
    painter.drawImage(QPoint(0, 0), *image);
    *image = newImage;
}

void SpectrumScreen::resizeEvent(QResizeEvent *event)
{
    if (width() > image.width() || height() > image.height()) {
        int newWidth = qMax(width() + 128, image.width());
        int newHeight = qMax(height() + 128, image.height());
        resizeImage(&image, QSize(newWidth, newHeight));
        QWidget::update();
    }
    QWidget::resizeEvent(event);
}

void SpectrumScreen::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    const QRect rect = event->rect();
    painter.scale(2,2);
    painter.drawImage(rect.topLeft(), image, rect);
}

SpectrumScreen::~SpectrumScreen()
{
    setMemory(nullptr);
}
