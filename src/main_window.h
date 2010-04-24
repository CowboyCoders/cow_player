#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "piece_dialog.h"
#include "select_program_dialog.h"
#include "settings_dialog.h"
#include "client_configuration.h"
#include "cow_io_device.h"

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
    void set_fullscreen(bool fullscreen);
    bool start_download(const libcow::program_info& program_info);
    void stop_download();
    void register_download_devices();
    
    Ui::main_window *ui;

    QSvgWidget* buffer_indicator_;

    cowplayer::client_configuration config_;

    libcow::cow_client client_;
	libcow::download_control* download_ctrl_;

    cow_io_device* iodevice_;
	
    piece_dialog piece_dialog_;
    select_program_dialog select_program_dialog_;
    settings_dialog settings_dialog_;

    Phonon::MediaObject* media_object_;
    Phonon::AudioOutput* audio_output_;
    Phonon::MediaSource* media_source_;

    bool fullscreen_mode_;
    
private slots:
    void media_stateChanged();
    void on_stopButton_clicked();
    void on_playButton_clicked();
    void on_actionShow_program_list_triggered();
    void on_actionShow_pieces_triggered();
    void on_actionFullscreen_triggered();
    void on_actionSettings_triggered();
    void on_actionExit_triggered();
    void leaveFullscreen_triggered();

    void tick(qint64 time);
    void buffer_status(int percent_filled);
    void total_time_changed(qint64 total_time);
    
};

#endif // MAIN_WINDOW_H
