#include "main_window.h"
#include "ui_main_window.h"


#include <boost/shared_ptr.hpp>
#include <boost/log/trivial.hpp>

#include <QFile>
#include <QDir>
#include <qthread.h>
#include <QToolBar>
#include <QIcon>

const std::string config_filename = "cow_player_config.xml";

main_window::main_window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::main_window),
    piece_dialog_(this),
    select_program_dialog_(this),
    settings_dialog_(this),
    about_dialog_(this),
    client_(),
    download_ctrl_(0),
    iodevice_(0),
    media_object_(0),
    audio_output_(0),
    media_source_(0),
    fullscreen_mode_(false),
    stopped_(false)
{

    setup_phonon();
    setup_ui();
    setup_actions();
    load_config_file();
    init_client();
    register_download_devices();

    // It's COWTASTIC!
    statusBar()->showMessage(tr("Cowtastic!"));
}
    
void main_window::setup_phonon()
{
    media_object_ = new Phonon::MediaObject(this);
    audio_output_ = new Phonon::AudioOutput(Phonon::VideoCategory, this);
    
    Phonon::createPath(media_object_, audio_output_);
}

void main_window::setup_actions()
{
    connect(play_action_,SIGNAL(triggered()),this,SLOT(play_action_triggered()));
    connect(pause_action_,SIGNAL(triggered()),this,SLOT(pause_action_triggered()));
    connect(stop_action_,SIGNAL(triggered()),this,SLOT(stop_action_triggered()));
    
    connect(media_object_, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(media_stateChanged()));
    connect(ui->videoPlayer, SIGNAL(leaveFullscreen()), this, SLOT(leaveFullscreen_triggered()));
    connect(this,SIGNAL(startup_complete()),this,SLOT(start_io_device()));

}

void main_window::setup_ui()
{
    ui->setupUi(this);
    setup_playbackLayout();
    
    QIcon icon("player_icon.png");
    setWindowIcon(icon);
        
    // Make sure the fullscreen mode menu is checked correctly
    ui->actionFullscreen->setChecked(fullscreen_mode_);
    
    // Connect UI gauges
    ui->seekSlider->setTracking(false);
    ui->seekSlider->setMediaObject(media_object_);
    ui->volumeSlider->setAudioOutput(audio_output_);
   
    // Connect phonon video output
    Phonon::createPath(media_object_, ui->videoPlayer);
}

void main_window::setup_playbackLayout()
{
    QToolBar *bar = new QToolBar;
    
    play_action_ = new QAction(style()->standardIcon(QStyle::SP_MediaPlay), tr("Play"), this);
    pause_action_ = new QAction(style()->standardIcon(QStyle::SP_MediaPause), tr("Pause"), this);
    stop_action_ = new QAction(style()->standardIcon(QStyle::SP_MediaStop), tr("Stop"), this);
    
    set_playback_buttons_disabled(true);

    bar->addAction(play_action_);
    bar->addAction(pause_action_);
    bar->addAction(stop_action_);

    ui->playbackLayout->addWidget(bar);
}  

void main_window::load_config_file()
{
	try {
        config_.load(config_filename);
	} catch (cow_player::configuration::exceptions::load_config_error e) {
        BOOST_LOG_TRIVIAL(warning) << "cow_player: main_window: could not open config file!";
	}
        
    settings_dialog_.set_configuration(&config_);
   
    try {
        BOOST_LOG_TRIVIAL(debug) << "cow_player: main_window: constructor: config_.get_download_dir: " << config_.get_download_dir();
        BOOST_LOG_TRIVIAL(debug) << "cow_player: main_window: constructor: config_.get_bittorrent_port: " << config_.get_bittorrent_port();
        BOOST_LOG_TRIVIAL(debug) << "cow_player: main_window: constructor: config_.get_program_table_url: " << config_.get_program_table_url();
    } catch (cow_player::configuration::exceptions::conversion_error e) {
        BOOST_LOG_TRIVIAL(warning) << "cow_player: main_window: conversion error! error: " << e.what(); 
    }
}

void main_window::init_client()
{
  	client_.start_logger();
    
    set_download_dir(); 
   
    int bt_port = 55678; // a random port number as default
    try {
        bt_port = config_.get_bittorrent_port();
    } catch (cow_player::configuration::exceptions::conversion_error e) {
        BOOST_LOG_TRIVIAL(warning) << "cow_player: main_window: conversion error! error: " << e.what(); 
    }
    client_.set_bittorrent_port(bt_port);
}
    
void main_window::on_actionAbout_triggered()
{
    about_dialog_.show();
}
    
