#pragma once
#include <core/handler.h>

#include "memoryviewer.h"

class MemoryHandler : public Handler
{
public:
    MemoryHandler();
    virtual ~MemoryHandler();

    virtual void initialize(MainWindow *) override;

private:
    MemoryViewer* viewer;
};


