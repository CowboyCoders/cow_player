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

cow_io_device::~cow_io_device()
{
    BOOST_LOG_TRIVIAL(debug) << "destroying cow_io_device";
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
    if (!isOpen()) {
        return -1;
    }

    return size_;
}

bool cow_io_device::seek(qint64 pos)
{
    BOOST_LOG_TRIVIAL(debug) << "seek() : offset " << pos;

    if(pos >= size_) {
        return false;
    } 

    if (!isOpen()) {
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

    size_t position = pos();

    // Prioritize pieces in libcow
    download_control_->set_playback_position(position);
    
    bool has_data = download_control_->has_data(position, maxlen);
    BOOST_LOG_TRIVIAL(debug) << "cow_io_device::readData: "
                             << "pos: " << position << " maxlen: "
                             << maxlen << " has_data: " << has_data;

    if (!has_data) {

        {   // Update buffering flag
            boost::mutex::scoped_lock lock(buffering_mutex_);
            buffering_ = true;
        }

        emit buffering_state(true);

        // Delay * sleep time equals the time 
        int retry_delay = 20; 
        const int retry_delay_max = 400;

        int retry_counter = 0;

        int iter = 1;
        bool done = false;
        while (!done) {

            if (!isOpen()) {
                BOOST_LOG_TRIVIAL(debug) << "cow_io_device::readData: device is closed.";

                boost::mutex::scoped_lock lock(buffering_mutex_);
                buffering_ = false;

                return -1;
            }

            {   
                boost::mutex::scoped_lock lock(blocking_mutex_);
                if (!blocking_) {
                    boost::mutex::scoped_lock lock(buffering_mutex_);
                    buffering_ = false;
                    return 0;
                }
            }

            // Zzzz....
            libcow::system::sleep(50);

            // Check again if the data has arrived
            done = download_control_->has_data(position, maxlen);

            // If we still aren't done and haven't been so for quite some time
            // it is time to force another piece request
            if(!done && ++retry_counter >= retry_delay) {
                // forced request for critical window
                download_control_->set_playback_position(position, true);
                // Increase delay
                retry_delay = (std::min)(retry_delay+10, retry_delay_max);
                // Reset counter
                retry_counter = 0;
            }            

            BOOST_LOG_TRIVIAL(debug) << "read() MISSING DATA : pos " << position << " : maxlen " << maxlen << " waiting ...." << iter;

            // Increment iterator counter
            ++iter;
        }

        {   // Update buffering flag
            boost::mutex::scoped_lock lock(buffering_mutex_);
            buffering_ = false;
        }

        emit buffering_state(false);
    }

    libcow::utils::buffer buf(data, maxlen);
    size_t len = download_control_->read_data(position, buf);

    BOOST_LOG_TRIVIAL(debug) << "read() : pos " << position << " : maxlen " << maxlen << " : len " << len;

    return len;
}

qint64 cow_io_device::writeData(const char *data, qint64 len) 
{
    return 0;
}
