#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <string>
#include <QDialog>
#include <cow/cow.hpp>
#include "client_configuration.h"

namespace Ui {
    class settings_dialog;
}

class settings_dialog : public QDialog {
    Q_OBJECT
public:
    settings_dialog(QWidget *parent = 0);
    ~settings_dialog();
    
    void set_configuration(cow_player::client_configuration *conf)
    {
        conf_ = conf;
        set_init_values();
    }


protected:
    void changeEvent(QEvent *e);

private:
    void setup_connections();
    void set_init_values();
    std::string qstring_to_std(QString qstr);
    Ui::settings_dialog *ui;

    cow_player::client_configuration* conf_;

private slots:
    void ok_button_clicked();
    void select_download_dir_clicked();
};

#endif // SETTINGS_DIALOG_H
