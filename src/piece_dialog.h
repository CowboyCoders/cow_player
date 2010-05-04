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
    typedef std::map<int,std::string> device_map; 
    piece_dialog(QWidget *parent = 0);
    ~piece_dialog();
    void set_download_control(libcow::download_control* control);

protected:
    void changeEvent(QEvent *e);

private:
    void set_legend(const std::map<int, std::string>& devices);
    QFrame* create_legend_item(const std::string& label, const QColor& color);

    void piece_downloaded_callback(int piece_idx, int device);
    void received_state(libcow::download_control* control, std::vector<int>* state);
    void received_devices(std::map<int,std::string> devices);

    libcow::download_control* download_control_; // do not delete this one
    
    Ui::piece_dialog *ui;

private slots:
    void handle_device_names(device_map devices);
    void startup_complete();

signals:
    void piece_downloaded(int piece_idx, int device);
    
};

#endif // PIECE_DIALOG_HPP
