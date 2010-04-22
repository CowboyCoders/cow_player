#ifndef ___video_player_h___
#define ___video_player_h___

#include <phonon>


class video_player : public Phonon::VideoWidget 
{
    Q_OBJECT

public:
    video_player(QWidget *parent = 0);
    ~video_player();

signals:
    void leaveFullscreen();

protected:
    void changeEvent(QEvent *e);
    void keyPressEvent(QKeyEvent *e);
};

#endif // ___video_player_h___
