#ifndef SELECT_PROGRAM_DIALOG_H
#define SELECT_PROGRAM_DIALOG_H

#include <QDialog>

#include <cow/cow_client.hpp>
#include <cow/program_info.hpp>


namespace Ui {
    class select_program_dialog;
}

class select_program_dialog : public QDialog {
    Q_OBJECT
public:
    select_program_dialog(libcow::cow_client* client, QWidget *parent = 0);
    ~select_program_dialog();

    void populate_list();
    int selected_id()
    {
        return id_;
    }

protected:
    void changeEvent(QEvent *e);

private:
    Ui::select_program_dialog *ui;
    libcow::cow_client* client_;
    int id_;

private slots:
    void on_buttonBox_accepted();
};

#endif // SELECT_PROGRAM_DIALOG_H
