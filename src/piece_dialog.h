#ifndef PIECE_DIALOG_HPP
#define PIECE_DIALOG_HPP

#include <QDialog>
#include <QColor>
#include <QFrame>

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
    void set_legend(const std::vector<std::string>& labels);
    QFrame* create_legend_item(const std::string& label, const QColor& color);

    void piece_downloaded_callback(int piece_idx, int device);
    
    Ui::piece_dialog *ui;

signals:
    void piece_downloaded(int piece_idx, int device);
    
};

#endif // PIECE_DIALOG_HPP
