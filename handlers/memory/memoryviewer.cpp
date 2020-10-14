#include "memoryviewer.h"

#include <QPainter>
#include <QScrollBar>

MemoryViewer::MemoryViewer(QWidget *parent) : QAbstractScrollArea(parent)
{
    lastModified.start=0;
    lastModified.size=0;
    init();
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this,
            &MemoryViewer::adjustContent);
    connect(horizontalScrollBar(), &QScrollBar::valueChanged, this,
            &MemoryViewer::adjustContent);
}

MemoryViewer::~MemoryViewer() { setMemory(nullptr); }

void MemoryViewer::init() {
  nBlockAddress = 1;
  mBytesPerLine = 16;

  pxWidth = fontMetrics().horizontalAdvance('0');
  pxHeight = fontMetrics().height();
}

int MemoryViewer::addressWidth()
{
  return (nBlockAddress * 4 + nBlockAddress - 1) * pxWidth;
}

void MemoryViewer::setMemory(Memory *mem)
{
    if (memory) memory->detach(this);
    memory=mem;
    if (memory) memory->attach(this);
    adjustContent();
}

void MemoryViewer::update(Memory* sender , const Memory::Message& msg)
{
    if (!isVisible()) return;	// Bug when it becomes visible, data is old

    centerViewOnAddress(msg.start+msg.size/2);
    lastModified = msg;
    // QString str = dataHex.mid(bPos * 2, 2).toUpper();
    adjustContent();	// TODO that can be optimized
}

void MemoryViewer::observableDies(const Memory* sender)
{
    memory = nullptr;
}

int MemoryViewer::hexWidth() { return (mBytesPerLine * 3 + 1) * pxWidth; }

int MemoryViewer::asciiWidth() { return (mBytesPerLine * 2 + 1) * pxWidth; }

QByteArray MemoryViewer::data(qint64 pos, qint64 count)
{
  QByteArray buffer;
  if (memory == nullptr)
      return buffer;

  while(pos<memory->size())
  {
      buffer.append(memory->peek(pos++));
      if (count--==0) break;
  }
  return buffer;
}

void MemoryViewer::resizeEvent(QResizeEvent *) { adjustContent(); }

void MemoryViewer::paintEvent(QPaintEvent *) {
  QPainter painter(viewport());

  int offsetX = horizontalScrollBar()->value();

  int y = pxHeight;
  QString address;

  painter.setPen(viewport()->palette().color(QPalette::WindowText));

  int yFocus = -1;

  auto addr = startPos;
  for (int row = 0; row <= dataVisible.size() / mBytesPerLine; row++)
  {
        if (memory==nullptr or addr >= memory->size()) break;

    if (addr <= focusAddr)
    {
        yFocus=-1;
    } else if (yFocus == -1)
        yFocus = y-pxHeight;
    QString str = QString("%1")
                      .arg(addr, nBlockAddress * 4,
                           16, QChar('0'))
                      .toUpper();
    int i = 0;
    address = "";
    while (i < nBlockAddress) {
      address += str.mid(i * 4, 4) + ":";
      i++;
    }
    address.remove(address.size() - 1, 1);

    painter.drawText(pxWidth / 2 - offsetX, y, address);
    y += pxHeight;
    addr += mBytesPerLine;
  }

  int x;
  int lx = addressWidth() + pxWidth;
  painter.drawLine(lx - offsetX, 0, lx - offsetX, height());
  lx += pxWidth / 2;

  // hex data
  x = lx - offsetX + 3 * pxWidth;
  int w = 3 * pxWidth;
  for (int col = 0; col < mBytesPerLine / 2; col++) {
    painter.fillRect(x - pxWidth / 2, 0, w, height(),
                     viewport()->palette().color(QPalette::AlternateBase));
    x += 6 * pxWidth;
  }
  yFocus++;
  painter.drawLine(0, yFocus, width(), yFocus);
  painter.drawLine(0, yFocus-pxHeight, width() , yFocus-pxHeight);

  y = pxHeight;
  int bPos = 0;
  bool red=false;
  for (int row = 0; row < nRowsVisible; row++) {
    x = lx - offsetX;
    for (int col = 0; (col < mBytesPerLine) && (bPos < dataHex.size()); col++) {
      QString str = dataHex.mid(bPos * 2, 2).toUpper();
      if (colorLayer.size() > bPos && colorLayer.at(bPos)=='X')
      {
          if (red==false) painter.setPen(QColor(Qt::red));
          red = true;
      }
      else if (red)
      {
        red = false;
        painter.setPen(viewport()->palette().color(QPalette::WindowText));
      }
      painter.drawText(x, y, str);
      x += 3 * pxWidth;
      bPos += 1;
    }
    y += pxHeight;
  }

  lx = addressWidth() + hexWidth();
  painter.drawLine(lx - offsetX, 0, lx - offsetX, height());

  lx += pxWidth / 2;

  bPos = 0;
  y = pxHeight;
  int ch;
  for (int row = 0; row < nRowsVisible; row++) {
    x = lx - offsetX;
    for (int col = 0; (col < mBytesPerLine) && (bPos < dataVisible.size());
         col++) {
      ch = (uchar)dataVisible.at(bPos);
      if (ch < 0x20)
        ch = '.';
      painter.drawText(x, y, QChar(ch));
      x += 2 * pxWidth;
      bPos += 1;
    }
    y += pxHeight;
  }
}

void MemoryViewer::centerViewOnAddress(qint64 addr)
{
  focusAddr = addr;
  if (mBytesPerLine==0) return;
  if (memory==nullptr) return;

  uint32_t size=memory->size();
  int lineCount = size / mBytesPerLine;
  int range = lineCount-nRowsVisible;
  int row = addr / mBytesPerLine;
  row -= nRowsVisible/2;
  long first = row*range/(lineCount-nRowsVisible);
  verticalScrollBar()->setSliderPosition(first);
}

void MemoryViewer::adjustContent()
{
  uint32_t size=memory ? memory->size() : 65536;
  int w = addressWidth() + hexWidth() + asciiWidth();
  horizontalScrollBar()->setRange(0, w - viewport()->width());
  horizontalScrollBar()->setPageStep(viewport()->width());

  nRowsVisible = viewport()->height() / pxHeight;
  int val = verticalScrollBar()->value();
  startPos = (qint64)val * mBytesPerLine;
  endPos = startPos + nRowsVisible * mBytesPerLine - 1;

  int lineCount = size / mBytesPerLine;
  verticalScrollBar()->setRange(0, lineCount - nRowsVisible);
  verticalScrollBar()->setPageStep(nRowsVisible);

  if (endPos >= size)
  {
    endPos = size - 1;
  }
  dataVisible = data(startPos, endPos - startPos + mBytesPerLine + 1);
  dataHex = dataVisible.toHex();

  colorLayer = dataVisible;
  qint64 pos=lastModified.start-startPos;
  uint32_t count=lastModified.size;
  while(count && pos>=0 && pos < colorLayer.size())
  {
      colorLayer.replace(pos++, 1, "X");
      count--;
  }

  viewport()->update();
}

