CMAKE_MINIMUM_REQUIRED (VERSION 2.8.11)

CMAKE_POLICY(SET CMP0020 NEW)
#CMAKE_POLICY(SET CMP0043 NEW)

SET(PROJ_AUTHOR Tannin)
SET(PROJ_ARCH x86)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  SET(PROJ_ARCH x64)
endif()

macro(add_msvc_precompiled_header PrecompiledHeader PrecompiledSource SourcesVar HeadersVar MocSource)
  if(MSVC)
    get_filename_component(PrecompiledBasename ${PrecompiledHeader} NAME_WE)
    set(PrecompiledBinary "${CMAKE_CURRENT_BINARY_DIR}/${PrecompiledBasename}.pch")
    set(Sources ${${SourcesVar}})

    set_source_files_properties(
		${PrecompiledSource} PROPERTIES
		COMPILE_FLAGS "/Yc\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
		OBJECT_OUTPUTS "${PrecompiledBinary}")

    set_source_files_properties(
		${Sources} ${MocSource} PROPERTIES
		COMPILE_FLAGS "/Yu\"${PrecompiledHeader}\" /FI\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
		OBJECT_DEPENDS "${PrecompiledBinary}")

    list(APPEND ${SourcesVar} ${PrecompiledSource})
	list(APPEND ${HeadersVar} ${PrecompiledHeader})
  endif(MSVC)
endmacro(add_msvc_precompiled_header)


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
    steamutility.cpp
	log.cpp
	expanderwidget.cpp
	errorcodes.cpp
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
	isavegame.h
	isavegameinfowidget.h
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
    steamutility.h
	log.h
	expanderwidget.h
	errorcodes.h
  )

SET(UIS
    finddialog.ui
    questionboxmemory.ui
    textviewer.ui
  )

SET(uibase_RCS
    version.rc
	)


source_group(src REGULAR_EXPRESSION ".*\\.(h|cpp|ui)")

set(interfaces
	imoinfo
	installationtester
	iplugin
	iplugindiagnose
	ipluginfilemapper
	iplugingame
	iplugininstaller
	iplugininstallercustom
	iplugininstallersimple
	ipluginlist
	ipluginmodpage
	ipluginpreview
	ipluginproxy
	iplugintool
	iprofile
	isavegame
	isavegameinfowidget
	iinstallationmanager
	imodinterface
	imodlist
	imodrepositorybridge
)

set(tutorials
	tutorabledialog
	tutorialcontrol
	tutorialmanager
)

set(widgets
	finddialog
	lineeditclear
	questionboxmemory
	sortabletreewidget
	taskprogressmanager
	textviewer
	expanderwidget
)

set(src_filters interfaces tutorials widgets
)

foreach(filter in list ${src_filters})
	set(files)
	foreach(d in lists ${${filter}})
		set(files ${files} ${d}.cpp ${d}.h ${d}.inc ${d}.ui)
	endforeach()

	source_group(src\\${filter} FILES ${files})
endforeach()

source_group(cmake FILES CMakeLists.txt)
source_group(resources FILES ${uibase_RCS})

add_msvc_precompiled_header(
	"pch.h" "pch.cpp"
	uibase_SRCS uibase_HDRS
	"${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}_autogen/mocs_compilation.cpp")

message("${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}_autogen/mocs_compilation.cpp")

SET(CMAKE_INCLUDE_CURRENT_DIR ON)
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON)

set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP autogen)
set_property(GLOBAL PROPERTY AUTOMOC_SOURCE_GROUP autogen)
set_property(GLOBAL PROPERTY AUTORCC_SOURCE_GROUP autogen)

# make the automoc file include pch.h so warnings are disabled in it
set(CMAKE_AUTOMOC_MOC_OPTIONS "-bpch.h")

FIND_PACKAGE(Qt5Widgets REQUIRED)
FIND_PACKAGE(Qt5WinExtras REQUIRED)
FIND_PACKAGE(Qt5Qml REQUIRED)
FIND_PACKAGE(Qt5QuickWidgets REQUIRED)

INCLUDE_DIRECTORIES(${Qt5Declarative_INCLUDES})

SET(Boost_USE_STATIC_LIBS        ON)
SET(Boost_USE_MULTITHREADED      ON)
SET(Boost_USE_STATIC_RUNTIME    OFF)
FIND_PACKAGE(Boost REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

FIND_PACKAGE(fmt REQUIRED)
INCLUDE_DIRECTORIES(${SPDLOG_ROOT}/include)

ADD_DEFINITIONS(-DUIBASE_EXPORT)

ADD_LIBRARY(uibase SHARED ${uibase_HDRS} ${uibase_SRCS} ${uibase_RCS} ${UIS} ${RSCS} ${TRS} ${MOCS})
TARGET_LINK_LIBRARIES(uibase Qt5::Widgets Qt5::WinExtras Qt5::Qml Qt5::QuickWidgets ${Boost_LIBRARIES} fmt::fmt)

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
