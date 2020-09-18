#include "projecthandler.h"

static ProjectHandler handler1;

ProjectHandler::ProjectHandler() : Handler("Project")
{

}

void ProjectHandler::initialize(MainWindow *main)
{
    main->addMenuEntry("Project", "Open");
    main->addMenuEntry("Project", "Close");
}
