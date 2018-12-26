CMAKE_MINIMUM_REQUIRED (VERSION 2.8.11)

CMAKE_POLICY(SET CMP0020 NEW)
#CMAKE_POLICY(SET CMP0043 NEW)

SET(PROJ_AUTHOR Tannin)
SET(PROJ_ARCH x86)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  SET(PROJ_ARCH x64)
endif()


SET(uibase_SRCS
    utility.cpp
    textviewer.cpp
    finddialog.cpp
    report.cpp
    versioninfo.cpp
    lineeditclear.cpp
    mytree.cpp
    installationtester.cpp
    tutorialmanager.cpp
    tutorialcontrol.cpp
    tutorabledialog.cpp
    scopeguard.cpp
    pluginsetting.cpp
    diagnosisreport.cpp
    directorytree.cpp
    iplugininstaller.cpp
    guessedvalue.cpp
    json.cpp
    imodrepositorybridge.cpp
    nxmurl.cpp
    taskprogressmanager.cpp
    questionboxmemory.cpp
    modrepositoryfileinfo.cpp
    sortabletreewidget.cpp
    executableinfo.cpp
    delayedfilewriter.cpp
    filenamestring.cpp
    safewritefile.cpp
    registry.cpp
  )

SET(uibase_HDRS
    iplugin.h
    iplugintool.h
    iplugininstaller.h
    iplugininstallersimple.h
    iplugininstallercustom.h
    iplugindiagnose.h
    ipluginproxy.h
    ipluginlist.h
    ipluginmodpage.h
    ipluginpreview.h
    iplugingame.h
    ipluginfilemapper.h
    utility.h
    textviewer.h
    finddialog.h
    report.h
    versioninfo.h
    imoinfo.h
    imodinterface.h
    lineeditclear.h
    dllimport.h
    directorytree.h
    mytree.h
    installationtester.h
    tutorialmanager.h
    tutorialcontrol.h
    tutorabledialog.h
    iinstallationmanager.h
    scopeguard.h
    pluginsetting.h
    diagnosisreport.h
    guessedvalue.h
    json.h
    imodrepositorybridge.h
    idownloadmanager.h
    nxmurl.h
    taskprogressmanager.h
    questionboxmemory.h
    sortabletreewidget.h
    imodlist.h
    modrepositoryfileinfo.h
    executableinfo.h
    iprofile.h
    delayedfilewriter.h
    filenamestring.h
    filemapping.h
    safewritefile.h
    registry.h
  )

SET(UIS
    finddialog.ui
    questionboxmemory.ui
    textviewer.ui
  )


SET(CMAKE_INCLUDE_CURRENT_DIR ON)
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON)
FIND_PACKAGE(Qt5Widgets REQUIRED)
FIND_PACKAGE(Qt5WinExtras REQUIRED)
FIND_PACKAGE(Qt5Qml REQUIRED)
FIND_PACKAGE(Qt5QuickWidgets REQUIRED)
QT5_WRAP_UI(uibase_UIHDRS ${UIS})

INCLUDE_DIRECTORIES(${Qt5Declarative_INCLUDES})

SET(Boost_USE_STATIC_LIBS        ON)
SET(Boost_USE_MULTITHREADED      ON)
SET(Boost_USE_STATIC_RUNTIME    OFF)
FIND_PACKAGE(Boost REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

ADD_DEFINITIONS(-DUIBASE_EXPORT)

ADD_LIBRARY(uibase SHARED ${uibase_HDRS} ${uibase_SRCS} ${uibase_UIHDRS} ${UIS} ${RSCS} ${TRS} ${MOCS})
TARGET_LINK_LIBRARIES(uibase Qt5::Widgets Qt5::WinExtras Qt5::Qml Qt5::QuickWidgets ${Boost_LIBRARIES})

IF (MSVC)
  SET_TARGET_PROPERTIES(uibase PROPERTIES COMPILE_FLAGS "/std:c++latest")
ENDIF()
IF (MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 4)
  # 32 bits
  SET_TARGET_PROPERTIES(uibase PROPERTIES LINK_FLAGS "/LARGEADDRESSAWARE")
ENDIF()

IF (NOT "${OPTIMIZE_COMPILE_FLAGS}" STREQUAL "")
  SET_TARGET_PROPERTIES(${PROJ_NAME} PROPERTIES COMPILE_FLAGS_RELWITHDEBINFO ${OPTIMIZE_COMPILE_FLAGS})
ENDIF()
IF (NOT "${OPTIMIZE_LINK_FLAGS}" STREQUAL "")
  SET_TARGET_PROPERTIES(${PROJ_NAME} PROPERTIES LINK_FLAGS_RELWITHDEBINFO ${OPTIMIZE_LINK_FLAGS})
ENDIF()

###############
## Installation

INSTALL(TARGETS uibase
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION libs)
INSTALL(FILES $<TARGET_PDB_FILE:uibase>
        DESTINATION pdb)
