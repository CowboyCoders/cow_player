#ifndef ___cow_io_device_h___
#define ___cow_io_device_h___

#include <QIODevice>
#include <QFile>
#include <cow/cow.hpp>

#if defined(WIN32) && defined(_DEBUG)
    #include <Windows.h>
#endif

class cow_io_device : public QIODevice
{
public:
    cow_io_device(libcow::download_control* download_control)
        : download_control_(download_control)
        , length_(0)
    {
        open(QFile::ReadOnly);
        //setOpenMode(QIODevice::ReadOnly);        
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
        //return original_.reset();
        
        return true;
    }

    bool open(OpenMode openMode)
    {
        //QIODevice::open(openMode);
        setOpenMode(openMode);  

        //original_.setFileName("original.mpg");
        //return original_.open(openMode);

        return true;
    }

    virtual qint64 size() const 
    {
#if _DEBUG
        BOOST_LOG_TRIVIAL(debug) << "size()";
#endif

        return 150118400;
        return original_.size();
    }

    virtual bool seek(qint64 pos)
    {
#if _DEBUG
        BOOST_LOG_TRIVIAL(debug) << "seek() : offset " << pos;
#endif

        //QIODevice::seek(pos);
        //return original_.seek(pos);

        return QIODevice::seek(pos);
    }

    virtual qint64 readData(char *data, qint64 maxlen)
    {

        //qint64 len = download_control_->read_data(pos(), buffer);

        int iter = 1;
        if (pos() == size() - 1024*1024) {
#if _DEBUG
        BOOST_LOG_TRIVIAL(debug) << "read() ABORT BECAUSE READING LAST PIECE";
#endif        
            return 0;
        }

        while (!download_control_->has_data(pos(), maxlen) /*&& pos() < 150102016*/) {
#if _DEBUG
        BOOST_LOG_TRIVIAL(debug) << "read() MISSING DATA :pos " << pos() << " : maxlen " << maxlen << " waiting ...." << iter++;
#endif        
            if (iter > 50)
                return 0;

            libcow::system::sleep(250);

           // return 0;
        }

        libcow::utils::buffer sbuffer(data /*new char[32000]*/, maxlen);
        qint64 len1 = download_control_->read_data(pos(), sbuffer);

        //original_.seek(pos());
        //qint64 len1 = original_.read(data, maxlen);

        //int cr = memcmp(data, sbuffer.data(), len1);

#if _DEBUG
        BOOST_LOG_TRIVIAL(debug) << "read() : pos " << pos() << " : maxlen " << maxlen << " : len " << len1; // << " : len 2 : " << " : cmp " << cr;
#endif        

        return len1;
    }

    virtual qint64 writeData(const char *data, qint64 len) 
    {
        return 0;
    }

private:
    qint64 length_;

    libcow::download_control* download_control_;

    QFile original_;
};

#endif // ___cow_io_device_h___
