#include "piece_dialog.h"
#include "ui_piece_dialog.h"
#include <cow/cow.hpp>
#include <stdio.h>

piece_dialog::piece_dialog(QWidget *parent , libcow::download_control* dctrl) :
    QDialog(parent),
    ui(new Ui::piece_dialog),
    download_ctrl_(dctrl)
{
    ui->setupUi(this);

     timer_ = new QTimer(this);
     timer_->setInterval(10);

     connect(timer_, SIGNAL(timeout()), this, SLOT(debugPieceIndicator()));
}

piece_dialog::~piece_dialog()
{
    delete ui;
    delete timer_;
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

void piece_dialog::set_download_control(libcow::download_control* dctrl){
    download_ctrl_ = dctrl;

}

void piece_dialog::showEvent(QShowEvent* e)
{

    if(download_ctrl_ != NULL ){
    libcow::progress_info progress = download_ctrl_->get_progress();
    std::vector<int> origins = progress.piece_origin();
    libtorrent::bitfield have_piece = progress.downloaded();
    std::vector<piece_state> piece_states(origins.size());
    for (size_t i = 0; i < piece_states.size(); i++) {
        if(have_piece[i] == 1) {
            piece_states[i] = static_cast<piece_state>(origins[i] + 1);
        } else {
            piece_states[i] = static_cast<piece_state>(0);
        }

    }
    ui->pieceWidget->set_piece_states(piece_states);

    timer_->start();
    }
}

void piece_dialog::debugPieceIndicator()
{    
    if(download_ctrl_ != NULL ){
        std::cout << "update progress " << download_ctrl_->piece_length() << std::endl;
        //download_ctrl_->get_progress();
        libcow::progress_info info = download_ctrl_->get_progress();
        std::cout << "progress: " << info.progress() << std::endl;
        const std::vector<int>& vec = info.piece_origin();
    }

    //ui->pieceWidget->update_piece_state(rand()%10000, static_cast<piece_state>(rand() % 3 + 1));
}
