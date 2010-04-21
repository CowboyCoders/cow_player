#ifndef PIECE_WIDGET_HPP
#define PIECE_WIDGET_HPP

#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>

#include <vector>

namespace Ui {
    class piece_widget;
}

enum piece_state {
    missing = 0,
    bittorrent,
    ondemand,
    multicast
};

class piece_widget : public QAbstractScrollArea {
    Q_OBJECT
public:
    piece_widget(QWidget *parent = 0);
    ~piece_widget();

    void set_piece_states(const std::vector<piece_state>& piece_states);

    void update_piece_state(size_t piece_index, piece_state state);

    void request_repaint()
    {
        repaint_needed_ = true;
        if (!paint_timer_->isActive())
            paint_timer_->start();
    }

protected:
    void changeEvent(QEvent *e);

private:
    Ui::piece_widget *ui;

    QTimer* paint_timer_;

    bool repaint_needed_;
    
    std::vector<piece_state> piece_states_;

    void refresh_scrollbar();

    int piece_at(const QPoint& pos) const;

protected:
    bool event(QEvent* e);

    void resizeEvent(QResizeEvent* e);
    void paintEvent(QPaintEvent* e);
    void mouseMoveEvent(QMouseEvent* e);

private slots:
    void timed_repaint();
};

#endif // PIECE_WIDGET_HPP
