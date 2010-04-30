#include <QThread>
#include <QCoreApplication>
#include "win32_io_device.h"

cow_io_device::cow_io_device(Phonon::MediaObject* media_object,
                             libcow::download_control* download_control)
    : download_control_(download_control)
    , media_object_(media_object)
    , buffering_(false)
    , blocking_(true)
{
    open(QIODevice::ReadOnly);
    size_ = download_control_->file_size();
}

void cow_io_device::set_blocking(bool blocking)
{
    boost::mutex::scoped_lock lock(blocking_mutex_);
    blocking_ = blocking;
}

bool cow_io_device::is_buffering() const 
{
    boost::mutex::scoped_lock lock(buffering_mutex_);
    return buffering_;
}

bool cow_io_device::isSequential() const
{
    return false;
}

bool cow_io_device::reset()
{
    BOOST_LOG_TRIVIAL(debug) << "reset()";
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
    if(pos >= size_) {
        return false;
    } 
    
    return QIODevice::seek(pos);
}

qint64 cow_io_device::readData(char *data, qint64 maxlen)
{
    if (!isOpen()) {
        BOOST_LOG_TRIVIAL(debug) << "cow_io_device::readData: device is closed.";
        return -1;
    }

    // Prioritize pieces in libcow
    download_control_->set_playback_position(pos());
    
    bool has_data = download_control_->has_data(pos(), maxlen);
    BOOST_LOG_TRIVIAL(debug) << "cow_io_device::readData: "
                             << "pos: " << pos() << " maxlen: "
                             << maxlen << " has_data: " << has_data;

    if (!has_data) {

        {   // Update buffering flag
            boost::mutex::scoped_lock lock(buffering_mutex_);
            buffering_ = true;
            media_object_->pause();
        }

        int iter = 1;
        bool done = false;
        while (!done) {

            {   
                boost::mutex::scoped_lock lock(blocking_mutex_);
                if (!blocking_) {
                    boost::mutex::scoped_lock lock(buffering_mutex_);
                    buffering_ = false;
                    return 0;
                }
            }

            libcow::system::sleep(50);
            done = download_control_->has_data(pos(), maxlen);

            BOOST_LOG_TRIVIAL(debug) << "read() MISSING DATA : pos " << pos() << " : maxlen " << maxlen << " waiting ...." << iter++;
        }

        {   // Update buffering flag
            boost::mutex::scoped_lock lock(buffering_mutex_);
            buffering_ = false;
            media_object_->play();
        }

        //media_object_->play();
    }

    libcow::utils::buffer buf(data, maxlen);
    size_t len = download_control_->read_data(pos(), buf);

    BOOST_LOG_TRIVIAL(debug) << "read() : pos " << pos() << " : maxlen " << maxlen << " : len " << len;

    return len;
}

qint64 cow_io_device::writeData(const char *data, qint64 len) 
{
    return 0;
}
