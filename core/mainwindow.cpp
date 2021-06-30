#include <QtWidgets>
#include "mainwindow.h"
#include <iostream>

using std::cout;
using std::endl;

#if defined(QT_PRINTSUPPORT_LIB)
#include <QtPrintSupport/qtprintsupportglobal.h>
#if QT_CONFIG(printdialog)
#include <QtPrintSupport>
#endif
#endif

#include "mainwindow.h"
#include "core/handler.h"
#include <iostream>
#include <algorithm>
#include <DockManager.h>
#include <DockAreaWidget.h>
#include <ads_globals.h>

using ads::CDockWidget;

MainWindow::MainWindow() : QMainWindow()
{
    ads::CDockManager::setConfigFlag(ads::CDockManager::OpaqueSplitterResize, true);
    ads::CDockManager::setConfigFlag(ads::CDockManager::XmlCompressionEnabled, false);
    ads::CDockManager::setConfigFlag(ads::CDockManager::FocusHighlighting, true);
    DockManager = new ads::CDockManager(this);

    // Set central widget
    QPlainTextEdit* w = new QPlainTextEdit();
    w->setPlaceholderText("This is the central editor. Enter your text here.");
    CDockWidget* CentralDockWidget = new CDockWidget("CentralWidget");
    CentralDockWidget->setWidget(w);
    CentralDockArea = DockManager->setCentralWidget(CentralDockWidget);
    CentralDockArea->setAllowedAreas(ads::DockWidgetArea::OuterDockAreas);

    createActions();
    createStatusBar();
   // createDockWindows();

    Handlers::initialize(this);

    setWindowTitle(tr("KLive IDE"));

    setUnifiedTitleAndToolBarOnMac(true);
    restoreLayout();
    cout << "Main window init done" << endl;
}

MainWindow::~MainWindow()
{
}

void MainWindow::print()
{
#if defined(QT_PRINTSUPPORT_LIB) && QT_CONFIG(printdialog)
    QTextDocument *document = textEdit->document();
    QPrinter printer;

    QPrintDialog dlg(&printer, this);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    document->print(&printer);
    statusBar()->showMessage(tr("Ready"), 2000);
#endif
}

void MainWindow::closeMdi()
{

}

void MainWindow::about()
{
   QMessageBox::about(this, tr("About Klive"),
            tr("The <b>Klive</b> is a ZX Spectrum IDE "
               "(c) HSaturn 2020"));
}

QAction* MainWindow::addMenuEntry(
        const std::string smenu,
        const std::string sentry,
        std::string before)
{
    if (before=="") before="Help";
    std::string menu_key(smenu);
    std::string before_key(before);
    menu_key.erase(std::remove(menu_key.begin(), menu_key.end(), '&'), menu_key.end());
    before_key.erase(std::remove(before_key.begin(), before_key.end(), '&'), before_key.end());

    QMenu* menu;
    QAction* action = nullptr;

    // TODO what happens if QMenu* diseaper between calls...
    if (menus.find(menu_key) == menus.end())
    {
        menu = new QMenu(tr(smenu.c_str()));
        menus[menu_key] = menu;

        // TODO ownership ?
        if (actions.find(before_key) == actions.end())
        {
            action = menuBar()->addMenu(menu);
            actions[menu_key] = action;
        }
        else
        {
            action = actions[before_key];
            action = menuBar()->insertMenu(action, menu);
        }
    }
    else
    {
        menu = menus[menu_key];
    }

    if (sentry.size())
    {
        if (sentry == "-")
        {
            menu->addSeparator();
        }
        else
        {
            // TODO ownership ?
            action = new QAction(tr(sentry.c_str()));
            menu->addAction(action);
        }
    }

    return action;
}

