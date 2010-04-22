#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <QDialog>
#include <cow/cow.hpp>

namespace Ui {
    class settings_dialog;
}

class settings_dialog : public QDialog {
    Q_OBJECT
public:
    settings_dialog(QWidget *parent = 0);
    ~settings_dialog();

protected:
    void changeEvent(QEvent *e);

private:
    libcow::cow_client* client_;
    Ui::settings_dialog *ui;
};

#endif // SETTINGS_DIALOG_H
