#include "select_program_dialog.h"
#include "ui_select_program_dialog.h"
#include <iostream>

select_program_dialog::select_program_dialog(libcow::cow_client* client, QWidget *parent) :
    QDialog(parent),
    client_(client),
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

void select_program_dialog::populate_table(){
    std::list<libcow::program_info> p_info = client_->get_program_table();
    std::list<libcow::program_info>::iterator it;
    ui->program_table->setRowCount(p_info.size());
    QTableWidgetItem * id;
    QTableWidgetItem * name;
    int i = 0;
    for(it = p_info.begin(); it != p_info.end(); it++){
        QString qs(it->id);
        id = new QTableWidgetItem(qs, 0);
        QString qs2(it->name.c_str());
        name = new QTableWidgetItem(qs2, 0);
        ui->program_table->setItem(i,0,id);
        ui->program_table->setItem(i,1,name);
        i++;
    }
}
