#include <QThread>
#include <QCoreApplication>
#include "linux_io_device.h"

cow_io_device::cow_io_device(Phonon::MediaObject* media_object,
                             libcow::download_control* download_control)
    : download_control_(download_control)
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
    BOOST_LOG_TRIVIAL(debug) << "file size: " << size_;
    if(pos >= size_) {
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

bool cow_io_device::check_for_shutdown() const
{
    boost::mutex::scoped_lock lock(shutdown_mutex_);

    return shutdown_;
}

void cow_io_device::set_buffering_status(bool status)
{
    boost::mutex::scoped_lock lock(buffering_mutex_);
    buffering_ = status;
}

qint64 cow_io_device::readData(char *data, qint64 maxlen)
{
    if(QCoreApplication::instance() != 0) {
        bool is_gui_thread = QThread::currentThread()==QCoreApplication::instance()->thread();
        BOOST_LOG_TRIVIAL(debug) << "cow_io_device::readData: is_gui_thread: " << is_gui_thread;
    }
    
    if(check_for_shutdown()) {
        BOOST_LOG_TRIVIAL(debug) << "cow_io_device:readData: shutting down";
        return -1;
    }

    size_t position = pos();

    // Prioritize pieces in libcow
    download_control_->set_playback_position(position);
    
    bool has_data = download_control_->has_data(position, maxlen);
    BOOST_LOG_TRIVIAL(debug) << "cow_io_device::readData: "
                             << "pos: " << position << " maxlen: "
                             << maxlen << " has_data: " << has_data;
    if (!has_data) {
        
        set_buffering_status(true);

        int recheck_count = 1;
        int retry_delay = 10; 
        int max = 100;
        while (true) {
            if(check_for_shutdown()) {
                BOOST_LOG_TRIVIAL(debug) << "cow_io_device:readData: shutting down";
                return -1;
            }
            libcow::system::sleep(250);

            if(( download_control_->has_data(position, maxlen) )) {
                break;
            }
            BOOST_LOG_TRIVIAL(debug) << "read() MISSING DATA : pos " << position 
                                     << " : maxlen " << maxlen << " waiting ...." << recheck_count;

            if(recheck_count % retry_delay == 0) {
                // forced request for critical window
                download_control_->set_playback_position(position, true);
            }
            
            if(recheck_count >= max) {
                BOOST_LOG_TRIVIAL(warning) << "linux_io_device: couldn't get any data!";
                return -1;
            }
            ++recheck_count;
        }

        set_buffering_status(false);
    }

    libcow::utils::buffer buf(data, maxlen);
    size_t len = download_control_->read_data(position, buf);
    BOOST_LOG_TRIVIAL(debug) << "linux_io_device:read: read len: " << len << " from pos: " << position << " with maxlen: " << maxlen;

    return len;
}

qint64 cow_io_device::writeData(const char *data, qint64 len) 
{
    return 0;
}
