# -------------------------------------------------
# Project created by QtCreator 2010-03-28T18:30:55
# -------------------------------------------------
QT += phonon
TARGET = cow_player
TEMPLATE = app
SOURCES += ../src/main.cpp \
    ../src/main_window.cpp \
    ../src/piece_dialog.cpp \
    ../src/piece_widget.cpp \
    ../src/client_configuration.cpp \
    ../src/video_player.cpp \
    ../src/configuration.cpp \
    ../../external_libraries/tinyxml/tinyxml.cpp \
    ../../external_libraries/tinyxml/tinyxmlerror.cpp \
    ../../external_libraries/tinyxml/tinyxmlparser.cpp \
    ../src/select_program_dialog.cpp \
    ../src/settings_dialog.cpp
HEADERS += ../src/main_window.h \
    ../src/piece_dialog.h \
    ../src/piece_widget.h \
    ../src/video_player.h \
    ../src/select_program_dialog.h \
    ../src/settings_dialog.h
FORMS += ../src/main_window.ui \
    ../src/piece_dialog.ui \
    ../src/piece_widget.ui \
    ../src/select_program_dialog.ui \
    ../src/settings_dialog.ui
INCLUDEPATH = ../src/ \
    ../../external_libraries/tinyxml/ \
    ../../libcow/include/
DEFINES += TIXML_USE_STL
MOC_DIR = moc
UI_DIR = ui
