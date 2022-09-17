QT -= gui

CONFIG += console
CONFIG -= app_bundle

QMAKE_CXXFLAGS += -std=c++20

INCLUDEPATH += ../../include
HEADERS += ../../src/OrderedKeyMap.hpp
SOURCES +=  main.cpp
