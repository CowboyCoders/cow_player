#include "piece_dialog.h"
#include "ui_piece_dialog.h"

#include <QColor>
#include <QLabel>

#include <boost/bind.hpp>
#include <cow/cow.hpp>
#include <iostream>

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

    std::vector<std::string> labels;
    labels.push_back("Missing");
    labels.push_back("Test");
    set_legend(labels);

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
    std::vector<int> state;
    BOOST_LOG_TRIVIAL(info) << "piece_dialog: before set state";
    control->current_state(state);
    BOOST_LOG_TRIVIAL(info) << "piece_dialog: after set state";
    ui->pieceWidget->set_piece_states(state);
    
    ui->pieceWidget->set_device_map(control->get_device_names());
    ui->pieceWidget->set_colors(std::vector<QColor>(colors, colors + sizeof(colors)/sizeof(QColor)));

    control->set_piece_finished_callback(boost::bind(&piece_dialog::piece_downloaded_callback,this,_1,_2));
    
    std::vector<std::string> labels;
    labels.push_back("Missing");
    labels.push_back("Test");
    set_legend(labels);
}

void piece_dialog::piece_downloaded_callback(int piece_idx, int device)
{
    emit piece_downloaded(piece_idx,device);
}

void piece_dialog::set_legend(const std::vector<std::string>& labels) 
{
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
    for (; it != labels.end(); ++it) {
        QFrame* item = create_legend_item(*it, QColor());
        ui->legend->layout()->addWidget(item);
    }
}

QFrame* piece_dialog::create_legend_item(const std::string& label, const QColor& color)
{
    QFrame* frame = new QFrame;

    QHBoxLayout* horizontalLayout = new QHBoxLayout(frame);
    horizontalLayout->setContentsMargins(0, 0, 40, 0);

    QFrame* color_box = new QFrame(frame);
    color_box->setMinimumSize(QSize(10, 10));
    color_box->setMaximumSize(QSize(10, 10));
    color_box->setBaseSize(QSize(10, 10));
    color_box->setStyleSheet(QString::fromUtf8("background: #24FF14;\n" "margin-top:1px;"));
    color_box->setFrameShape(QFrame::StyledPanel);
    color_box->setFrameShadow(QFrame::Raised);

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
