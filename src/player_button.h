#ifndef ___player_button_h___
#define ___player_button_h___

#include <QToolButton>
#include <QResizeEvent>
#include <QCursor>

class player_button : public QToolButton
{
    Q_OBJECT
public:

    struct icon_set {
        icon_set() {}
        icon_set(const QIcon& normalIcon, const QIcon& hotIcon, const QIcon& pressedIcon, const QIcon& disabledIcon)
            : normalIcon(normalIcon)
            , hotIcon(hotIcon)
            , pressedIcon(pressedIcon)
            , disabledIcon(disabledIcon)
        {
        }

        QIcon normalIcon;
        QIcon hotIcon;
        QIcon pressedIcon;
        QIcon disabledIcon;
    };

    player_button(const icon_set& iconSet, QWidget* parent = 0)
        : QToolButton(parent)
        , iconSet_(iconSet)
        , hover_(false)
    {
        setAutoRaise(false);
        setStyleSheet(QString::fromUtf8("border:0;"));
    }
    virtual ~player_button() {}

    void set_icon_set(const icon_set& iconSet) 
    {
        iconSet_ = iconSet;
        update_icon();
    }

protected:
    virtual void enterEvent(QEvent* e)
    {
        QToolButton::enterEvent(e);
        hover_ = true;
        update_icon();
    }

    virtual void leaveEvent(QEvent* e)
    {
        QToolButton::leaveEvent(e);
        hover_ = false;
        update_icon();
    }

    virtual void mousePressEvent(QMouseEvent* e)
    {
        QToolButton::mousePressEvent(e);
        update_icon();
    }

    virtual void mouseReleaseEvent(QMouseEvent* e)
    {
        QToolButton::mouseReleaseEvent(e);
        update_icon();
    }

    virtual void changeEvent(QEvent* e)
    {
        QToolButton::changeEvent(e);
        if (e->type() == QEvent::EnabledChange) {
            update_icon();
        }
    }

private:

    void update_icon() 
    {
        if (!isEnabled()) {
            setIcon(iconSet_.disabledIcon);
        } else {
            if (isDown()) {
                setIcon(iconSet_.pressedIcon);
            } else if (hover_) {
                setIcon(iconSet_.hotIcon);
            } else {
                setIcon(iconSet_.normalIcon);
            }
        }
    }

    icon_set iconSet_;

    bool hover_;

};

#endif // ___player_button_h___