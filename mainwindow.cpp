#include <QtWidgets>
#include "mainwindow.h"
#include "./ui_mainwindow.h"

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

MainWindow::MainWindow()
    : QMainWindow()
   // , ui(new Ui::MainWindow)
{
   // ui->setupUi(this);
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


    //ui->setupUi(this);
    // mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    // mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    // setCentralWidget(mdiArea);
    /*
    connect(mdiArea, &QMdiArea::subWindowActivated,
            this, &MainWindow::updateMenus);
    */
    createActions();
    createStatusBar();
   // createDockWindows();

    Handlers::initialize(this);

    setWindowTitle(tr("KLive IDE"));

    newLetter();
    setUnifiedTitleAndToolBarOnMac(true);
}


MainWindow::~MainWindow()
{
    // delete ui;
}



void MainWindow::newLetter()
{
/*
 *     textEdit->clear();

    QTextCursor cursor(textEdit->textCursor());
    cursor.movePosition(QTextCursor::Start);
    QTextFrame *topFrame = cursor.currentFrame();
    QTextFrameFormat topFrameFormat = topFrame->frameFormat();
    topFrameFormat.setPadding(16);
    topFrame->setFrameFormat(topFrameFormat);

    QTextCharFormat textFormat;
    QTextCharFormat boldFormat;
    boldFormat.setFontWeight(QFont::Bold);
    QTextCharFormat italicFormat;
    italicFormat.setFontItalic(true);

    QTextTableFormat tableFormat;
    tableFormat.setBorder(1);
    tableFormat.setCellPadding(16);
    tableFormat.setAlignment(Qt::AlignRight);
    cursor.insertTable(1, 1, tableFormat);
    cursor.insertText("The Firm", boldFormat);
    cursor.insertBlock();
    cursor.insertText("321 City Street", textFormat);
    cursor.insertBlock();
    cursor.insertText("Industry Park");
    cursor.insertBlock();
    cursor.insertText("Some Country");
    cursor.setPosition(topFrame->lastPosition());
    cursor.insertText(QDate::currentDate().toString("d MMMM yyyy"), textFormat);
    cursor.insertBlock();
    cursor.insertBlock();
    cursor.insertText("Dear ", textFormat);
    cursor.insertText("NAME", italicFormat);
    cursor.insertText(",", textFormat);
    for (int i = 0; i < 3; ++i)
        cursor.insertBlock();
    cursor.insertText(tr("Yours sincerely,"), textFormat);
    for (int i = 0; i < 3; ++i)
        cursor.insertBlock();
    cursor.insertText("The Boss", textFormat);
    cursor.insertBlock();
    cursor.insertText("ADDRESS", italicFormat);
    */
}
//! [2]

//! [3]
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
//! [3]

//! [4]
void MainWindow::save()
{
    /*
    QMimeDatabase mimeDatabase;
    QString fileName = QFileDialog::getSaveFileName(this,
                        tr("Choose a file name"), ".",
                        mimeDatabase.mimeTypeForName("text/html").filterString());
    if (fileName.isEmpty())
        return;
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Dock Widgets"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QTextStream out(&file);
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    out << textEdit->toHtml();
    QGuiApplication::restoreOverrideCursor();

    statusBar()->showMessage(tr("Saved '%1'").arg(fileName), 2000);
    */
}
//! [4]

//! [5]
void MainWindow::undo()
{
    /*
    QTextDocument *document = textEdit->document();
    document->undo();
    */
}
//! [5]

//! [6]
void MainWindow::insertCustomer(const QString &customer)
{
    /*
    if (customer.isEmpty())
        return;
    QStringList customerList = customer.split(", ");
    QTextDocument *document = textEdit->document();
    QTextCursor cursor = document->find("NAME");
    if (!cursor.isNull()) {
        cursor.beginEditBlock();
        cursor.insertText(customerList.at(0));
        QTextCursor oldcursor = cursor;
        cursor = document->find("ADDRESS");
        if (!cursor.isNull()) {
            for (int i = 1; i < customerList.size(); ++i) {
                cursor.insertBlock();
                cursor.insertText(customerList.at(i));
            }
            cursor.endEditBlock();
        }
        else
            oldcursor.endEditBlock();
    }
    */
}
//! [6]

void MainWindow::addParagraph(const QString &paragraph)
{
    /*
    if (paragraph.isEmpty())
        return;
    QTextDocument *document = textEdit->document();
    QTextCursor cursor = document->find(tr("Yours sincerely,"));
    if (cursor.isNull())
        return;
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor, 2);
    cursor.insertBlock();
    cursor.insertText(paragraph);
    cursor.insertBlock();
    cursor.endEditBlock();
    */

}

