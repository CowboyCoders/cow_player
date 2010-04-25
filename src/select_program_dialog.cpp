#include <iostream>

#include "select_program_dialog.h"
#include "ui_select_program_dialog.h"
#include "list_item.h"


select_program_dialog::select_program_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::select_program_dialog),
    selected_program_index_(-1),
    is_populated_(false),
    connected_(false)
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

void select_program_dialog::populate_list() 
{
    ui->program_list_->clear();

    if (!prog_table_.load_from_http(program_table_url_)) {
        QString error = "Could not connect to program server";
        ui->program_list_->addItem(error);
        connected_ = false;
    } else {
        libcow::program_table::iterator it = prog_table_.begin();
        for (; it != prog_table_.end(); ++it) {
            QString movie_name(it->name.c_str());
            ui->program_list_->addItem(movie_name);
        }
        connected_ = true;
    }
    
    is_populated_ = true;
}

void select_program_dialog::on_buttonBox_accepted()
{
    int row = ui->program_list_->currentRow();

    if (row > -1) {
        selected_program_index_ = row;
        BOOST_LOG_TRIVIAL(debug) << "select_program_dialog: user selected program on row: " << selected_program_index_;
        accept();
    }
}
