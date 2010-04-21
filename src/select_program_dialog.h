#ifndef SELECT_PROGRAM_DIALOG_H
#define SELECT_PROGRAM_DIALOG_H

#include <QDialog>

namespace Ui {
    class select_program_dialog;
}

class select_program_dialog : public QDialog {
    Q_OBJECT
public:
    select_program_dialog(QWidget *parent = 0);
    ~select_program_dialog();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::select_program_dialog *ui;
};

#endif // SELECT_PROGRAM_DIALOG_H
