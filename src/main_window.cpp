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
    load_config_file();

    setup_ui();
    setup_actions();

    init_client();

    // It's COWTASTIC!
    statusBar()->showMessage(tr("Cowtastic!"));
}

main_window::~main_window()
{
    reset_session();

    delete audio_output_;
    delete media_object_;
    delete ui;
}

void main_window::setup_actions()
{
    connect(play_action_,SIGNAL(triggered()),this,SLOT(play_action_triggered()));
    //connect(pause_action_,SIGNAL(triggered()),this,SLOT(pause_action_triggered()));
    connect(stop_action_,SIGNAL(triggered()),this,SLOT(stop_action_triggered()));
    
    connect(media_object_, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(media_stateChanged()));
    connect(ui->videoPlayer, SIGNAL(leaveFullscreen()), this, SLOT(leaveFullscreen_triggered()));
    connect(this,SIGNAL(startup_complete()),this,SLOT(start_io_device()));
}

void main_window::setup_ui()
{
    ui->setupUi(this);

    // Create Phonon objects
    media_object_ = new Phonon::MediaObject(this);
    audio_output_ = new Phonon::AudioOutput(Phonon::VideoCategory, this);    
  
    // Connect phonon video output
    Phonon::createPath(media_object_, audio_output_);
    Phonon::createPath(media_object_, ui->videoPlayer);

    // Init playback buttons
    setup_playback_buttons();
    
    // Set window icon
    QIcon icon("player_icon.png");
    setWindowIcon(icon);
        
    // Make sure the fullscreen mode menu is checked correctly
    ui->actionFullscreen->setChecked(fullscreen_mode_);
    
    // Connect UI gauges
    ui->seekSlider->setTracking(false);
    ui->seekSlider->setMediaObject(media_object_);
    ui->volumeSlider->setAudioOutput(audio_output_);
}

void main_window::setup_playback_buttons()
{    
    play_action_ = new QAction(this);
    stop_action_ = new QAction(style()->standardIcon(QStyle::SP_MediaStop), tr("Stop"), this);

    QToolBar *bar = new QToolBar;
    bar->addAction(play_action_);
    bar->addSeparator();
    bar->addAction(stop_action_);

    update_play_pause_button();    
    set_playback_buttons_disabled(true);

    ui->playbackLayout->addWidget(bar);
}  

void main_window::load_config_file()
{
	try {
        config_.load(config_filename);
	} catch (cow_player::configuration::exceptions::load_config_error e) {
        BOOST_LOG_TRIVIAL(warning) << "cow_player: main_window: could not load config file "
                                   << config_filename;
        BOOST_LOG_TRIVIAL(warning) << "cow_player: load config error was: " << e.what();
	}
        
    settings_dialog_.set_configuration(&config_);
  
    BOOST_LOG_TRIVIAL(debug) << "cow_player: configuration:";
    BOOST_LOG_TRIVIAL(debug) << "cow_player: download_dir: " << config_.get_download_dir();
    BOOST_LOG_TRIVIAL(debug) << "cow_player: bittorrent_port: " << config_.get_bittorrent_port();
    BOOST_LOG_TRIVIAL(debug) << "cow_player: program_table_url: " << config_.get_program_table_url();
}

void main_window::init_client()
{
    client_.set_download_directory(config_.get_download_dir()); 
    client_.set_bittorrent_port(config_.get_bittorrent_port());

    client_.register_download_device_factory(
        boost::shared_ptr<libcow::download_device_factory>(
            new libcow::on_demand_server_connection_factory()), 
        "http");
    
    client_.register_download_device_factory(
        boost::shared_ptr<libcow::download_device_factory>(
            new libcow::multicast_server_connection_factory()),
        "multicast");
}
    
void main_window::on_actionAbout_triggered()
{
    about_dialog_.show();
}

void main_window::stop_playback()
{
#ifdef WIN32
    if (iodevice_) {
        iodevice_->set_blocking(false);
        media_object_->stop();
        iodevice_->set_blocking(true);
        stopped_ = true;
    }
#else
    media_object_->pause();
    media_object_->seek(0);
    stopped_ = true;
#endif
}

void main_window::reset_session()
{
    stop_playback();

    if (iodevice_) {
        delete iodevice_;
        iodevice_ = 0;
    }

    if (media_source_) {
        delete media_source_;
        media_source_ = 0;
    }
}

