#include <iostream>

#include <QDialogButtonBox>
#include <QPushButton>
#include <QString>
#include <QFileDialog>

#include "settings_dialog.h"
#include "ui_settings_dialog.h"

settings_dialog::settings_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::settings_dialog),
    conf_(0)
{
    ui->setupUi(this);
    setup_connections();
}

void settings_dialog::load_values()
{
    ui->timeout_spinbox_->setValue(conf_->get_critical_window_timeout());
    ui->bt_port_spinbox_->setValue(conf_->get_bittorrent_port());
    ui->critical_window_spinbox_->setValue(conf_->get_critical_window());
    ui->program_server_url_line_->setText(QString(conf_->get_program_table_url().c_str()));
    ui->download_dir_line_->setText(QString(conf_->get_download_dir().c_str()));
}

void settings_dialog::setup_connections()
{
    connect(ui->button_box_,SIGNAL(accepted()),this,SLOT(ok_button_clicked()));
    connect(ui->button_box_,SIGNAL(rejected()),this,SLOT(cancel_button_clicked()));
    connect(ui->download_dir_button_,SIGNAL(clicked()),this,SLOT(select_download_dir_clicked()));
}

settings_dialog::~settings_dialog()
{
    delete ui;
}

void settings_dialog::changeEvent(QEvent *e)
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

void settings_dialog::showEvent(QShowEvent *event)
{
    load_values();
}

std::string settings_dialog::qstring_to_std(QString qstr)
{
    QByteArray ba = qstr.toLatin1();
    std::string str = ba.data();

    return str;
}

void settings_dialog::cancel_button_clicked()
{
    reject();
}

void settings_dialog::ok_button_clicked()
{
    int timeout = ui->timeout_spinbox_->value();
    conf_->set_property("critical_window_timeout",timeout);
    
    int bt_port = ui->bt_port_spinbox_->value();
    conf_->set_property("bittorrent_port",bt_port);

    int critical_window = ui->critical_window_spinbox_->value();
    conf_->set_property("critical_window",critical_window);

    std::string url = qstring_to_std(ui->program_server_url_line_->text());
    conf_->set_property("program_table_url",url);
    
    std::string dir = qstring_to_std(ui->download_dir_line_->text());
    conf_->set_property("download_dir",dir);
    
    accept();
}

void settings_dialog::select_download_dir_clicked()
{
    QString qstr = QFileDialog::getExistingDirectory(this,
                                                     tr("Select download directory"),
                                                     ui->download_dir_line_->text(),
                                                     QFileDialog::ShowDirsOnly);
    ui->download_dir_line_->setText(qstr);
}
