#include "main_window.h"
#include "ui_main_window.h"

#include <boost/shared_ptr.hpp>
#include <boost/log/trivial.hpp>

#include <QFile>
#include <QDir>
#include <QThread>
#include <QToolBar>
#include <QIcon>
#include <QMessageBox>

const std::string config_filename = "cow_player_config.xml";

std::string time2str(qint64 time)
{
    int hours = time / (1000*60*60);
    time -= hours * 1000*60*60;
    int minutes = time  / (1000*60);
    time -= minutes * 1000*60;
    int seconds = time  / 1000;

    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << hours << ":" 
       << std::setw(2) << std::setfill('0') << minutes << ":" 
       << std::setw(2) << std::setfill('0') << seconds;
    return ss.str();
}

main_window::main_window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::main_window),
    piece_dialog_(this),
    select_program_dialog_(this),
    settings_dialog_(this),
    about_dialog_(this),
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
    BOOST_LOG_TRIVIAL(debug) << "main_window: in destructor";
    reset_session();

    delete client_;
    client_ = 0;

    delete audio_output_;
    delete media_object_;
    delete ui;
}

void main_window::error_message(const QString& msg) 
{
    QMessageBox msgBox;
    msgBox.setText(msg);
    msgBox.exec();
}

void main_window::setup_actions()
{
    // Thread safe error messages
    connect(this,SIGNAL(signal_error_message(const QString&)),this,SLOT(error_message(const QString&)));

    // Playback buttons
    connect(play_action_,SIGNAL(clicked()),this,SLOT(play_action_triggered()));
    connect(stop_action_,SIGNAL(clicked()),this,SLOT(stop_action_triggered()));
    
    // Phonon events
    connect(media_object_, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(media_stateChanged()));
    connect(media_object_, SIGNAL(finished ()), this, SLOT(media_finished()));
    connect(media_object_, SIGNAL(tick(qint64)), this, SLOT(media_tick(qint64)));

    // Video player events
    connect(ui->videoPlayer, SIGNAL(leave_fullscreen()), this, SLOT(leave_fullscreen()));
    connect(ui->videoPlayer, SIGNAL(double_click()), this, SLOT(toggle_fullscreen()));

    // Signals concerning libcow
    connect(this,SIGNAL(prefetch_complete()),this,SLOT(prefetch_complete_triggered()));
    connect(this,SIGNAL(startup_complete()),this,SLOT(startup_complete_triggered()));
}

void main_window::setup_ui()
{
    ui->setupUi(this);

    // Create Phonon objects
    media_object_ = new Phonon::MediaObject(this);
    media_object_->setTickInterval(500);
    audio_output_ = new Phonon::AudioOutput(Phonon::VideoCategory, this);    
  
    // Connect phonon video output
    Phonon::createPath(media_object_, audio_output_);
    Phonon::createPath(media_object_, ui->videoPlayer);

    // Init playback buttons
    setup_playback_buttons();
    
    // Set window icon
    QIcon icon(":/player_icon.png");
    setWindowIcon(icon);
        
    // Make sure the fullscreen mode menu is checked correctly
    ui->actionFullscreen->setChecked(fullscreen_mode_);
    
    // Connect UI gauges
#ifdef WIN32
    ui->seekSlider->setTracking(true);
#else
    ui->seekSlider->setTracking(false);
#endif
    ui->seekSlider->setMediaObject(media_object_);
    ui->volumeSlider->setAudioOutput(audio_output_);
}

void main_window::setup_playback_buttons()
{   
    play_icons_ = player_button::icon_set(QIcon(tr(":/play_normal.png")),
                                          QIcon(tr(":/play_hot.png")),
                                          QIcon(tr(":/play_pressed.png")),
                                          QIcon(tr(":/play_disabled.png")));

    pause_icons_ = player_button::icon_set(QIcon(tr(":/pause_normal.png")),
                                           QIcon(tr(":/pause_hot.png")),
                                           QIcon(tr(":/pause_pressed.png")),
                                           QIcon(tr(":/pause_disabled.png")));

    stop_icons_ = player_button::icon_set(QIcon(tr(":/stop_normal.png")),
                                          QIcon(tr(":/stop_hot.png")),
                                          QIcon(tr(":/stop_pressed.png")),
                                          QIcon(tr(":/stop_disabled.png")));

    play_action_ = new player_button(play_icons_, this);
    play_action_->setIconSize(QSize(35,35));
    play_action_->setShortcut(QString("Space"));

    stop_action_ = new player_button(stop_icons_, this);
    stop_action_->setIconSize(QSize(35,35));

    ui->playbackLayout->addWidget(play_action_);
    ui->playbackLayout->addWidget(stop_action_);

    update_play_pause_button();    
    set_playback_buttons_disabled(true);
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
    client_ = new libcow::cow_client;
    client_->set_bittorrent_port(config_.get_bittorrent_port());

    client_->register_download_device_factory(
        boost::shared_ptr<libcow::download_device_factory>(
            new libcow::on_demand_server_connection_factory()), 
        "http");
    
    client_->register_download_device_factory(
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
        media_object_->pause();
        media_object_->seek(0);
        stopped_ = true;
    }
#else
    if(iodevice_ && media_source_ && media_object_) {
        media_object_->pause();
        media_object_->seek(1);
        stopped_ = true;
    }
#endif
}

