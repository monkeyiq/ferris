#!/bin/sh

g++  modeltestqml.cpp  -o modeltestqml -I /ferris -I/ferris/Ferris `pkg-config ferrisloki glib-2.0 --cflags --libs`  -L/usr/local/NokiaQtSDK64_v1.0.1/Simulator/Qt/gcc/lib \
 -lQtCore -lQtGui -lQtNetwork -lQtDeclarative  \
 -L/tmp/DevelopBuild/ferris/FerrisQtGui/.libs -lferrisqtgui \
 -lferris \
 -I/usr/local/NokiaQtSDK64_v1.0.1/Simulator/Qt/gcc/include/QtCore -I/usr/local/NokiaQtSDK64_v1.0.1/Simulator/Qt/gcc/include/QtGui -I/usr/local/NokiaQtSDK64_v1.0.1/Simulator/Qt/gcc/include/QtNetwork -I/usr/local/NokiaQtSDK64_v1.0.1/Simulator/Qt/gcc/include/QtDeclarative -I/usr/local/NokiaQtSDK64_v1.0.1/Simulator/Qt/gcc/include


#   `pkg-config QtGui --cflags --libs`
