#ifndef ___cow_io_device_h___
#define ___cow_io_device_h___

#include <QIODevice>

#include <cow/cow.hpp>

class cow_io_device : public QIODevice
{
    Phonon::MediaObject* media_object_;

public:
    cow_io_device(Phonon::MediaObject* media_object, libcow::download_control* download_control)
        : download_control_(download_control)
        , media_object_(media_object)
    {
        open(QIODevice::ReadOnly);
        size_ = download_control_->file_size();
    }

    bool isSequential() const
    {
        return false;
    }

    virtual bool reset(){
#if _DEBUG
        BOOST_LOG_TRIVIAL(debug) << "reset()";
#endif

        return QIODevice::reset();
    }

    bool open(OpenMode openMode)
    {
        setOpenMode(openMode);
        return true;
    }

    virtual qint64 size() const 
    {
        return size_;
    }

    virtual bool seek(qint64 pos)
    {
#if _DEBUG
        BOOST_LOG_TRIVIAL(debug) << "seek() : offset " << pos;
#endif

        return QIODevice::seek(pos);
    }

    qint64 bytesAvailable() const
    {
        qint64 len = download_control_->bytes_available(pos());
            //QIODevice::bytesAvailable();
        BOOST_LOG_TRIVIAL(debug) << "bytesAvailable() : result " << len;
        return len;
    }

    bool waitForReadyRead(int msecs)
    {
        BOOST_LOG_TRIVIAL(debug) << "waitForReadyRead() : msecs " << msecs;
        return QIODevice::waitForReadyRead(msecs);
    }

    virtual qint64 readData(char *data, qint64 maxlen)
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

        // Prioritize pieces in libcow
        download_control_->set_playback_position(pos());


        if (!download_control_->has_data(pos(), maxlen)) {
            media_object_->pause();

            libcow::system::sleep(250);

            int iter = 1;
            while (!download_control_->has_data(pos(), maxlen)) {

    #if _DEBUG
                BOOST_LOG_TRIVIAL(debug) << "read() MISSING DATA : pos " << pos() << " : maxlen " << maxlen << " waiting ...." << iter++;
    #endif        
                //if (iter > 45)
    //                return 0;

                libcow::system::sleep(250);            
            }

            media_object_->play();
        }

        libcow::utils::buffer buf(data, maxlen);
        qint64 len = download_control_->read_data(pos(), buf);

#if _DEBUG
        BOOST_LOG_TRIVIAL(debug) << "read() : pos " << pos() << " : maxlen " << maxlen << " : len " << len;
#endif        

        return len;
    }

    virtual qint64 writeData(const char *data, qint64 len) 
    {
        return 0;
    }

private:
    libcow::download_control* download_control_;
    size_t size_;

};

class sequential_cow_io_device : public QIODevice
{
public:
    sequential_cow_io_device(libcow::download_control* download_control)
        : download_control_(download_control)
    {
        open(QIODevice::ReadOnly);
        size_ = download_control_->file_size();
    }

    bool isSequential() const
    {
        return true;
    }

    virtual bool reset(){
#if _DEBUG
        BOOST_LOG_TRIVIAL(debug) << "reset()";
#endif

        return QIODevice::reset();
    }

    bool open(OpenMode openMode)
    {
        setOpenMode(openMode);
        return true;
    }

    virtual qint64 size() const 
    {
        return size_;
    }

     qint64 bytesAvailable() const
     {
         return 0; //buffer.size() + QIODevice::bytesAvailable();
     }

    virtual bool seek(qint64 pos)
    {
#if _DEBUG
        BOOST_LOG_TRIVIAL(debug) << "seek() : offset " << pos;
#endif

        return QIODevice::seek(pos);
    }

    virtual qint64 readData(char *data, qint64 maxlen)
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

        // Prioritize pieces in libcow
        download_control_->set_playback_position(pos());

        int iter = 1;
        while (!download_control_->has_data(pos(), maxlen)) {
#if _DEBUG
            BOOST_LOG_TRIVIAL(debug) << "read() MISSING DATA : pos " << pos() << " : maxlen " << maxlen << " waiting ...." << iter++;
#endif        
            //if (iter > 45)
//                return 0;

            libcow::system::sleep(250);
        }

        libcow::utils::buffer buf(data, maxlen);
        qint64 len = download_control_->read_data(pos(), buf);

#if _DEBUG
        BOOST_LOG_TRIVIAL(debug) << "read() : pos " << pos() << " : maxlen " << maxlen << " : len " << len;
#endif        

        return len;
    }

    virtual qint64 writeData(const char *data, qint64 len) 
    {
        return 0;
    }

private:
    libcow::download_control* download_control_;
    size_t size_;

};


#endif // ___cow_io_device_h___
