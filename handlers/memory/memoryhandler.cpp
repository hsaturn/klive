#include "memoryhandler.h"

static MemoryHandler instance;

MemoryHandler::MemoryHandler() : Handler("Memory")
  , viewer(nullptr)
{

}

MemoryHandler::~MemoryHandler()
{
    // TODO unique_ptr or shared_ptr
}

void MemoryHandler::initialize(MainWindow *main)
{
    QWidget* widget = new QWidget();
    viewer = new MemoryViewer(widget);
    main->createDockWindow(this, viewer,"Hexa");
}
