#include "video_player.h"
#include <QtGui/QApplication>
#include <QWidget>
#include <QEvent>
#include <QKeyEvent>
#include <QDesktopWidget>
#include <iostream>

video_player::video_player(QWidget *parent) :
    Phonon::VideoWidget(parent)
{
    QDesktopWidget * d = QApplication::desktop();
    int w = d->width();
    int h = d->height();
    setMaximumWidth(w-30); // values optimized for ubuntu 9.10
    setMaximumHeight(h-204); // values optimized for ubuntu 9.10
}

video_player::~video_player()
{
}

void video_player::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    default:
        break;
    }
}

void video_player::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        emit leaveFullscreen();
    }
}
