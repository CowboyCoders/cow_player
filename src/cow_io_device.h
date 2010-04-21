#ifndef ___cow_io_device_h___
#define ___cow_io_device_h___

#include <QIODevice>
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
        setOpenMode(QIODevice::ReadOnly);        
    }

    bool isSequential() const
    {
        return false;
    }

    virtual bool reset(){
#if  WIN32 && _DEBUG
        ::OutputDebugString("reset()\n");
#endif
        return true;
    }

    bool open(OpenMode openMode)
    {
        return true;
    }

    virtual qint64 size() const 
    {
        //return length_; 
#if WIN32 && _DEBUG
        ::OutputDebugString("size()\n");
#endif
        return 100000;
    }

    virtual bool seek(qint64 pos)
    {
#if WIN32 && _DEBUG
        static char buf[128];
        sprintf(buf, "#seek() : %d\n", pos);
        ::OutputDebugString(buf);
#endif

        return QIODevice::seek(pos);

    }

    virtual qint64 readData(char *data, qint64 maxlen)
    {
#if defined(WIN32) && defined(_DEBUG)
        static char buf[128];
        sprintf(buf, "read() : %d\n", maxlen);
        ::OutputDebugString(buf);
#endif

        libcow::utils::buffer buffer(data, maxlen);
        qint64 len = download_control_->read_data(pos(), buffer);

        return len;
    }

    virtual qint64 writeData(const char *data, qint64 len) 
    {
        return 0;
    }

private:
    qint64 length_;

    libcow::download_control* download_control_;
};

#endif // ___cow_io_device_h___
