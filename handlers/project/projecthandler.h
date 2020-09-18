#pragma once
#include <core/handler.h>

class ProjectHandler : public Handler
{
public:
    ProjectHandler();

    virtual void initialize(MainWindow*) override;

private:
    QMenu* menu;
};


