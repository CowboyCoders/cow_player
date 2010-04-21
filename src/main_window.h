#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "piece_dialog.h"
#include "select_program_dialog.h"
#include "settings_dialog.h"
#include "cow/cow.hpp"

#include <QtGui/QMainWindow>

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
    void changeEvent(QEvent *e);

private:
    Ui::main_window *ui;

    piece_dialog piece_dialog_;
    select_program_dialog select_program_dialog_;
    settings_dialog settings_dialog_;

    Phonon::MediaObject media_object_;
    Phonon::AudioOutput audio_output_;
    Phonon::MediaSource* media_source_;

    bool fullscreen_mode_;

    libcow::cow_client client_;
	libcow::download_control* download_ctrl_;

    void set_fullscreen(bool fullscreen);


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
    
};

#endif // MAIN_WINDOW_H
