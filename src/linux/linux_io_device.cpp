#include <QThread>
#include <QCoreApplication>
#include "linux_io_device.h"

cow_io_device::cow_io_device(Phonon::MediaObject* media_object,
                             libcow::download_control* download_control)
    : download_control_(download_control)
    , media_object_(media_object)
    , buffering_(false)
{
    open(QIODevice::ReadOnly);
    size_ = download_control_->file_size();
}

bool cow_io_device::is_buffering() const 
{
    return false;
}

bool cow_io_device::isSequential() const
{
    return false;
}

bool cow_io_device::reset()
{
    BOOST_LOG_TRIVIAL(debug) << "linux_io_device::reset called";
    return QIODevice::reset();
}

bool cow_io_device::open(OpenMode openMode)
{
    if (openMode != QIODevice::ReadOnly) {
        return false;
    }

    setOpenMode(QIODevice::ReadOnly);
    return true;
}

qint64 cow_io_device::size() const 
{
    return size_;
}

bool cow_io_device::seek(qint64 pos)
{
    BOOST_LOG_TRIVIAL(debug) << "seek() : offset " << pos;
    if(pos > size_) {
        return false;
    } 
    
    return QIODevice::seek(pos);
}

qint64 cow_io_device::bytesAvailable() const
{
    qint64 len = QIODevice::bytesAvailable(); 
    BOOST_LOG_TRIVIAL(debug) << "bytesAvailable() : result " << len;
    return len; 
}
qint64 cow_io_device::readData(char *data, qint64 maxlen)
{
    if(QCoreApplication::instance() != 0) {
        bool is_gui_thread = QThread::currentThread()==QCoreApplication::instance()->thread();
        BOOST_LOG_TRIVIAL(debug) << "cow_io_device::readData: is_gui_thread: " << is_gui_thread;
    }

    // Prioritize pieces in libcow
    download_control_->set_playback_position(pos());
    
    bool has_data = download_control_->has_data(pos(), maxlen);
    BOOST_LOG_TRIVIAL(debug) << "cow_io_device::readData: "
                             << "pos: " << pos() << " maxlen: "
                             << maxlen << " has_data: " << has_data;
    if (!has_data) {
        int iter = 1;
        bool done = false;
        while (!done) {
            libcow::system::sleep(250);
            done = download_control_->has_data(pos(), maxlen);
            BOOST_LOG_TRIVIAL(debug) << "read() MISSING DATA : pos " << pos() << " : maxlen " << maxlen << " waiting ...." << iter++;
        }
    }
    libcow::utils::buffer buf(data, maxlen);
    size_t len = download_control_->read_data(pos(), buf);
    BOOST_LOG_TRIVIAL(debug) << "linux_io_device:read: read len: " << len << " from pos: " << pos() << " with maxlen: " << maxlen;

    return len;
}

qint64 cow_io_device::writeData(const char *data, qint64 len) 
{
    return 0;
}