void main_window::reset_session()
{

#ifdef WIN32
    stop_playback();
    media_object_->clear();
#else
    media_object_->stop();
    stopped_ = true;
#endif

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
    try {
        // Propagate config changes
        client_->set_download_directory(config_.get_download_dir()); 

        download_ctrl_ = client_->start_download(program_info, 3);
        download_ctrl_->invoke_after_init(boost::bind(&main_window::on_startup_complete_callback, this));
       
        int window = config_.get_critical_window();
        download_ctrl_->set_critical_window(window);
        
        int timeout = config_.get_critical_window_timeout();
        download_ctrl_->set_critical_window_timeout(timeout);        

        // Pre-buffer
        download_ctrl_->pre_buffer(startup_pieces(), 
            boost::bind(&main_window::on_prefetch_complete_callback,this,_1));

    } catch (libcow::exception& e) {

        //statusBar()->showMessage("Error occured when starting program");

        BOOST_LOG_TRIVIAL(error) << "cow_player: could not start program: " << e.what();

        std::stringstream ss;
        ss << "Could not start the program: " << e.what();
        emit signal_error_message(QString::fromAscii(ss.str().c_str()));

        return false;
    }

    return true;
}

std::vector<int> main_window::startup_pieces()
{
    std::vector<int> pieces;
    for(int i = 0; i < 5; ++i)
        pieces.push_back(i);
    pieces.push_back(download_ctrl_->num_pieces()-1);
    return pieces;
}

void main_window::on_startup_complete_callback()
{
    emit startup_complete(); 
}

void main_window::on_prefetch_complete_callback(std::vector<int> pieces)
{
    emit prefetch_complete(); 
}

void main_window::startup_complete_triggered()
{
    piece_dialog_.set_download_control(download_ctrl_); // set here, since by now disk pieces will be read
}

void main_window::prefetch_complete_triggered()
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
        client_->remove_download(download_ctrl_);
        download_ctrl_ = 0;
    }
}

main_window::player_state main_window::get_player_state() const
{
    bool buffering = iodevice_ && iodevice_->is_buffering();

    switch(media_object_->state()) {
    case Phonon::LoadingState: return main_window::loading;
    case Phonon::PlayingState: return main_window::playing;
    case Phonon::PausedState:
        if (stopped_) {
            return main_window::stopped;
        } else if (buffering) {
            return main_window::buffering;
        } else {
            return main_window::paused;
        }
    }

    // Default player state if not matched by the above
    return main_window::stopped;
}

void main_window::set_fullscreen(bool fullscreen)
{
    fullscreen_mode_ = fullscreen;

    setVisible(!fullscreen);
    ui->videoPlayer->setFullScreen(fullscreen);
    ui->actionFullscreen->setChecked(fullscreen);
}

void main_window::toggle_fullscreen()
{
    set_fullscreen(!is_fullscreen());
}

void main_window::leave_fullscreen()
{
    set_fullscreen(false);
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
    piece_dialog_.hide();
    settings_dialog_.hide();
    select_program_dialog_.hide();
    about_dialog_.hide();

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

            if (media_object_->hasVideo()) {
                reset_session();
            }

            set_playback_buttons_disabled(true);
            statusBar()->showMessage("Loading...");

            boost::thread(boost::bind(&main_window::start_download, this, *program));
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
    if (get_player_state() == main_window::playing) {
        play_action_->set_icon_set(pause_icons_);
    } else {
        play_action_->set_icon_set(play_icons_);
    }
}

void main_window::on_actionPieces_triggered()
{
    piece_dialog_.show();
}

void main_window::on_actionPreferences_triggered()
{
    settings_dialog_.show();
}

void main_window::media_stateChanged()
{
    bool buffering = iodevice_ && iodevice_->is_buffering();

    switch(media_object_->state()) {
        case Phonon::LoadingState:
            statusBar()->showMessage("Loading...");
            break;
        case Phonon::PlayingState:
            if (stopped_) {
                media_object_->pause();
            } else {
                statusBar()->showMessage("Playing");
                //stopped_ = false;
            }
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
            std::stringstream ss;
            ss << "Media error: " << media_object_->errorString().toAscii().data();
            statusBar()->showMessage(QString::fromAscii(ss.str().c_str()));
            BOOST_LOG_TRIVIAL(error) << "Media object error: " << media_object_->errorString().toAscii().data();
            break;
    }

    update_play_pause_button();
}

void main_window::media_finished()
{
    // Stop also causes seekbar to be positoned at the beginning
    stop_playback();
}

void main_window::play_action_triggered()
{
    switch (get_player_state()) {
    case main_window::playing:
        media_object_->pause();
        break;
    case main_window::stopped:
    case main_window::paused:
        stopped_ = false;
        media_object_->play();
        break;
    }
}

void main_window::stop_action_triggered()
{
    stop_playback();
}

void main_window::media_tick(qint64 time)
{
    if (media_object_->state() == Phonon::PlayingState) {
        std::stringstream ss;
        ss << "Time: " << time2str(time) << "/" << time2str(media_object_->totalTime());
        statusBar()->showMessage(QString(ss.str().c_str()));
    }
}
