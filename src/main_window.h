#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "piece_dialog.h"
#include "select_program_dialog.h"
#include "settings_dialog.h"
#include "client_configuration.h"
#include "cow_io_device.h"
#include "about_dialog.h"

#include <QAction>
#include <QtGui/QMainWindow>
#include <QCloseEvent>
#include <QSvgWidget>

/* phonon with a capital "P" breaks ubuntu make */
#include <phonon>

#include <cow/cow.hpp>

namespace Ui {
    class main_window;
}

class main_window : public QMainWindow {
    Q_OBJECT
public:
    main_window(QWidget *parent = 0);
    ~main_window();

protected:
    void changeEvent(QEvent* e);
    void closeEvent(QCloseEvent* e);

private:
    void setup_actions();
    void setup_ui();
    void setup_playback_buttons();
    
    void load_config_file();
    void init_client();
    
    void stop_playback();

    void reset_session();
    
    void on_startup_complete_callback();
    void on_prefetch_complete_callback(std::vector<int> pieces);
    
    void set_fullscreen(bool fullscreen);
    void set_playback_buttons_disabled(bool state);

    void update_play_pause_button();
    
    bool start_download(const libcow::program_info& program_info);
    void stop_download();
    
    enum player_state {
        loading,
        buffering,
        playing,
        paused,
        stopped
    };

    player_state get_player_state() const;
    

    
    std::vector<int> startup_pieces();

    
    Ui::main_window *ui;

    QSvgWidget* buffer_indicator_;

    cow_player::client_configuration config_;

	
    piece_dialog piece_dialog_;
    select_program_dialog select_program_dialog_;
    settings_dialog settings_dialog_;
    about_dialog about_dialog_;
    
    libcow::cow_client* client_;
    libcow::download_control* download_ctrl_;
    cow_io_device* iodevice_;

    Phonon::MediaObject* media_object_;
    Phonon::AudioOutput* audio_output_;
    Phonon::MediaSource* media_source_;
    QAction *play_action_;
    QAction *pause_action_;
    QAction *stop_action_;

    bool fullscreen_mode_;
    bool stopped_;
    
private slots:
    void prefetch_complete_triggered();
    void startup_complete_tiggered();

    void media_stateChanged();
    void on_actionShow_program_list_triggered();
    void on_actionPieces_triggered();
    void on_actionFullscreen_triggered();
    void on_actionPreferences_triggered();
    void on_actionExit_triggered();
    void on_actionAbout_triggered();
    void leaveFullscreen_triggered();
    void play_action_triggered();
    void stop_action_triggered();

    void tick(qint64 time);

signals:
    void startup_complete();
    void prefetch_complete();
    
};

#endif // MAIN_WINDOW_H