void MainWindow::createActions()
{
    std::cout << "Creating actions" << std::endl;
    // File Manager
    QToolBar *fileToolBar = addToolBar(tr("File"));

/*
 *     QMenu *projectMenu = menuBar()->addMenu(tr("&Projet"));
    QAction *newProject = new QAction( tr("&New"), this);
    projectMenu->addAction(newProject);

*/
    // const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    QAction *saveLayoutAct = addMenuEntry("&File", "&Save Layout");
    // saveLayoutAct->setShortcuts(QKeySequence::New);
    saveLayoutAct->setStatusTip(tr("Save windows layout"));
    connect(saveLayoutAct, &QAction::triggered, this, &MainWindow::saveLayout);
    fileToolBar->addAction(saveLayoutAct);

    const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
    QAction *saveAct = addMenuEntry("&File", "&Save...");
    saveAct->setIcon(saveIcon);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the current form letter"));
    // connect(saveAct, &QAction::triggered, this, &MainWindow::save);
    // fileToolBar->addAction(saveAct);

    const QIcon printIcon = QIcon::fromTheme("document-print", QIcon(":/images/print.png"));
    QAction *printAct = addMenuEntry("&File", "&Print...");
    printAct->setIcon(printIcon);
    printAct->setShortcuts(QKeySequence::Print);
    printAct->setStatusTip(tr("Print the current form letter"));
    connect(printAct, &QAction::triggered, this, &MainWindow::print);
    fileToolBar->addAction(printAct);

    // TODO Bad, because could be null ptr
    menus["File"]->addSeparator();

    // TODO fileMenu could be nullptr
    QMenu* fileMenu = menus["File"];
    QAction *quitAct = fileMenu->addAction(tr("&Quit"), this, &QWidget::close);
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setStatusTip(tr("Quit the application"));

    // QToolBar *editToolBar = addToolBar(tr("Edit"));
    const QIcon undoIcon = QIcon::fromTheme("edit-undo", QIcon(":/images/undo.png"));
    QAction *undoAct = addMenuEntry("&Edit", "&Undo");
    undoAct->setIcon(undoIcon);
    undoAct->setShortcuts(QKeySequence::Undo);
    undoAct->setStatusTip(tr("Undo the last editing action"));
    // connect(undoAct, &QAction::triggered, this, &MainWindow::undo);
    // editToolBar->addAction(undoAct);

    addMenuEntry("&View");
    // TODO could be nullptr
    viewMenu = menus["View"];

    menuBar()->addSeparator();

    addMenuEntry("&Help");
    // TODO Bad design, could be nullptr etc...
    QMenu *helpMenu = menus["Help"];

    QAction *aboutAct = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    aboutAct->setStatusTip(tr("Show the application's About box"));

    QAction *aboutQtAct = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
}

void MainWindow::saveLayout()
{
    QSettings Settings("Settings.ini", QSettings::IniFormat);
    Settings.setValue("mainWindow/Geometry", saveGeometry());
    Settings.setValue("mainWindow/State", saveState());
    Settings.setValue("mainWindow/DockingState", DockManager->saveState());
}

void MainWindow::restoreLayout()
{
    cout << "Restoring layout" << endl;
    QSettings Settings("Settings.ini", QSettings::IniFormat);
    restoreGeometry(Settings.value("mainWindow/Geometry").toByteArray());
    restoreState(Settings.value("mainWindow/State").toByteArray());
    DockManager->restoreState(Settings.value("mainWindow/DockingState").toByteArray());
}

void MainWindow::createStatusBar()
{
    std::cout << "Creating status bar" << std::endl;
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::createDockWindow(Handler*, QWidget* widget, const std::string title)
{
   ads::CDockWidget *dock = new ads::CDockWidget(tr(title.c_str()));
   dock->setWidget(widget);
   dock->resize(250, 250);
   // dock->setMinimumSize(300,250);
   // dock->setMinimumSizeHintMode(ads::CDockWidget::MinimumSizeHintFromDockWidget);
   DockManager->addDockWidget(ads::DockWidgetArea::LeftDockWidgetArea, dock, CentralDockArea);
   viewMenu->addAction(dock->toggleViewAction());
}