bool main_window::start_download(const libcow::program_info& program_info)
{
    reset_session();

    download_ctrl_ = client_.start_download(program_info);
    if (!download_ctrl_) {
        BOOST_LOG_TRIVIAL(error) << "cow_player: Could not play program.";
        return false;
    }
    
    download_ctrl_->invoke_after_init(boost::bind(&main_window::on_startup_complete,this));
    
    statusBar()->showMessage("Loading...");

    return true;
}

void main_window::start_io_device()
{
    assert(!iodevice_);
    assert(!media_source_);
   
    iodevice_ = new cow_io_device(media_object_, download_ctrl_);
    media_source_ = new Phonon::MediaSource(iodevice_);
    
    media_object_->setCurrentSource(*media_source_);
    media_object_->play();

    set_playback_buttons_disabled(false);
}

void main_window::stop_download()
{
    if (download_ctrl_) {
        client_.remove_download(download_ctrl_);
        download_ctrl_ = 0;
    }
}

player_state main_window::get_player_state() const
{
    bool buffering = iodevice_ && iodevice_->is_buffering();

    switch(media_object_->state()) {
    case Phonon::LoadingState: return player_state::loading;
    case Phonon::PlayingState: return player_state::playing;
    case Phonon::PausedState:
        if (stopped_) {
            return player_state::stopped;
        } else if (buffering) {
            return player_state::buffering;
        } else {
            return player_state::paused;
        }
    }

    // Default player state if not matched by the above
    return player_state::stopped;
}

void main_window::set_fullscreen(bool fullscreen)
{
    fullscreen_mode_ = fullscreen;

    setVisible(!fullscreen);
    ui->videoPlayer->setFullScreen(fullscreen);
    ui->actionFullscreen->setChecked(fullscreen);
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
#ifdef WIN32
    if (iodevice_) {
        iodevice_->set_blocking(false);
        media_object_->stop();
        iodevice_->close();

        iodevice_ = 0;
        media_source_ = 0;
    }
#endif

    // Save configuration
	try {
        config_.save(config_filename);
    } catch (cow_player::configuration::exceptions::save_config_error e) {
        BOOST_LOG_TRIVIAL(warning) << "cow_player: could not save config file!";
	}

    e->accept();
}

void main_window::on_actionExit_triggered()
{
    close();
}

void main_window::on_actionFullscreen_triggered()
{
    set_fullscreen(ui->actionFullscreen->isChecked());
}

void main_window::on_actionShow_program_list_triggered()
{
    select_program_dialog_.set_program_table_url(config_.get_program_table_url());
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
    play_action_->setDisabled(state);
    stop_action_->setDisabled(state);
}

void main_window::update_play_pause_button()
{
    if (get_player_state() == player_state::playing) {
        play_action_->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        play_action_->setIconText(tr("Pause"));
    } else {
        play_action_->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        play_action_->setIconText(tr("Play"));
    }
}

void main_window::on_request_complete(std::vector<int> pieces)
{
    emit startup_complete(); 
}

std::vector<int> main_window::startup_pieces()
{
    std::vector<int> pieces;
    for(int i = 0; i < 5; ++i)
        pieces.push_back(i);
    pieces.push_back(download_ctrl_->num_pieces()-1);
    return pieces;
}

void main_window::on_startup_complete()
{
    std::cout << "got invok after init" << std::endl;
    //piece_dialog_.set_download_control(download_ctrl_); // set here, since by now disk pieces will be read
    download_ctrl_->pre_buffer(startup_pieces());
    download_ctrl_->invoke_when_downloaded(startup_pieces(),boost::bind(&main_window::on_request_complete,this,_1));
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

    switch(media_object_->state()) {
        case Phonon::LoadingState:
            statusBar()->showMessage("Loading...");
            break;
        case Phonon::PlayingState:
            statusBar()->showMessage("Playing");
            stopped_ = false;
            break;
        case Phonon::PausedState:
            if(!stopped_) {
                if (buffering) {
                    statusBar()->showMessage("Buffering...");
                } else {
                    statusBar()->showMessage("Paused");
                }
            } else {
                statusBar()->showMessage("Paused");
            }
            break;
        case Phonon::ErrorState:
		    statusBar()->showMessage(media_object_->errorString());
            BOOST_LOG_TRIVIAL(error) << "Media object error: " << media_object_->errorString().toAscii().data();
            break;
    }

    update_play_pause_button();
}

void main_window::play_action_triggered()
{
    switch (get_player_state()) {
    case player_state::playing:
        media_object_->pause();
        break;
    case player_state::stopped:
    case player_state::paused:
        media_object_->play();
        break;
    }
}

void main_window::stop_action_triggered()
{
    stop_playback();
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
