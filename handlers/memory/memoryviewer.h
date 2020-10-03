#pragma once

#include <core/hardware/memory.h>
#include <QAbstractScrollArea>
#include <QBuffer>
#include <common/observer.h>

using hw::Memory;

class MemoryViewer : public QAbstractScrollArea, public Observer<Memory> {
  Q_OBJECT
public:
  MemoryViewer(QWidget *parent = 0);
  ~MemoryViewer();

  void update(qint64 addr, uint32_t size);
  void setMemory(Memory*);

  virtual void update(Memory* sender, const Memory::Message& msg);
  virtual void observableDies(const Memory* sender);

protected:
  void paintEvent(QPaintEvent *);
  void resizeEvent(QResizeEvent *);

private:
  void adjustContent();
  void init();

  int addressWidth();
  int hexWidth();
  int asciiWidth();

  QByteArray data(qint64 pos = 0, qint64 count = -1);

  int nBlockAddress;
  int mBytesPerLine;

  int pxWidth;
  int pxHeight;

  qint64 startPos;
  qint64 endPos;

  int nRowsVisible;

  QByteArray dataVisible;
  QByteArray colorLayer;
  QByteArray dataHex;
  Memory* memory = nullptr;
  Memory::Message lastModified;
};

