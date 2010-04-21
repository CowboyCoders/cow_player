#include "select_program_dialog.h"
#include "ui_select_program_dialog.h"

select_program_dialog::select_program_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::select_program_dialog)
{
    ui->setupUi(this);
}

select_program_dialog::~select_program_dialog()
{
    delete ui;
}

void select_program_dialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
