#ifndef ___cow_io_device_h___
#define ___cow_io_device_h___

#include <cow/cow.hpp>

#include <QIODevice>
#include <phonon>

#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>

class cow_io_device : public QIODevice, public boost::noncopyable
{
public:
    cow_io_device(Phonon::MediaObject* media_object, libcow::download_control* download_control);

   /**
    * Shuts down the IODevice. After a call to shutdown there will be no more calls to 
    * the media object or the download control.
    */
    void shutdown();

    bool is_buffering() const;

    // Implementation of random-access QIODevice
    bool isSequential() const;
    bool reset();
    bool open(OpenMode openMode);
    qint64 size() const;
    bool seek(qint64 pos);
    qint64 bytesAvailable() const;
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len) ;

private:
    Phonon::MediaObject* media_object_;
    libcow::download_control* download_control_;

    size_t size_;

    bool buffering_;
    bool shutdown_;

    mutable boost::mutex buffering_mutex_;
    mutable boost::mutex shutdown_mutex_;

};


#endif // ___cow_io_device_h___
