HEADERS += Arranger.h ArrangerPanel.h

SOURCES += main.cpp Arranger.cpp ArrangerPanel.cpp

CONFIG += debug console

QMAKE_CXXFLAGS += -std=c++17

QT += widgets

TARGET = arranger
