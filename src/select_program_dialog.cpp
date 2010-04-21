#include <iostream>

#include "select_program_dialog.h"
#include "ui_select_program_dialog.h"
#include "list_item.h"


select_program_dialog::select_program_dialog(libcow::cow_client* client, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::select_program_dialog),
    client_(client),
    id_(-1),
    is_populated_(false)
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

void select_program_dialog::populate_list(){
    std::list<libcow::program_info> p_info = client_->get_program_table();
    std::list<libcow::program_info>::iterator it;

    list_item* name;
    int i = 0;
    for(it = p_info.begin(); it != p_info.end(); it++){
        QString movie_name(it->name.c_str());
        name = new list_item(it->id,it->name);
        name->setText(movie_name);

        ui->program_list_->insertItem(i,name);
        ++i;
    }

    is_populated_ = true;
}

void select_program_dialog::on_buttonBox_accepted()
{
    int row = ui->program_list_->currentRow();
    QListWidgetItem* item = ui->program_list_->item(row);

    if(list_item* li = dynamic_cast<list_item*>(item)) {
        id_ = li->id();
        accept();
    }
}
