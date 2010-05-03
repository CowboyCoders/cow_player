#include "piece_dialog.h"
#include "ui_piece_dialog.h"
#include <QColor>

#include <boost/bind.hpp>
#include <cow/cow.hpp>

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

void piece_dialog::set_download_control(libcow::download_control& control)
{
    std::vector<int> state(control.num_pieces());
    control.current_state(state);
    ui->pieceWidget->set_piece_states(state);
    
    ui->pieceWidget->set_device_map(control.get_device_names());
    ui->pieceWidget->set_colors(std::vector<QColor>(colors, colors + sizeof(colors)/sizeof(QColor)));

    control.set_piece_finished_callback(boost::bind(&piece_dialog::piece_downloaded_callback,this,_1,_2));
    
}

void piece_dialog::piece_downloaded_callback(int piece_idx, int device)
{
    emit piece_downloaded(piece_idx,device);
}
