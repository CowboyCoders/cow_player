project(COW_PLAYER)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE Debug)  

if(WIN32)
    # Use static MT version of Boost
    SET(Boost_USE_STATIC_LIBS ON)
    SET(Boost_USE_MULTITHREAD ON)
endif(WIN32)

find_package(Boost COMPONENTS system thread log filesystem REQUIRED)
find_package(Qt4 REQUIRED)

# see http://cmake.org/cmake/help/cmake2.6docs.html#module:FindQt4 for a complete list
set(QT_USE_PHONON TRUE)

if(NOT EXISTS cow_player_config.xml)
    file(COPY ${COW_PLAYER_SOURCE_DIR}/src/cow_player_config.xml DESTINATION .)
endif(NOT EXISTS cow_player_config.xml)

if(UNIX)
    ####################### IMPORTANT ###########################
    # You must set the enviroment variable LIBCOW_PATH in your  #
    # .bashrc to be able to link to libcow!                     #
    #                                                           #
    # You can do this by executing:                             #
    #   foo@computer:~$ gedit ~/.bashrc                         #
    #                                                           #
    # At the end of the file add the following:                 #
    #   export LIBCOW_PATH=/path/to/p2pmediaplayer/trunk/libcow #
    #                                                           #
    # Then log out and log in, and the variable will be set!    #
    # You can now build cow_player!                             #
    ####################### IMPORTANT ###########################

    set(LIBCOW_INCLUDE_DIR $ENV{LIBCOW_PATH}/include)
    set(LIBCOW_LIBRARY $ENV{LIBCOW_PATH}/bin/libcowd.so)
    
    set(QT_QMAKE_EXECUTABLE /usr/local/lib/qtsdk-2010.02/qt/bin/qmake)
endif(UNIX)

include(${QT_USE_FILE})

if(UNIX)
	include_directories(
	    ${COW_PLAYER_SOURCE_DIR}/src
	    ${COW_PLAYER_SOURCE_DIR}/../utils/include
	    ${COW_PLAYER_SOURCE_DIR}/../external_libraries/tinyxml
        ${LIBCOW_INCLUDE_DIR} 
	    ${Boost_INCLUDE_DIRS}
	    ${QT_INCLUDES}
	    
	    # Don't forget to include output directory, otherwise
	    # the UI file won't be wrapped!
	    ${CMAKE_CURRENT_BINARY_DIR}    
	)
endif(UNIX)
if(WIN32)
	include_directories(
	    ${COW_PLAYER_SOURCE_DIR}/include
        ${COW_PLAYER_SOURCE_DIR}/../libcow/include
	    ${COW_PLAYER_SOURCE_DIR}/../utils/include
	    ${COW_PLAYER_SOURCE_DIR}/../external_libraries/tinyxml
	    ${Boost_INCLUDE_DIRS}
	    ${QT_INCLUDES}
    
	    # Don't forget to include output directory, otherwise
	    # the UI file won't be wrapped!
	    ${CMAKE_CURRENT_BINARY_DIR}    
	)
endif(WIN32)

link_directories(
    ${Boost_LIBRARY_DIRS}
)

add_definitions(${QT_DEFINITIONS})
add_definitions(-DTIXML_USE_STL)

IF(WIN32)
    add_definitions(
        -DCURL_STATICLIB 
        -DBOOST_ALL_NO_LIB
        -DTORRENT_BUILDING_SHARED
        -DTORRENT_DEBUG
        -DTORRENT_DISABLE_ENCRYPTION
        -DTORRENT_DISABLE_GEO_IP
        -DTORRENT_DISABLE_INVARIANT_CHECKS
    )
ENDIF(WIN32)

set(cow_player_SRCS
    ${COW_PLAYER_SOURCE_DIR}/src/main.cpp
    ${COW_PLAYER_SOURCE_DIR}/src/main_window.cpp
    ${COW_PLAYER_SOURCE_DIR}/src/piece_dialog.cpp
    ${COW_PLAYER_SOURCE_DIR}/src/select_program_dialog.cpp
    ${COW_PLAYER_SOURCE_DIR}/src/settings_dialog.cpp
    ${COW_PLAYER_SOURCE_DIR}/src/piece_widget.cpp
    ${COW_PLAYER_SOURCE_DIR}/src/video_player.cpp    
    ${COW_PLAYER_SOURCE_DIR}/src/configuration.cpp
    ${COW_PLAYER_SOURCE_DIR}/src/client_configuration.cpp
    ${COW_PLAYER_SOURCE_DIR}/src/cow_io_device.cpp
    ${COW_PLAYER_SOURCE_DIR}/../external_libraries/tinyxml/tinyxml.cpp
    ${COW_PLAYER_SOURCE_DIR}/../external_libraries/tinyxml/tinyxmlerror.cpp
    ${COW_PLAYER_SOURCE_DIR}/../external_libraries/tinyxml/tinyxmlparser.cpp
)

set(cow_player_INCLUDES
    ${COW_PLAYER_SOURCE_DIR}/src/configuration.h
    ${COW_PLAYER_SOURCE_DIR}/src/client_configuration.h
    ${COW_PLAYER_SOURCE_DIR}/src/cow_io_device.h
    ${COW_PLAYER_SOURCE_DIR}/src/list_item.h
    ${COW_PLAYER_SOURCE_DIR}/../external_libraries/tinyxml/tinyxml.h
)

set(cow_player_MOCS
    ${COW_PLAYER_SOURCE_DIR}/src/main_window.h
    ${COW_PLAYER_SOURCE_DIR}/src/piece_widget.h
    ${COW_PLAYER_SOURCE_DIR}/src/video_player.h
    ${COW_PLAYER_SOURCE_DIR}/src/piece_dialog.h    
    ${COW_PLAYER_SOURCE_DIR}/src/select_program_dialog.h
    ${COW_PLAYER_SOURCE_DIR}/src/settings_dialog.h
)

SET(cow_player_UIS
    ${COW_PLAYER_SOURCE_DIR}/src/main_window.ui
    ${COW_PLAYER_SOURCE_DIR}/src/piece_widget.ui
    ${COW_PLAYER_SOURCE_DIR}/src/piece_dialog.ui    
    ${COW_PLAYER_SOURCE_DIR}/src/select_program_dialog.ui    
    ${COW_PLAYER_SOURCE_DIR}/src/settings_dialog.ui    
)

# build ui_XXX files from the XML-style .ui files
QT4_WRAP_UI(cow_player_SRCS ${cow_player_UIS})
# this moc's the above variable and appends to the cxx sources
QT4_WRAP_CPP(cow_player_SRCS ${cow_player_MOCS})

# uses /SUBSYSTEM:WINDOWS in msvc
add_executable(cow_player WIN32 ${cow_player_SRCS} ${cow_player_INCLUDES} ${cow_player_MOCS})

IF(WIN32)
    # Set exception handling model
    set_target_properties(cow_player PROPERTIES COMPILE_FLAGS "/EHa")
ENDIF(WIN32)

if(UNIX)
    target_link_libraries(cow_player 
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_THREAD_LIBRARY}
        ${Boost_LOG_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
        ${QT_LIBRARIES}
        ${LIBCOW_LIBRARY}
    )
endif(UNIX)

if(WIN32)
    target_link_libraries(cow_player    
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_THREAD_LIBRARY}
        ${Boost_LOG_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
        ${QT_LIBRARIES}
        
        ws2_32
        winmm
        wldap32
               
        torrent
        
        debug qtmaind
        debug cowd
            
        optimized qtmain
        optimized cow
    )
    
endif(WIN32)

