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


    
    // Make sure the fullscreen mode menu is checked correctly
    ui->actionFullscreen->setChecked(fullscreen_mode_);

    media_object_ = new Phonon::MediaObject(this);
    audio_output_ = new Phonon::AudioOutput(Phonon::VideoCategory, this);

    // Bind events
    connect(media_object_, SIGNAL(stateChanged(Phonon::State, Phonon::State)), 
        this, SLOT(media_stateChanged(Phonon::State, Phonon::State)));
    connect(ui->videoPlayer, SIGNAL(leaveFullscreen()), this, SLOT(leaveFullscreen_triggered()));
    connect(this,SIGNAL(startup_complete()),this,SLOT(start_io_device()));

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
        BOOST_LOG_TRIVIAL(warning) << "cow_player: could not load config file \"" << config_filename << "\": " << e.what();
	}
   
    BOOST_LOG_TRIVIAL(debug) << "cow_player: configuration:";
    BOOST_LOG_TRIVIAL(debug) << "cow_player:   download_dir:           " << config_.get_download_dir();
    BOOST_LOG_TRIVIAL(debug) << "cow_player:   bittorrent_port:        " << config_.get_bittorrent_port();
    BOOST_LOG_TRIVIAL(debug) << "cow_player:   program_table_url:      " << config_.get_program_table_url();

    // Init client
  	client_.start_logger();
   
    client_.set_download_directory(config_.get_download_dir()); 
    client_.set_bittorrent_port(config_.get_bittorrent_port());

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

void main_window::stop_playback()
{
    BOOST_LOG_TRIVIAL(debug) << "cow_player: pre stop";  

    if(media_object_ && iodevice_) {
#ifdef WIN32
        iodevice_->set_blocking(false);
        media_object_->stop();
        iodevice_->set_blocking(true);
#else
        media_object_->pause();
        media_object_->seek(0);
#endif        
    }

    BOOST_LOG_TRIVIAL(debug) << "cow_player: post stop";  
}

bool main_window::start_download(const libcow::program_info& program_info)
{
    stop_playback(); 

    statusBar()->showMessage("Starting program...");

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
    if (iodevice_) {
        //iodevice_->shutdown();
        //media_object_->stop();
    }
    if (iodevice_) {
#ifdef WIN32
        iodevice_->set_blocking(false);
        media_object_->stop();
        iodevice_->close();
#endif
    }

    // Save configuration
	try {
        config_.save(config_filename);
    } catch (cowplayer::configuration::exceptions::save_config_error e) {
        BOOST_LOG_TRIVIAL(warning) << "cow_player: main_window: could not save config file!";
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
        std::string program_table_url;
        
        try{
            program_table_url = config_.get_program_table_url();
        } catch (cowplayer::configuration::exceptions::conversion_error e) {
            BOOST_LOG_TRIVIAL(warning) << "cow_player: main_window: conversion error! error: " << e.what(); 

            //TODO: Add error message dialog

            return;
        }

        select_program_dialog_.set_program_table_url(program_table_url);
        size_t timeout = 15; // in seconds
        select_program_dialog_.populate_list(timeout);
    }
    
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

void main_window::start_io_device()
{
    iodevice_ = new cow_io_device(media_object_, download_ctrl_); // leaking memory right here
    if(media_source_ != 0) {
        delete media_source_;
        media_source_ = 0;
    }

    if(iodevice_ != 0) {
        delete iodevice_;
        iodevice_ = 0;
    }

    
    iodevice_ = new cow_io_device(media_object_, download_ctrl_);
    media_source_ = new Phonon::MediaSource(iodevice_);
    
    media_object_->setCurrentSource(*media_source_);
    media_object_->play();
}

void main_window::on_request_complete(std::vector<int> pieces)
{
    emit startup_complete(); 
}

std::vector<int> main_window::startup_pieces()
{
    std::vector<int> pieces;
    for (int i = 0; i < 5; ++i)
        pieces.push_back(i);

    pieces.push_back(download_ctrl_->num_pieces()-1);

    return pieces;
}

void main_window::on_startup_complete()
{
    download_ctrl_->wait_for_pieces(startup_pieces(),boost::bind(&main_window::on_request_complete,this,_1));
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

void main_window::media_stateChanged(Phonon::State oldState, Phonon::State newState)
{
    bool buffering = iodevice_ && iodevice_->is_buffering();

    switch (media_object_->state()) {
    case Phonon::LoadingState:
        statusBar()->showMessage("Loading...");
        break;
    case Phonon::PlayingState:
        statusBar()->showMessage("Playing");
        ui->playButton->setText("Pause");
        break;
    case Phonon::StoppedState:
        ui->playButton->setText("Play");
        statusBar()->showMessage("Stopped");
        break;
    case Phonon::PausedState:
        if (buffering) {
            ui->playButton->setText("Pause");
            statusBar()->showMessage("Buffering...");
        } else {
            ui->playButton->setText("Play");
            statusBar()->showMessage("Paused");
        }
        break;
    case Phonon::ErrorState:
        ui->playButton->setText("Play");
		statusBar()->showMessage(media_object_->errorString());
        BOOST_LOG_TRIVIAL(error) << "Media object error: " << media_object_->errorString().toUtf8().data();
        break;
    }
}

void main_window::on_playButton_clicked()
{
    bool buffering = iodevice_ && iodevice_->is_buffering();

    switch (media_object_->state()) {
    case Phonon::PlayingState:
        iodevice_->set_blocking(false);
        media_object_->pause();
        iodevice_->set_blocking(true);
        break;
    case Phonon::StoppedState:
        media_object_->play();
        break;
    case Phonon::PausedState:
        if (!buffering) {
            media_object_->play();
        }
        break;
    case Phonon::ErrorState:
        // Try to play anyway...!
        media_object_->play();
        break;
    }
}

void main_window::on_stopButton_clicked()
{
    stop_playback();
}

void main_window::buffer_status(int percent_filled)
{
#ifdef WIN32
    static char buf[256];
    sprintf(buf, "Buffering: %d", percent_filled);
    ::OutputDebugString(buf);
#endif
}

std::string time2str(qint64 time)
{
    int hours = time / (1000*60*60);
    time -= hours * 1000*60*60;
    int minutes = time  / (1000*60);
    time -= minutes * 1000*60;
    int seconds = time  / 1000;

    std::stringstream ss;
    ss << std::setw(2) << hours << ":" << minutes << ":" << seconds;
    return ss.str();
}

void main_window::tick(qint64 time)
{
    if (media_object_->state() == Phonon::PlayingState) {
        std::stringstream ss;
        ss << "Time: " << time2str(time) << "/" << time2str(media_object_->totalTime());
        statusBar()->showMessage(QString(ss.str().c_str()));
    }
}

void main_window::total_time_changed(qint64 total_time)
{
    std::stringstream ss;
    ss << "Total time: " << (total_time/1000.0);
    statusBar()->showMessage(QString(ss.str().c_str()));
}
