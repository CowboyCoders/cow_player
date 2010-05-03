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
    piece_dialog(QWidget *parent = 0);
    ~piece_dialog();
    void set_download_control(libcow::download_control* control);

protected:
    void changeEvent(QEvent *e);

private:
    void piece_downloaded_callback(int piece_idx, int device);
    void received_state(libcow::download_control* control, std::vector<int>* state);

    libcow::download_control* download_control_; // do not delete this one
    
    Ui::piece_dialog *ui;

signals:
    void piece_downloaded(int piece_idx, int device);
    
};

#endif // PIECE_DIALOG_HPP
