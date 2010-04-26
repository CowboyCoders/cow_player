#ifndef ___cow_io_device_h___
#define ___cow_io_device_h___

// include os specific implementation 
// of io_device

#ifdef _WIN32
    #include "win32/win32_io_device.h"
#else
    #include "linux/linux_io_device.h"
#endif

#endif // ___cow_io_device_h___
