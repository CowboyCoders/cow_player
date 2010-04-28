#include <QThread>
#include <QCoreApplication>
#include "win32_io_device.h"

cow_io_device::cow_io_device(Phonon::MediaObject* media_object,
                             libcow::download_control* download_control)
    : download_control_(download_control)
    , media_object_(media_object)
    , buffering_(false)
    , shutdown_(false)
{
    open(QIODevice::ReadOnly);
    size_ = download_control_->file_size();
}

void cow_io_device::shutdown()
{
    boost::mutex::scoped_lock lock(shutdown_mutex_);
    shutdown_ = true;
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
    if(pos > size_) {
        return false;
    } 
    
    return QIODevice::seek(pos);
}

/*
qint64 cow_io_device::bytesAvailable() const
{
    boost::mutex::scoped_lock lock(shutdown_mutex_);

    if (shutdown_) {
        return 0;
    }

    qint64 len = download_control_->bytes_available(pos());
    BOOST_LOG_TRIVIAL(debug) << "bytesAvailable() : result " << len;

    return len;
}
*/

qint64 cow_io_device::bytesAvailable() const
{
    qint64 len = QIODevice::bytesAvailable(); 
    BOOST_LOG_TRIVIAL(debug) << "bytesAvailable() : result " << len;
    return len; 
}
qint64 cow_io_device::readData(char *data, qint64 maxlen)
{
    /*
    // DirextShow9 backend do seek to size-1024^2 when it opens up a video.
    // If this method returns 0, the backend can not determine the length of
    // the movie, but the playback will still work properly. The seekbar
    // will however not function.
    if (pos() == size() - 1024*1024) {
#if _DEBUG
    BOOST_LOG_TRIVIAL(debug) << "read() ABORT BECAUSE READING LAST PIECE";
#endif        
        return 0;
    }
    */
    
    if(QCoreApplication::instance() != 0) {
        bool is_gui_thread = QThread::currentThread()==QCoreApplication::instance()->thread();
        BOOST_LOG_TRIVIAL(debug) << "cow_io_device::readData: is_gui_thread: " << is_gui_thread;
    }

    shutdown_mutex_.lock();

    if (shutdown_) {
        shutdown_mutex_.unlock();
        return 0;
    }
	shutdown_mutex_.unlock();

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
        }

        media_object_->pause();

        int iter = 1;
        bool done = false;
        while (!done) {

            libcow::system::sleep(25);

            shutdown_mutex_.lock();

            if (shutdown_) {
                shutdown_mutex_.unlock();
                return 0;
            }
			shutdown_mutex_.unlock();
            
			done = download_control_->has_data(pos(), maxlen);

            BOOST_LOG_TRIVIAL(debug) << "read() MISSING DATA : pos " << pos() << " : maxlen " << maxlen << " waiting ...." << iter++;
        }

        {   // Update buffering flag
            boost::mutex::scoped_lock lock(buffering_mutex_);
            buffering_ = false;
        }

        media_object_->play();
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
