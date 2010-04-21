#include "piece_dialog.h"
#include "ui_piece_dialog.h"

piece_dialog::piece_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::piece_dialog)
{
    ui->setupUi(this);

     timer_ = new QTimer(this);
     timer_->setInterval(10);

     connect(timer_, SIGNAL(timeout()), this, SLOT(debugPieceIndicator()));
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

void piece_dialog::showEvent(QShowEvent* e)
{
    std::vector<piece_state> piece_states(10007);
    for (size_t i = 0; i < piece_states.size(); i++) {
        piece_states[i] = static_cast<piece_state>(0 /*rand() % 4*/);
    }
    ui->pieceWidget->set_piece_states(piece_states);
    ui->pieceWidget->update_piece_state(200, bittorrent);

    timer_->start();
}

void piece_dialog::debugPieceIndicator()
{    
    ui->pieceWidget->update_piece_state(rand()%10000, static_cast<piece_state>(rand() % 3 + 1));
}