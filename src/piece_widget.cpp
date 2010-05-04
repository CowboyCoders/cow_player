#include "piece_widget.h"
#include "ui_piece_widget.h"

#include <QPainter>
#include <QToolTip>
#include <QMouseEvent>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <iostream>

// Block size in pixels
const int block_size = 5;

// Spacing between and before blocks
const int margin = 2;

piece_widget::piece_widget(QWidget *parent) :
    QAbstractScrollArea(parent),
    ui(new Ui::piece_widget),
    repaint_needed_(false)
{
    ui->setupUi(this);

    paint_timer_ = new QTimer(this);
    paint_timer_->setInterval(250);

    connect(paint_timer_, SIGNAL(timeout()), this, SLOT(timed_repaint()));

    setMouseTracking(true); // Receive mouse move events even without buttons pressed
    setBackgroundRole(QPalette::Dark);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    refresh_scrollbar();
}

piece_widget::~piece_widget()
{
    delete ui;
}

void piece_widget::timed_repaint()
{
    repaint_needed_ = false;
    paint_timer_->stop();

    viewport()->update(); // Redraw. TODO: Dont redraw if not neccessary
}

void piece_widget::set_piece_states(const std::vector<int>& piece_states)
{
    piece_states_ = piece_states;
    refresh_scrollbar();
    request_repaint();
}

void piece_widget::set_device_map(const std::map<int,std::string>& map)
{
    device_map_ = map;
}

void piece_widget::set_colors(const std::vector<QColor>& colors)
{
    colors_ = colors;
    request_repaint();
}

void piece_widget::piece_downloaded(int piece_index, int device)
{
    piece_states_[piece_index] = device;

    if (!repaint_needed_) {

        // Check if this piece is being displayed

        const int item_size = block_size + margin;

        // Number of columns
        int ncols = (viewport()->width() - margin) / item_size;

        // Number of rows
        int nrows = viewport()->height() / item_size + 2;

        int npieces = ncols * nrows;

        // Index of the piece in the upper left corner
        size_t piece_index = ((verticalScrollBar()->value() - margin) / item_size) * ncols;

        // Check if inside visible piece range
        if (piece_index >= piece_index && piece_index <= piece_index + npieces) {
            request_repaint();
        }
    }
}

void piece_widget::refresh_scrollbar()
{
    const int item_size = block_size + margin;

    // Number of columns
    int ncols = (viewport()->width() - margin) / item_size;

    // Number of rows
    int nrows = piece_states_.size() / ncols + 1;

    // Number of rows in the windows
    int nrowsvisible = viewport()->height() / item_size;

    verticalScrollBar()->setRange(0, (nrows - nrowsvisible) * item_size + margin);
    verticalScrollBar()->setPageStep(nrowsvisible * item_size);
    verticalScrollBar()->setSingleStep(item_size);

    // Force a repaint
    viewport()->repaint();
}

int piece_widget::piece_at(const QPoint& pos) const
{
    const int item_size = block_size + margin;

    // The amout of pixels to offset the pieces with to get smooth scrolling
    int offset = ((this->verticalScrollBar()->value() - margin) % item_size + margin);

    // Number of columns
    int ncols = (this->viewport()->width() - margin) / item_size;

    // Index of the piece in the upper left corner
    size_t piece_index = ((this->verticalScrollBar()->value() - margin) / item_size) * ncols;

    // Number of rows
    int nrows = std::min<int>((piece_states_.size() - piece_index) / ncols + 2,
                               viewport()->height() / item_size + 2);

    int corrected_x = (std::max)(pos.x() - 1, 0);
    int corrected_y = (std::max)(pos.y() + offset - 1, 0);

    int piece = piece_index + (corrected_y / item_size) * ncols + corrected_x / item_size;

    return piece < piece_states_.size() ? piece : -1;
}

void piece_widget::changeEvent(QEvent *e)
{
    QAbstractScrollArea::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void piece_widget::resizeEvent(QResizeEvent* e)
{
    refresh_scrollbar();
}

void piece_widget::paintEvent(QPaintEvent* e)
{
    QAbstractScrollArea::paintEvent(e);
    
    if(colors_.empty() || piece_states_.empty()) {
        return;
    }

    // Unset repaint flags and timer
    repaint_needed_ = false;
    paint_timer_->stop();

    QPainter painter(this->viewport());
    painter.setPen(Qt::NoPen);

    const int item_size = block_size + margin;

    // The amout of pixels to offset the pieces with to get smooth scrolling
    int offset = -((this->verticalScrollBar()->value() - margin) % item_size + margin);

    // Number of columns
    int ncols = (this->viewport()->width() - margin) / item_size;

    // Index of the piece in the upper left corner
    size_t piece_index = ((this->verticalScrollBar()->value() - margin) / item_size) * ncols;

    // Number of rows
    int nrows = std::min<int>((piece_states_.size() - piece_index) / ncols + 2,
                                viewport()->height() / item_size + 2);

    for (int r = 0; r < nrows; r++) {
        for (int c = 0; c < ncols && piece_index < piece_states_.size(); c++) {
            
            painter.setBrush(colors_[piece_states_[piece_index++] % colors_.size()]);

            painter.drawRect(QRect(c*(block_size+margin) + margin,
                offset + r*(block_size+margin)+margin, block_size, block_size));
        }
    }
}

void piece_widget::mouseMoveEvent(QMouseEvent* e)
{
    int piece = piece_at(e->pos());

    if (piece != -1) {
        std::stringstream ss;
        ss << piece << " : " << device_map_[piece_states_[piece]];
        QToolTip::showText(e->globalPos(), QString(ss.str().c_str()), this);
    } else {
        QToolTip::hideText();
    }
}
