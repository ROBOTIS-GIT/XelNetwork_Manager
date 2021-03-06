QT += widgets serialport printsupport

TARGET = XelNetwork_Manager
TEMPLATE = app

message($${QMAKE_HOST.os})

equals(QMAKE_HOST.os,Windows) {
DXLSDK_PORT = windows
CONFIG += c++11
} else:equals(QMAKE_HOST.os,Linux) {
DXLSDK_PORT = linux
message("linux")
}

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    settingsdialog.cpp \
    console.cpp \
    log_debug.cpp \
    DynamixelSDK/src/dynamixel_sdk/group_bulk_read.cpp \
    DynamixelSDK/src/dynamixel_sdk/group_bulk_write.cpp \
    DynamixelSDK/src/dynamixel_sdk/group_sync_read.cpp \
    DynamixelSDK/src/dynamixel_sdk/group_sync_write.cpp \
    DynamixelSDK/src/dynamixel_sdk/packet_handler.cpp \
    DynamixelSDK/src/dynamixel_sdk/port_handler.cpp \
    DynamixelSDK/src/dynamixel_sdk/protocol1_packet_handler.cpp \
    DynamixelSDK/src/dynamixel_sdk/protocol2_packet_handler.cpp \
    DynamixelSDK/src/dynamixel_sdk_$${DXLSDK_PORT}/port_handler_$${DXLSDK_PORT}.cpp \
    tab_terminal.cpp \
    tab_xel.cpp \

INCLUDEPATH += DynamixelSDK/include \
    xel_loader/include \
    qtcsv \
    qtcsv/include

QMAKE_CXXFLAGS += -std=c++0x -Wno-unused-parameter

HEADERS += \
    mainwindow.h \
    settingsdialog.h \
    console.h \
    log_debug.h \
    DynamixelSDK/include/dynamixel_sdk/group_bulk_read.h \
    DynamixelSDK/include/dynamixel_sdk/group_bulk_write.h \
    DynamixelSDK/include/dynamixel_sdk/group_sync_read.h \
    DynamixelSDK/include/dynamixel_sdk/group_sync_write.h \
    DynamixelSDK/include/dynamixel_sdk/packet_handler.h \
    DynamixelSDK/include/dynamixel_sdk/port_handler.h \
    DynamixelSDK/include/dynamixel_sdk/protocol1_packet_handler.h \
    DynamixelSDK/include/dynamixel_sdk/protocol2_packet_handler.h \
    DynamixelSDK/include/dynamixel_sdk_$${DXLSDK_PORT}/port_handler_$${DXLSDK_PORT}.h \
    DynamixelSDK/include/dynamixel_sdk.h \
    tab_terminal.h \
    tab_xel.h \

FORMS += \
    mainwindow.ui \
    settingsdialog.ui \
    tab_terminal.ui \
    tab_xel.ui

RESOURCES += \
    xel_network.qrc

target.path = ./target
INSTALLS += target
