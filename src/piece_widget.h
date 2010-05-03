#ifndef PIECE_WIDGET_HPP
#define PIECE_WIDGET_HPP

#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>
#include <QColor>

#include <vector>
#include <map>
#include <string>
namespace Ui {
    class piece_widget;
}

class piece_widget : public QAbstractScrollArea {
    Q_OBJECT
public:
    piece_widget(QWidget *parent = 0);
    ~piece_widget();

    void set_device_map(const std::map<int,std::string>& map);
    void set_colors(const std::vector<QColor>& colors);

    void request_repaint()
    {
        repaint_needed_ = true;
        if (!paint_timer_->isActive())
            paint_timer_->start();
    }

protected:
    void changeEvent(QEvent *e);

private:
    void refresh_scrollbar();
    int piece_at(const QPoint& pos) const;
    
    Ui::piece_widget *ui;

    QTimer* paint_timer_;

    bool repaint_needed_;
    
    std::vector<int> piece_states_;
    std::map<int,std::string> device_map_;
    std::vector<QColor> colors_;

protected:
    bool event(QEvent* e);

    void resizeEvent(QResizeEvent* e);
    void paintEvent(QPaintEvent* e);
    void mouseMoveEvent(QMouseEvent* e);

public slots:
    void set_piece_states(const std::vector<int>& piece_states);
    void piece_downloaded(int piece_index, int device);

private slots:
    void timed_repaint();
};

#endif // PIECE_WIDGET_HPP
