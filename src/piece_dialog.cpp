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
    connect(this,SIGNAL(send_device_names(device_map)),this,SLOT(handle_device_names(device_map)));
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
    std::vector<int>* state = new std::vector<int>(control->num_pieces());
    control->current_state(state, boost::bind(&piece_dialog::received_state,this,control,_1));
}

void piece_dialog::received_state(libcow::download_control* control, std::vector<int>* state)
{
    ui->pieceWidget->set_piece_states(*state);
    ui->pieceWidget->set_colors(std::vector<QColor>(colors, colors + sizeof(colors)/sizeof(QColor)));
    control->set_piece_finished_callback(boost::bind(&piece_dialog::piece_downloaded_callback,this,_1,_2));
    delete state;
    control->get_device_names(boost::bind(&piece_dialog::received_devices,this,_1));
}

void piece_dialog::received_devices(std::map<int,std::string> devices)
{
    ui->pieceWidget->set_device_map(devices);
    emit send_device_names(devices); 
}
    
void piece_dialog::handle_device_names(device_map devices)
{
    std::vector<std::string> labels;
    std::map<int,std::string>::iterator it;
    for(it = devices.begin(); it != devices.end(); ++it) {
        std::pair<int,std::string> ele = *it;
        labels.push_back(ele.second);
    }
    set_legend(labels);
}

void piece_dialog::piece_downloaded_callback(int piece_idx, int device)
{
    emit piece_downloaded(piece_idx,device);
}

void piece_dialog::set_legend(const std::vector<std::string>& labels) 
{
    assert(sizeof(colors)/sizeof(QColor) > 0);

    const QObjectList& children = ui->legend->children(); //layout()->children();
    QObjectList::const_iterator wit = children.begin();
    for (; wit != children.end(); ++wit) {
        
        if ((*wit)->isWidgetType()) {
            QWidget* w = reinterpret_cast<QWidget*>(*wit);
            ui->legend->layout()->removeWidget(w);
            delete w;
        }
    }

    std::vector<std::string>::const_iterator it = labels.begin();
    for (int i = 0; it != labels.end(); ++it, ++i) {
        int ind = i % (sizeof(colors)/sizeof(QColor));
        QFrame* item = create_legend_item(*it, colors[ind]);
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
    //palette.setColor(foregroundRole(), color);
    palette.setColor(backgroundRole(), color);    
    color_box->setPalette(palette);

    horizontalLayout->addWidget(color_box);

    QLabel* item_lbl = new QLabel(frame);
    item_lbl->setText(QString::fromAscii(label.c_str()));

    horizontalLayout->addWidget(item_lbl);

    /*
    horizontalSpacer_3 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    horizontalLayout->addItem(horizontalSpacer_3);
    */

    return frame;
}
