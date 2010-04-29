#include "main_window.h"
#include "ui_main_window.h"

#include <boost/shared_ptr.hpp>
#include <boost/log/trivial.hpp>

#include <QFile>
#include <QDir>
#include <qthread.h>

const std::string config_filename = "cow_player_config.xml";

main_window::main_window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::main_window),
    client_(),
    download_ctrl_(0),
    piece_dialog_(this),
    select_program_dialog_(this),
    settings_dialog_(this),
    media_object_(0),
    audio_output_(0),
    media_source_(0),
    fullscreen_mode_(false),
    iodevice_(0)
{
    ui->setupUi(this);

    //buffer_indicator_ = new QSvgWidget("spinner.svg", this);
    //ui->verticalLayout_2->addWidget(buffer_indicator_);
    //ui->videoFrame->layout()->addWidget(buffer_indicator_);
    
    // Make sure the fullscreen mode menu is checked correctly
    ui->actionFullscreen->setChecked(fullscreen_mode_);

    media_object_ = new Phonon::MediaObject(this);
    audio_output_ = new Phonon::AudioOutput(Phonon::VideoCategory, this);

    // Bind events
    connect(media_object_, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(media_stateChanged()));
    //connect(media_object_, SIGNAL(bufferStatus(int)), this, SLOT(buffer_status(int)));
    //connect(media_object_, SIGNAL(totalTimeChanged (qint64)), this, SLOT(total_time_changed(qint64)));
    //connect(media_object_, SIGNAL(tick (qint64)), this, SLOT(tick(qint64)));
    connect(ui->videoPlayer, SIGNAL(leaveFullscreen()), this, SLOT(leaveFullscreen_triggered()));

    // Connect the media object with both VideoWidget and the AudioOutput
    Phonon::createPath(media_object_, ui->videoPlayer);
    Phonon::createPath(media_object_, audio_output_);

    // Connect UI gauges
    ui->seekSlider->setMediaObject(media_object_);
    ui->volumeSlider->setAudioOutput(audio_output_);


    // Load client configuration
	try {
        config_.load(config_filename);
	} catch (cowplayer::configuration::exceptions::load_config_error e) {
        BOOST_LOG_TRIVIAL(warning) << "cow_player: main_window: could not open config file!";
	}
   
    try {
        BOOST_LOG_TRIVIAL(debug) << "cow_player: main_window: constructor: config_.get_download_dir: " << config_.get_download_dir();
        BOOST_LOG_TRIVIAL(debug) << "cow_player: main_window: constructor: config_.get_bittorrent_port: " << config_.get_bittorrent_port();
        BOOST_LOG_TRIVIAL(debug) << "cow_player: main_window: constructor: config_.get_program_table_url: " << config_.get_program_table_url();
    } catch (cowplayer::configuration::exceptions::conversion_error e) {
        BOOST_LOG_TRIVIAL(warning) << "cow_player: main_window: conversion error! error: " << e.what(); 
    }


    // Init client
  	client_.start_logger();
   
    std::string download_dir = ".";
    try {
        download_dir = config_.get_download_dir();
    } catch (cowplayer::configuration::exceptions::conversion_error e) {
        BOOST_LOG_TRIVIAL(warning) << "cow_player: main_window: conversion error! error: " << e.what(); 
    }
    client_.set_download_directory(download_dir); 
    
    int bt_port = 23454; 
    try {
        bt_port = config_.get_bittorrent_port();
    } catch (cowplayer::configuration::exceptions::conversion_error e) {
        BOOST_LOG_TRIVIAL(warning) << "cow_player: main_window: conversion error! error: " << e.what(); 
    }
    client_.set_bittorrent_port(bt_port);

    register_download_devices();

    // It's COWTASTIC!
    statusBar()->showMessage(tr("Cowtastic!"));
}
   
void main_window::register_download_devices()
{
    client_.register_download_device_factory(
        boost::shared_ptr<libcow::download_device_factory>(
            new libcow::on_demand_server_connection_factory()), 
        "http");
    
    client_.register_download_device_factory(
        boost::shared_ptr<libcow::download_device_factory>(
            new libcow::multicast_server_connection_factory()),
        "multicast");
}

bool main_window::start_download(const libcow::program_info& program_info)
{
    assert(download_ctrl_ == 0);
    assert(media_source_ == 0);
   
    download_ctrl_ = client_.start_download(program_info);
    if (!download_ctrl_) {
        BOOST_LOG_TRIVIAL(error) << "cow_player: Could not play program.";
        return false;
    }

    std::vector<libcow::piece_request> reqs;
    reqs.push_back(libcow::piece_request(download_ctrl_->piece_length(), 0, 4));
    download_ctrl_->pre_buffer(reqs);

    piece_dialog_.set_download_control(download_ctrl_);
    iodevice_ = new cow_io_device(media_object_, download_ctrl_);

    media_source_ = new Phonon::MediaSource(iodevice_);
    media_object_->setCurrentSource(*media_source_);
   
    statusBar()->showMessage("Loading...");

    media_object_->play();

    return true;
}

