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
        emit leave_fullscreen();
    }
}

void video_player::mouseDoubleClickEvent(QMouseEvent* e)
{
    emit double_click();
}
