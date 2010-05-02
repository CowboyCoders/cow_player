#include <iostream>
#include <QDialogButtonBox>
#include <QPushButton>


#include "select_program_dialog.h"
#include "ui_select_program_dialog.h"
#include "list_item.h"


select_program_dialog::select_program_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::select_program_dialog),
    selected_program_index_(-1),
    is_populated_(false),
    connected_(false),
    disp_(0)
{
    ui->setupUi(this);
    QPushButton* ok_button = ui->buttonBox->button(QDialogButtonBox::Ok);
    ok_button->setDisabled(true);
    connect(this,SIGNAL(download_completed(bool)),this,SLOT(handle_download_completed(bool)));
    connect(ui->program_list_,SIGNAL(itemSelectionChanged()),this,SLOT(current_item_changed()));
}

select_program_dialog::~select_program_dialog()
{
    delete ui;
}
    
void select_program_dialog::current_item_changed()
{
    if(connected_) {
        QPushButton* ok_button = ui->buttonBox->button(QDialogButtonBox::Ok);
        ok_button->setDisabled(false);
    }
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

void select_program_dialog::show_msg(QString msg)
{
    ui->program_list_->clear();
    ui->program_list_->addItem(msg);
}

void select_program_dialog::show_list()
{
    ui->program_list_->clear();
    libcow::program_table::iterator it = prog_table_.begin();
    for (; it != prog_table_.end(); ++it) {
        QString movie_name(it->name.c_str());
        ui->program_list_->addItem(movie_name);
    }
    is_populated_ = true;
}

void select_program_dialog::handle_download_completed(bool res)
{
    if(res) {
        connected_ = true;
        show_list();
    } else {
        connected_ = false;
        show_msg("Could not connect to server");
    }
}

void select_program_dialog::download_list(std::string url, size_t timeout)
{
    bool res = prog_table_.load_from_http(url,timeout);
    emit download_completed(res);
}

void select_program_dialog::populate_list(size_t timeout) 
{
    show_msg("Connecting to server");
    disp_.post<boost::function<void()> >(boost::bind(&select_program_dialog::download_list,this,program_table_url_,timeout));
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