void main_window::stop_download()
{
    if (download_ctrl_) {
        client_.remove_download(download_ctrl_);
    }
}

main_window::~main_window()
{
    delete media_source_;
    delete ui;
}

void main_window::set_fullscreen(bool fullscreen)
{
    if (fullscreen) {        
        ui->videoPlayer->setFullScreen(true);
        this->hide();
    } else {
        ui->videoPlayer->setFullScreen(false);
        this->show();
    }

    fullscreen_mode_ = fullscreen;
    this->ui->actionFullscreen->setChecked(fullscreen_mode_);
}

void main_window::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void main_window::closeEvent(QCloseEvent* e)
{
    if (iodevice_) {
        //iodevice_->shutdown();
        //media_object_->stop();
    }
    e->accept();
}

void main_window::on_actionExit_triggered()
{
    QApplication::quit();
}

void main_window::on_actionFullscreen_triggered()
{
    set_fullscreen(this->ui->actionFullscreen->isChecked());
}

void main_window::on_actionShow_program_list_triggered()
{
    if(!select_program_dialog_.is_populated()) {
        std::string program_table_url = "";
        
        try{
            program_table_url = config_.get_program_table_url();
        } catch (cowplayer::configuration::exceptions::conversion_error e) {
            BOOST_LOG_TRIVIAL(warning) << "cow_player: main_window: conversion error! error: " << e.what(); 
        }

        select_program_dialog_.set_program_table_url(program_table_url);
        select_program_dialog_.populate_list();
    }
    
    select_program_dialog_.show();
    
    if(select_program_dialog_.exec() == QDialog::Accepted) {
        const libcow::program_info* program = select_program_dialog_.selected_program();
        if(program) {
            BOOST_LOG_TRIVIAL(debug) << "cow_player: main_window: starting download of " << program->name;  
            start_download(*program);
        } else {
            BOOST_LOG_TRIVIAL(warning) << "cow_player: main_window: got bad program_info pointer from select_program_dialog";
        }
    }
}

void main_window::on_actionShow_pieces_triggered()
{
    piece_dialog_.show();
}

void main_window::on_actionSettings_triggered()
{
    settings_dialog_.show();
}

void main_window::leaveFullscreen_triggered()
{
    set_fullscreen(false);
}

void main_window::media_stateChanged()
{
    bool buffering = iodevice_ && iodevice_->is_buffering();

    if (media_object_->state()==Phonon::LoadingState) {
        statusBar()->showMessage("Loading...");
    } else if (media_object_->state()==Phonon::PlayingState) {
        ui->playButton->setText("Pause");
    } else if(media_object_->state() == Phonon::StoppedState) {
        ui->playButton->setText("Play");
    } else if(media_object_->state()==Phonon::ErrorState) {
		this->statusBar()->showMessage(media_object_->errorString());
    } else if (media_object_->state()==Phonon::PausedState) {
        if (buffering) {
            ui->playButton->setText("Pause");
            statusBar()->showMessage("Buffering...");
        } else {
            ui->playButton->setText("Play");
        }
    }
}

void main_window::on_playButton_clicked()
{
    bool buffering = iodevice_ && iodevice_->is_buffering();

    Phonon::State s = media_object_->state();
    if (media_object_->state() == Phonon::PlayingState){
        media_object_->pause();
    } else if (media_object_->state() == Phonon::StoppedState) {
        media_object_->play();
    } else if (media_object_->state() == Phonon::PausedState && !buffering) {
        media_object_->play();
    } else if (media_object_->state()==Phonon::ErrorState) {
        statusBar()->showMessage(media_object_->errorString());
    }
}

void main_window::on_stopButton_clicked()
{
    media_object_->stop();
}

void main_window::buffer_status(int percent_filled)
{
#ifdef WIN32
    static char buf[256];
    sprintf(buf, "Buffering: %d", percent_filled);
    ::OutputDebugString(buf);
#endif
}

void main_window::tick(qint64 time)
{
    std::stringstream ss;
    ss << "Time: " << (time/1000.0) << "/" << (media_object_->totalTime()/1000.0);
    statusBar()->showMessage(QString(ss.str().c_str()));
}

void main_window::total_time_changed(qint64 total_time)
{
    std::stringstream ss;
    ss << "Total time: " << (total_time/1000.0);
    statusBar()->showMessage(QString(ss.str().c_str()));
}