void main_window::set_download_dir()
{
    std::string download_dir = "."; // use current directory as default
    try {
        download_dir = config_.get_download_dir();
    } catch (cow_player::configuration::exceptions::conversion_error e) {
        BOOST_LOG_TRIVIAL(warning) << "cow_player: main_window: conversion error! error: " << e.what(); 
    }
    
    client_.set_download_directory(download_dir); 
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

void main_window::stop_playback()
{
    if(media_object_) {
        media_object_->pause();
        media_object_->seek(0);
        stopped_ = true;
    }
}

bool main_window::start_download(const libcow::program_info& program_info)
{
    stop_playback(); 

    download_ctrl_ = client_.start_download(program_info);
    if (!download_ctrl_) {
        BOOST_LOG_TRIVIAL(error) << "cow_player: Could not play program.";
        return false;
    }

    piece_dialog_.set_download_control(download_ctrl_);
    
    if(download_ctrl_->is_running()) {
        download_ctrl_->wait_for_pieces(startup_pieces(),boost::bind(&main_window::on_request_complete,this,_1));
    } else {
        download_ctrl_->wait_for_startup(boost::bind(&main_window::on_startup_complete,this));
    }
    
    statusBar()->showMessage("Loading...");
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
    delete audio_output_;
    delete iodevice_;
    delete media_source_;
    delete media_object_;
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
    config_.save(config_filename);
    e->accept();
}

void main_window::on_actionExit_triggered()
{
    close();
}

void main_window::on_actionFullscreen_triggered()
{
    set_fullscreen(this->ui->actionFullscreen->isChecked());
}

void main_window::on_actionShow_program_list_triggered()
{
    std::string program_table_url = "";
    
    try{
        program_table_url = config_.get_program_table_url();
    } catch (cow_player::configuration::exceptions::conversion_error e) {
        BOOST_LOG_TRIVIAL(warning) << "cow_player: main_window: conversion error! error: " << e.what(); 
    }

    select_program_dialog_.set_program_table_url(program_table_url);
    size_t timeout = 15; // in seconds
    select_program_dialog_.populate_list(timeout);

    select_program_dialog_.show();
    
    if(select_program_dialog_.exec() == QDialog::Accepted) {
        const libcow::program_info* program = select_program_dialog_.selected_program();
        if(program) {
            start_download(*program);
        } else {
            BOOST_LOG_TRIVIAL(warning) << "cow_player: main_window: got bad program_info pointer from select_program_dialog";
        }
    }
}

void main_window::set_playback_buttons_disabled(bool state)
{
    if(play_action_ && pause_action_ && stop_action_) {
        play_action_->setDisabled(state);
        pause_action_->setDisabled(state);
        stop_action_->setDisabled(state);
    }
}

void main_window::start_io_device()
{
    if(iodevice_ != 0) {
        delete iodevice_;
    }
    if(media_source_ != 0) {
        delete media_source_;
    }
    
    iodevice_ = new cow_io_device(media_object_, download_ctrl_);
    media_source_ = new Phonon::MediaSource(iodevice_);
    
    media_object_->setCurrentSource(*media_source_);
    media_object_->play();
    set_playback_buttons_disabled(false);
}

void main_window::on_request_complete(std::vector<int> pieces)
{
    emit startup_complete(); 
}

std::vector<int> main_window::startup_pieces()
{
    std::vector<int> pieces;
    pieces.push_back(0);
    pieces.push_back(1);
    pieces.push_back(2);
    pieces.push_back(3);
    pieces.push_back(4);
    pieces.push_back(download_ctrl_->num_pieces()-1);
    return pieces;
}

void main_window::on_startup_complete()
{
    download_ctrl_->wait_for_pieces(startup_pieces(),boost::bind(&main_window::on_request_complete,this,_1));
}

void main_window::on_actionPieces_triggered()
{
    piece_dialog_.show();
}

void main_window::on_actionPreferences_triggered()
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

    if (media_object_->state() == Phonon::LoadingState) {
        statusBar()->showMessage("Loading...");
    } else if (media_object_->state() == Phonon::PlayingState) {
        statusBar()->showMessage("Playing");
        stopped_ = false;
    } else if (media_object_->state() == Phonon::PausedState) {
        if(!stopped_) {
            statusBar()->showMessage("Paused");
        }
    } else if(media_object_->state()==Phonon::ErrorState) {
		this->statusBar()->showMessage(media_object_->errorString());
    }
}

void main_window::play_action_triggered()
{
    if(media_object_) {
        bool buffering = iodevice_ && iodevice_->is_buffering();
        
        if (media_object_->state() == Phonon::PausedState && !buffering) {
            media_object_->play();
        }
    }
}

void main_window::pause_action_triggered()
{
    if(media_object_) {
        if(media_object_->state() == Phonon::PlayingState) {
            media_object_->pause();
        }
    }
}

void main_window::stop_action_triggered()
{
    if(media_object_) {
        stop_playback();
        statusBar()->showMessage("Stopped");
    }
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
