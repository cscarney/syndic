QT += core qml gui widgets quick quickcontrols2

SOURCES += \
    src/basicfeedstorage.cpp \
    src/feedlistmodel.cpp \
    src/feedmanager.cpp \
    src/itemmodel.cpp \
    src/main.cpp \
    src/ttrssfeedsource.cpp
    src/resources.qrc

RESOURCES += \
    src/resources.qrc

HEADERS += \
    src/basicfeedstorage.h \
    src/feedlistmodel.h \
    src/feedmanager.h \
    src/feedsource.h \
    src/feedstorage.h \
    src/itemmodel.h \
    src/ttrssfeedsource.h

CONFIG += optimize_full ltcg

linux : !nokf5 {
    DEFINES += HAVE_KF5
    QT += KDeclarative KCrash KI18n KDBusAddons
    LIBS += -lKF5QuickAddons
}

QMAKE_CXXFLAGS += -std=c++17
QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter
QMAKE_CXXFLAGS_DEBUG+=-Werror

ltcg:linux-clang {
    QMAKE_LFLAGS += -pie
}

isEmpty(PREFIX):PREFIX=/usr/local
target.path = $$PREFIX/bin
INSTALLS += target
