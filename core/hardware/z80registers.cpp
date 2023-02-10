#include "z80registers.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

#include <QTableView>
#include <QHeaderView>
#include <QFontDatabase>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLabel>

using namespace std;

namespace hw
{

static int firstRow=0;

FlagWidget::FlagWidget(const char* text, reg8& flags_, uint8_t mask_)
    : flags(flags_), mask(mask_)
{
    label = new QLabel(text);
    addWidget(label);
    checkbox = new QCheckBox;
    addWidget(checkbox);

    connect(checkbox, &QCheckBox::stateChanged, this, &FlagWidget::onClick);
}

void FlagWidget::onClick()
{
    if (disableSignal)
        return;

    emit stateChanged();
}

void FlagWidget::repaint()
{
    QPalette pal = label->palette();

    bool old = checkbox->isChecked();
    if (old != (flags & mask))
        pal.setColor(QPalette::WindowText, Qt::red);
    else
        pal.setColor(QPalette::WindowText, Qt::black);
    label->setPalette(pal);
}

void FlagWidget::update()
{
    disableSignal = true;
    repaint();
    checkbox->setChecked(flags & mask);
    disableSignal = false;
}

void addCells(QTableView* table, int col, std::string list)
{
    int row=firstRow;

    auto model=dynamic_cast<QStandardItemModel*>(table->model());
    while(list.length())
    {
        auto pos=list.find(',');
        if (pos==string::npos)
            pos=list.length();
        string reg=list.substr(0,pos)+':';
        list.erase(0, pos+1);

        QStandardItem *newItem;
        newItem = new QStandardItem(reg.c_str());
        model->setItem(row, col, newItem);
        row++;
    }
}

bool Z80Registers::set(string reg,int32_t value)
{
    std::for_each(reg.begin(), reg.end(), [](char&c){ c=tolower(c); });

    if (reg=="af") af.val=static_cast<uint16_t>(value);
    else if (reg=="sp") sp=static_cast<uint16_t>(value);
    else if (reg=="pc") pc=static_cast<uint16_t>(value);
    else if (reg=="bc") bc.val=static_cast<uint16_t>(value);
    else if (reg=="de") de.val=static_cast<uint16_t>(value);
    else if (reg=="hl") hl.val=static_cast<uint16_t>(value);
    else if (reg=="af'") af2.val=static_cast<uint16_t>(value);
    else if (reg=="bc'") bc2.val=static_cast<uint16_t>(value);
    else if (reg=="de'") de2.val=static_cast<uint16_t>(value);
    else if (reg=="hl'") hl2.val=static_cast<uint16_t>(value);
    else if (reg=="ix") ix.val=static_cast<uint16_t>(value);
    else if (reg=="iy") iy.val=static_cast<uint16_t>(value);
    else if (reg=="a") a=static_cast<reg8>(value);
    else if (reg=="f") f.f=static_cast<reg8>(value);
    else if (reg=="b") b=static_cast<reg8>(value);
    else if (reg=="c") c=static_cast<reg8>(value);
    else if (reg=="d") d=static_cast<reg8>(value);
    else if (reg=="e") e=static_cast<reg8>(value);
    else if (reg=="h") h=static_cast<reg8>(value);
    else if (reg=="l") l=static_cast<reg8>(value);
    else if (reg=="i") i=static_cast<reg8>(value);
    else if (reg=="r") r=static_cast<reg8>(value);
    else return false;

    update();
    return true;
}

std::string Z80Registers::serialize() const
{
    std::stringstream out;

    out << hex << showbase << '{';
    out << " sp:" << sp;
    out << " ix:" << ix.val;
    out << " iy:" << iy.val;
    out << " af:" << af.val;
    out << " bc:" << bc.val;
    out << " de:" << de.val;
    out << " hl:" << hl.val;
    out << " af':" << af2.val;
    out << " bc':" << bc2.val;
    out << " de':" << de2.val;
    out << " hl':" << hl2.val;
    out << " i:" << (uint16_t)i;
    out << " r:" << (uint16_t)i;

    out << '}';

    return out.str();
}

uint16_t Z80Registers::get(string regi)
{
    string reg(regi);	// TODO ...
    std::for_each(reg.begin(), reg.end(), [](char& chr){ chr=tolower(chr); });

    if (reg=="af") return af.val;
    else if (reg=="sp") return sp;
    else if (reg=="pc") return pc;
    else if (reg=="bc") return bc.val;
    else if (reg=="de") return de.val;
    else if (reg=="hl") return hl.val;
    else if (reg=="af'") return af2.val;
    else if (reg=="bc'") return bc2.val;
    else if (reg=="de'") return de2.val;
    else if (reg=="hl'") return hl2.val;
    else if (reg=="ix") return ix.val;
    else if (reg=="iy") return iy.val;
    else if (reg=="a") return a;
    else if (reg=="f") return f.f;
    else if (reg=="b") return b;
    else if (reg=="c") return c;
    else if (reg=="d") return d;
    else if (reg=="e") return e;
    else if (reg=="h") return h;
    else if (reg=="l") return l;
    else if (reg=="i") return i;
    else if (reg=="r") return r;
    else if (reg=="carry") return f.c;
    else if (reg=="zero") return f.z;
    else if (reg=="pv") return f.pv;
    else if (reg=="sign") return f.s;
    else throw "Unknown register";
}

void Z80RegisterWidgets::onFlagChange()
{
    cout << "ON FLAG CHANGE (AT LEAST)" << endl;
}

void Z80RegisterWidgets::addFlag(FlagWidget* flag)
{
    flagsLayout->addLayout(flag);
    flags.push_back(flag);
    connect(flag, &FlagWidget::stateChanged, this, &Z80RegisterWidgets::onFlagChange);
}


Z80RegisterWidgets::Z80RegisterWidgets(Z80Registers* regs)
    : r(regs)
{
    QVBoxLayout* layout=new QVBoxLayout;
    setLayout(layout);

    table=new QTableView;

    auto* model=new QStandardItemModel();
    table->horizontalHeader()->hide();
    table->verticalHeader()->hide();
    table->setModel(model);

    // TODO part of this should be user configuration
    QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    // fixedFont.setPointSize(9);
    table->setShowGrid(false);
    table->setFont(fixedFont);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    table->verticalHeader()->setDefaultSectionSize(12);
    // table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    addCells(table, 0, "PC,AF,BC,DE,HL,IX,I");
    addCells(table, 2, "SP,AF',BC',DE',HL',IY,R");

    layout->addWidget(table);

    { // flags
        flagsLayout  = new QHBoxLayout;

        QLabel* label;
        label=new QLabel;
        label->setText("Flags:");
        flagsLayout->addWidget(label);

        //  ----

        addFlag(new FlagWidget("S", r->f.f, 0x80));
        addFlag(new FlagWidget("Z", r->f.f, 0x40));
        addFlag(new FlagWidget("5", r->f.f, 0x20));
        addFlag(new FlagWidget("H", r->f.f, 0x10));
        addFlag(new FlagWidget("3", r->f.f,  0x8));
        addFlag(new FlagWidget("PV", r->f.f, 0x4));
        addFlag(new FlagWidget("N", r->f.f,  0x2));
        addFlag(new FlagWidget("C", r->f.f,  0x1));

        layout->addLayout(flagsLayout);
    }
}

template<class T>
static void setCell(QStandardItemModel* model, T& reg, int& row, int& col)
{
static    int counter=0;
    stringstream r;
    r << hex << setw(4) << std::setfill('0') << showbase << reg;
    QString str=QString::fromStdString(r.str());

    QStandardItem *newItem;
    newItem = new QStandardItem(str);
    if (model->index(row, col).data().toString()!=str)
    {
        newItem->setData(QColor(Qt::red), Qt::ForegroundRole);
        counter++;
        // old value  : model->index(row, col).data().toString()
    }

    model->setItem(row, col, newItem);
    row++;
}

static void setCells(QTableView* table, int col, reg16& top, regaf& af, reg16u& bc, reg16u& de, reg16u& hl, reg16u& ii, reg8& bottom)
{
    auto model=dynamic_cast<QStandardItemModel*>(table->model());
    int row=firstRow;
    setCell(model, top, row, col);
    setCell(model, af, row, col);
    setCell(model, bc, row, col);
    setCell(model, de, row, col);
    setCell(model, hl, row, col);
    setCell(model, ii, row, col);
    setCell(model, bottom, row, col);
}

void Z80RegisterWidgets::update()
{
    if (table == nullptr) return;
    if (!isVisible()) return;

    setCells(table, 1, r->pc, r->af,  r->bc,  r->de,  r->hl,  r->ix, r->i);
    setCells(table, 3, r->sp, r->af2, r->bc2, r->de2, r->hl2, r->iy, r->r);

    for(FlagWidget* flag : flags)
        flag->update();
}

Z80RegisterWidgets* Z80Registers::createViewForm(QWidget *parent)
{
    if (view == nullptr)
    view = new Z80RegisterWidgets(this);

    return view;
}

static ostream& operator<<(ostream& out, const regaf& af)
{
    out << af.val;
    return out;
}

static ostream& operator<<(ostream& out, const reg16u& u)
{
    out << u.val;
    return out;
}

static ostream& operator<<(ostream& out, const reg8& u)
{
    out << (int16_t)u;
    return out;
}

// TODO why does registers contains a view on registers...
// New design needed there, we should remove this function
// and see what happens
void Z80Registers::update()
{
    view->update();
}

reg16u::reg16u()
{
    lo() = 0x34;
    hi() = 0x12;

    if (val != 0x1234)
    {
        std::cerr << "ERROR: Auto check indianess failed. The cpu won't work." << std::endl;
    }
    val=0;
}

} // ns
