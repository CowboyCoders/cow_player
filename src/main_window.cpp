#include "main_window.h"
#include "ui_main_window.h"
#include "cow_io_device.h"

#include <boost/shared_ptr.hpp>

#include <QFile>
#include <QDir>

main_window::main_window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::main_window),
    media_object_(this),
    audio_output_(Phonon::VideoCategory, this),
    media_source_(NULL),
    piece_dialog_(this),
    select_program_dialog_(this),
    settings_dialog_(this),
    fullscreen_mode_(false)
{
    ui->setupUi(this);
    

    // Make sure the fullscreen mode menu is checked correctly
    ui->actionFullscreen->setChecked(fullscreen_mode_);

    // Bind events
    connect(ui->videoPlayer, SIGNAL(leaveFullscreen()), this, SLOT(leaveFullscreen_triggered()));
    connect(&media_object_, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(media_stateChanged()));

    this->statusBar()->showMessage(tr("Cowtastic!"));

    // Connect the media object with both VideoWidget and the AudioOutput
    Phonon::createPath(&media_object_, ui->videoPlayer);
    Phonon::createPath(&media_object_, &audio_output_);

    // Connect UI gauges
    ui->seekSlider->setMediaObject(&media_object_);
    ui->volumeSlider->setAudioOutput(&audio_output_);

#if 0
    // Filename to optional auto-loaded movie
    std::string mediaPath;

#ifdef WIN32
    QStringList args = QApplication::arguments();
    if (args.length() == 2) {
        mediaPath = args[1].toAscii().data();
    }
#else
    mediaPath = QDir::homePath() + tr("/Videos/big_buck_bunny_480p_h264.mov");
#endif

    // Create a mock IODevice
	QIODevice* iod = new mock_io_device(ctrl);

    if (iod->open(QIODevice::ReadOnly)) {
        // Create a new media source
        media_source_ = new Phonon::MediaSource(iod);
        // Auto-play
        media_object_.setCurrentSource(*media_source_);
        media_object_.play();
    }


#endif

	//START COW CLIENT HOORAY
	client_.start_logger();
    /*
	client_.set_download_directory(".");
	client_.set_bittorrent_port(12345);
	client_.get_program_table();
	client_.register_download_device_factory(
       boost::shared_ptr<libcow::download_device_factory>(
           new libcow::on_demand_server_connection_factory()), 
       "http");
   client_.register_download_device_factory(
       boost::shared_ptr<libcow::download_device_factory>(
           new libcow::multicast_server_connection_factory()),
       "multicast");
       */
   /*
   download_ctrl_ = client_.start_download(1); // FETCH BIG FUCK BUN-BUN

    // Create a mock IODevice
	cow_io_device* cow_iod = new cow_io_device(download_ctrl_);
    */

    // Create a new media source
//    media_source_ = new Phonon::MediaSource(cow_iod);

    // Auto-play
    /*
    media_object_.setCurrentSource(*media_source_);
    media_object_.play();
    */
        
    //client_.start_logger();


    client_.set_download_directory(".");
    client_.set_bittorrent_port(12345);
	//client_.get_program_table();
    client_.register_download_device_factory(
        boost::shared_ptr<libcow::download_device_factory>(
            new libcow::on_demand_server_connection_factory()), 
        "http");
    client_.register_download_device_factory(
        boost::shared_ptr<libcow::download_device_factory>(
            new libcow::multicast_server_connection_factory()),
        "multicast");

    std::cout << "starting download" << std::endl;
    client_.get_program_table();
    libcow::download_control* ctrl = client_.start_download(1);

    if(!ctrl) {
        std::cerr << "Failed to start download." << std::endl;
        QApplication::exit(-1);
    }

    ctrl->get_progress();
	
    while(true) {
		
		libcow::progress_info progress_info = ctrl->get_progress();
        std::cout << "State: " << progress_info.state_str() 
            << ", Progress: " << (progress_info.progress() * 100.0) 
            << "%" << std::endl;
    }

    client_.stop_download(1);



}

main_window::~main_window()
{
    client_.stop_download(1);
    media_object_.stop();
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
    select_program_dialog_.show();
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

void main_window::media_stateChanged(){
    if(media_object_.state()==Phonon::PlayingState){
        ui->playButton->setText("Pause");
    }else if(media_object_.state()==Phonon::ErrorState){
		this->statusBar()->showMessage(media_object_.errorString() );
    }else{
        ui->playButton->setText("Play");
    }
}

void main_window::on_playButton_clicked()
{
    if(media_object_.state()==Phonon::PlayingState){
        media_object_.pause();
    }else if(media_object_.state() == Phonon::PausedState){
        media_object_.play();
    }else if(media_object_.state() == Phonon::StoppedState){
        media_object_.play();
    }
}

void main_window::on_stopButton_clicked()
{
    media_object_.pause();
    media_object_.seek(0);
}
