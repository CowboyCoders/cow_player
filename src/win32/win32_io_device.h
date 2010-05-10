#ifndef ___win32_io_device_h___
#define ___win32_io_device_h___

#include <cow/cow.hpp>

#include <QIODevice>
#include <phonon>

#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>

class cow_io_device : public QIODevice, public boost::noncopyable
{
    Q_OBJECT
public:
    cow_io_device(Phonon::MediaObject* media_object, libcow::download_control* download_control);
    ~cow_io_device();

    void set_blocking(bool blocking);

    bool is_buffering() const;

    // Implementation of random-access QIODevice
    bool isSequential() const;
    bool reset();
    bool open(OpenMode openMode);
    qint64 size() const;
    bool seek(qint64 pos);
    //qint64 bytesAvailable() const;
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len) ;

private:
    Phonon::MediaObject* media_object_;
    libcow::download_control* download_control_;

    size_t size_;

    bool buffering_;
    bool blocking_;

    mutable boost::mutex buffering_mutex_;
    mutable boost::mutex blocking_mutex_;

signals:
    void buffering_state(bool buffering);

};


#endif // ___win32_io_device_h___
