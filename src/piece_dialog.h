#ifndef PIECE_DIALOG_HPP
#define PIECE_DIALOG_HPP

#include <QDialog>
#include <QTimer>
#include <cow/cow.hpp>

namespace Ui {
    class piece_dialog;
}

class piece_dialog : public QDialog {
    Q_OBJECT
public:
    piece_dialog(QWidget *parent = NULL, libcow::download_control* = NULL);
    ~piece_dialog();
    void set_download_control(libcow::download_control*);

protected:
    void changeEvent(QEvent *e);
    void showEvent(QShowEvent* e);
	

private:
    Ui::piece_dialog *ui;
    libcow::download_control* download_ctrl_;
    QTimer* timer_;

private slots:
    void debugPieceIndicator();

};

#endif // PIECE_DIALOG_HPP
