HEADERS += Arranger.h ArrangerPanel.h Targa.h

SOURCES += main.cpp Arranger.cpp ArrangerPanel.cpp Targa.cpp

CONFIG += debug console

QMAKE_CXXFLAGS += -std=c++17

QT += widgets

TARGET = arranger
