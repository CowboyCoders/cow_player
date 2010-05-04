#include "piece_dialog.h"
#include "ui_piece_dialog.h"
#include <cow/cow.hpp>

#include <QMetaType>
#include <QColor>
#include <QLabel>
#include <boost/bind.hpp>
#include <iostream>
#include <cassert>

// predefined colors for different download devices
static QColor grey = QColor::fromRgb(220,220,220);
static QColor forest_green = QColor::fromRgb(0,255,30);
static QColor ruby_red = QColor::fromRgb(255,200,0); 
static QColor golden_yellow = QColor::fromRgb(255,0,150);
static QColor turqoise = QColor::fromRgb(0,206,209);

static QColor colors[] = {grey,forest_green,ruby_red,golden_yellow,turqoise};

piece_dialog::piece_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::piece_dialog)
{
    ui->setupUi(this);
    connect(this,SIGNAL(piece_downloaded(int,int)),ui->pieceWidget,SLOT(piece_downloaded(int,int)));
    qRegisterMetaType<device_map>("device_map");
}

piece_dialog::~piece_dialog()
{


    delete ui;
}

void piece_dialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void piece_dialog::set_download_control(libcow::download_control* control)
{
    // Extract current pieces states from download control
    std::vector<int> state;
    control->get_current_state(state);

    std::map<int, std::string> devices = control->get_device_names();

    set_legend(devices);

    ui->pieceWidget->set_piece_states(state);
    ui->pieceWidget->set_device_map(devices);
    ui->pieceWidget->set_colors(std::vector<QColor>(colors, colors + sizeof(colors)/sizeof(QColor)));

    // Register callback for piece finished event
    control->set_piece_finished_callback(boost::bind(&piece_dialog::piece_downloaded_callback,this,_1,_2));    
}

void piece_dialog::piece_downloaded_callback(int piece_idx, int device)
{
    emit piece_downloaded(piece_idx, device);
}

void piece_dialog::set_legend(const std::map<int, std::string>& devices) 
{
    assert(sizeof(colors)/sizeof(QColor) > 0);

    std::vector<QWidget*>::iterator wit = legend_items_.begin();
    for (; wit != legend_items_.end(); ++wit) {
        delete *wit;
    }

    legend_items_.clear();

    std::map<int, std::string>::const_iterator it = devices.begin();
    for (int i = 0; it != devices.end(); ++it, ++i) {
        int ind = i % (sizeof(colors)/sizeof(QColor));
        QFrame* item = create_legend_item(it->second, colors[ind]);
        legend_items_.push_back(item);
        ui->legend->layout()->addWidget(item);
    }
}

QFrame* piece_dialog::create_legend_item(const std::string& label, const QColor& color)
{
    QFrame* frame = new QFrame;

    QHBoxLayout* horizontalLayout = new QHBoxLayout(frame);
    horizontalLayout->setContentsMargins(0, 0, 20, 0);

    QFrame* color_box = new QFrame(frame);
    color_box->setMinimumSize(QSize(10, 10));
    color_box->setMaximumSize(QSize(10, 10));
    color_box->setBaseSize(QSize(10, 10));
    color_box->setStyleSheet(QString::fromUtf8("margin-top:1px;"));
    color_box->setFrameShape(QFrame::NoFrame);
    color_box->setFrameShadow(QFrame::Plain);
    color_box->setAutoFillBackground(true);

    QPalette palette = color_box->palette();
    palette.setColor(backgroundRole(), color);    
    color_box->setPalette(palette);

    horizontalLayout->addWidget(color_box);

    QLabel* item_lbl = new QLabel(frame);
    item_lbl->setText(QString::fromAscii(label.c_str()));

    horizontalLayout->addWidget(item_lbl);

    return frame;
}
