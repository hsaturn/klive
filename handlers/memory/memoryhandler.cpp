#include "memoryhandler.h"

static MemoryHandler instance;

MemoryHandler::MemoryHandler() : Handler("Memory")
  , viewer(nullptr)
{

}

MemoryHandler::~MemoryHandler()
{
    // TODO unique_ptr
    if (viewer)
        delete viewer;
    viewer=nullptr;
}

void MemoryHandler::initialize(MainWindow *main)
{
    QWidget* widget = new QWidget();
    viewer = new MemoryViewer(widget);
    main->createDockWindow(this, viewer);
}
