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
    select_program_dialog(libcow::cow_client*, QWidget *parent = 0);
    ~select_program_dialog();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::select_program_dialog *ui;
    std::list<libcow::program_info> p_info_;

    void populate_table();
};

#endif // SELECT_PROGRAM_DIALOG_H