void MainWindow::dockTopLevelChanged()
{
    static QWidget* widget;
    static QMdiSubWindow* mdi;

    QDockWidget* dock = dynamic_cast<QDockWidget*>(sender());
    if (dock)
    {
        if (dock->isFloating())
        {
            std::cerr << "DOCK FLOATING" << std::endl;

            widget=dock->widget();
            // mdi = mdiArea->addSubWindow(widget);
            mdi->show();
           // connect(mdi, )
            // dock->hide();
        }
        else
        {
            std::cerr << "DOCKED" << std::endl;
            dock->setWidget(mdi->widget());
            mdi->setWidget(nullptr);
            mdi->hide();
        }
    }
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
    QAction* action;

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
    // File Manager
    QToolBar *fileToolBar = addToolBar(tr("File"));

/*
 *     QMenu *projectMenu = menuBar()->addMenu(tr("&Projet"));
    QAction *newProject = new QAction( tr("&New"), this);
    projectMenu->addAction(newProject);

*/
    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    QAction *newLetterAct = addMenuEntry("&File", "&New Letter");
    newLetterAct->setShortcuts(QKeySequence::New);
    newLetterAct->setStatusTip(tr("Create a new form letter"));
    connect(newLetterAct, &QAction::triggered, this, &MainWindow::newLetter);
    fileToolBar->addAction(newLetterAct);

    const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
    QAction *saveAct = addMenuEntry("&File", "&Save...");
    saveAct->setIcon(saveIcon);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the current form letter"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::save);
    fileToolBar->addAction(saveAct);

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

    QToolBar *editToolBar = addToolBar(tr("Edit"));
    const QIcon undoIcon = QIcon::fromTheme("edit-undo", QIcon(":/images/undo.png"));
    QAction *undoAct = addMenuEntry("&Edit", "&Undo");
    undoAct->setIcon(undoIcon);
    undoAct->setShortcuts(QKeySequence::Undo);
    undoAct->setStatusTip(tr("Undo the last editing action"));
    connect(undoAct, &QAction::triggered, this, &MainWindow::undo);
    editToolBar->addAction(undoAct);

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

//! [8]
void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}
//! [8]

//! [9]
void MainWindow::createDockWindows()
{
    {
        ads::CDockWidget *dock = new ads::CDockWidget(tr("Customers"), this);

        dock->setMinimumSizeHintMode(ads::CDockWidget::MinimumSizeHintFromDockWidget);
        dock->resize(250, 150);
        dock->setMinimumSize(200,150);
        customerList = new QListWidget(dock);
        customerList->addItems(QStringList()
                << "John Doe, Harmony Enterprises, 12 Lakeside, Ambleton"
                << "Jane Doe, Memorabilia, 23 Watersedge, Beaton"
                << "Tammy Shea, Tiblanka, 38 Sea Views, Carlton"
                << "Tim Sheen, Caraba Gifts, 48 Ocean Way, Deal"
                << "Sol Harvey, Chicos Coffee, 53 New Springs, Eccleston"
                << "Sally Hobart, Tiroli Tea, 67 Long River, Fedula");
        dock->setWidget(customerList);
        viewMenu->addAction(dock->toggleViewAction());
    }


    auto dock = new QDockWidget(tr("Paragraphs"), this);
    paragraphsList = new QListWidget(dock);
    paragraphsList->addItems(QStringList()
            << "Thank you for your payment which we have received today."
            << "Your order has been dispatched and should be with you "
               "within 28 days."
            << "We have dispatched those items that were in stock. The "
               "rest of your order will be dispatched once all the "
               "remaining items have arrived at our warehouse. No "
               "additional shipping charges will be made."
            << "You made a small overpayment (less than $5) which we "
               "will keep on account for you, or return at your request."
            << "You made a small underpayment (less than $1), but we have "
               "sent your order anyway. We'll add this underpayment to "
               "your next bill."
            << "Unfortunately you did not send enough money. Please remit "
               "an additional $. Your order will be dispatched as soon as "
               "the complete amount has been received."
            << "You made an overpayment (more than $5). Do you wish to "
               "buy more items, or should we return the excess to you?");

    dock->setWidget(paragraphsList);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());

    connect(customerList, &QListWidget::currentTextChanged,
            this, &MainWindow::insertCustomer);
    connect(paragraphsList, &QListWidget::currentTextChanged,
            this, &MainWindow::addParagraph);

    dock = new QDockWidget(tr("ASM"), this);
    paragraphsList = new QListWidget(dock);
    paragraphsList->addItems(QStringList()
            << "LD A, (HL)"
            << "DJNZ #label"
    );

    dock->setWidget(paragraphsList);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());
/*
    dock = new QDockWidget(tr("Clock"), this);
    Clock* clock = new Clock();
    dock->setWidget(clock);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());
    */
}

void MainWindow::createDockWindow(Handler* handler, QWidget* widget)
{
    {
       ads::CDockWidget *dock = new ads::CDockWidget(tr(handler->name().c_str()));
       dock->setWidget(widget);
       dock->resize(250, 150);
       dock->setMinimumSize(200,150);
       // dock->setMinimumSizeHintMode(ads::CDockWidget::MinimumSizeHintFromDockWidget);
       auto* dockArea = DockManager->addDockWidget(ads::DockWidgetArea::LeftDockWidgetArea, dock, CentralDockArea);
       viewMenu->addAction(dock->toggleViewAction());
       return;
   }

    // mdiArea->addSubWindow(widget);
    // return;
    // TODO what happens when widget is deleted ???
    QDockWidget *dock = new QDockWidget(tr(handler->name().c_str()));
    dock->setWidget(widget);
    // TODO Should be a caller's decision
    addDockWidget(Qt::RightDockWidgetArea, dock);
    connect(dock, &QDockWidget::topLevelChanged, this, &MainWindow::dockTopLevelChanged);
    viewMenu->addAction(dock->toggleViewAction());
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);

    // dock->setFloating(false);
    // mdiArea->addSubWindow(dock);
}
