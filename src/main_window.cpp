#include "main_window.h"
#include "ui_main_window.h"
#include "cow_io_device.h"

#include <boost/shared_ptr.hpp>
#include <boost/log/trivial.hpp>

#include <QFile>
#include <QDir>

main_window::main_window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::main_window),
    client_(),
    download_ctrl_(0),
    piece_dialog_(this),
    select_program_dialog_(&client_, this),
    settings_dialog_(this),
    media_object_(this),
    audio_output_(Phonon::VideoCategory, this),
    media_source_(NULL),
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

bool main_window::start_download(std::string dir, 
                                 int bt_port, 
                                 int movie_id)
{
	client_.start_logger();
    client_.set_download_directory(dir);
    client_.set_bittorrent_port(bt_port);
    register_download_devices();
    
    libcow::download_control* ctrl = client_.start_download(movie_id);

    if(!ctrl) {
        BOOST_LOG_TRIVIAL(error) << "Failed to start download of movie id: " << movie_id;
        return false;
    } else {
        piece_dialog_.set_download_control(ctrl);
        return true;
    }

}

void main_window::stop_download(int movie_id)
{
    client_.stop_download(movie_id);
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
    if(!select_program_dialog_.is_populated())
    {
        select_program_dialog_.populate_list();
    }
    
    select_program_dialog_.show();
    
    if(select_program_dialog_.exec() == QDialog::Accepted)
    {
        int id = select_program_dialog_.selected_id();
        if(id != -1)
        {
            BOOST_LOG_TRIVIAL(debug) << "User selected movie id: " << id;
            start_download(".",12345,id);
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
