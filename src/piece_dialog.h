#ifndef PIECE_DIALOG_HPP
#define PIECE_DIALOG_HPP

#include <QDialog>
#include <QTimer>

namespace Ui {
    class piece_dialog;
}

class piece_dialog : public QDialog {
    Q_OBJECT
public:
    piece_dialog(QWidget *parent = 0);
    ~piece_dialog();

protected:
    void changeEvent(QEvent *e);
    void showEvent(QShowEvent* e);

private:
    Ui::piece_dialog *ui;

    QTimer* timer_;

private slots:
    void debugPieceIndicator();

};

#endif // PIECE_DIALOG_HPP
