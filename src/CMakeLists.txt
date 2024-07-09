cmake_minimum_required(VERSION 3.16)

find_package(Qt6 CONFIG REQUIRED COMPONENTS Network Qml Quick QuickWidgets Widgets)
find_package(spdlog CONFIG REQUIRED)
find_package(Boost REQUIRED CONFIG COMPONENTS thread)

file(GLOB_RECURSE uibase_headers RELATIVE ${CMAKE_CURRENT_LIST_DIR} "${CMAKE_CURRENT_LIST_DIR}/../include/**/*.h")

add_library(uibase SHARED)
mo2_configure_target(uibase WARNINGS ON EXTERNAL_WARNINGS ON TRANSLATIONS OFF)
target_sources(uibase
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS ${CMAKE_CURRENT_LIST_DIR}/../include
		FILES ${uibase_headers}
)

# TODO: remove this after fixing UIBase #include<> directives
target_include_directories(uibase PRIVATE
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/uibase>
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/uibase/game_features>
)

add_library(mo2::uibase ALIAS uibase)

target_compile_definitions(uibase PRIVATE -DUIBASE_EXPORT SPDLOG_USE_STD_FORMAT)

target_link_libraries(uibase
	PUBLIC Qt6::Widgets Qt6::Network Qt6::QuickWidgets
	PRIVATE Boost::headers Boost::thread spdlog::spdlog_header_only Qt6::Qml Qt6::Quick)

# installation
install(TARGETS uibase EXPORT uibaseTargets FILE_SET HEADERS)
install(FILES $<TARGET_PDB_FILE:uibase> DESTINATION pdb)
install(EXPORT uibaseTargets
	FILE mo2-uibase-targets.cmake
	NAMESPACE mo2::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mo2-uibase
)

# filters
source_group(src FILES
	../include/uibase/delayedfilewriter.h
	../include/uibase/diagnosisreport.h
	../include/uibase/dllimport.h
	../include/uibase/errorcodes.h
	../include/uibase/eventfilter.h
	../include/uibase/exceptions.h
	../include/uibase/executableinfo.h
	../include/uibase/filemapping.h
	../include/uibase/filesystemutilities.h
	../include/uibase/guessedvalue.h
	../include/uibase/idownloadmanager.h
	../include/uibase/json.h
	../include/uibase/log.h
	../include/uibase/memoizedlock.h
	../include/uibase/moassert.h
	../include/uibase/modrepositoryfileinfo.h
	../include/uibase/nxmurl.h
	../include/uibase/pluginrequirements.h
	../include/uibase/pluginsetting.h
	../include/uibase/registry.h
	../include/uibase/report.h
	../include/uibase/safewritefile.h
	../include/uibase/scopeguard.h
	../include/uibase/steamutility.h
	../include/uibase/utility.h
	../include/uibase/versioninfo.h
)

source_group(src/interfaces FILES
	../include/uibase/ifiletree.h
	../include/uibase/iinstallationmanager.h
	../include/uibase/imodinterface.h
	../include/uibase/imodlist.h
	../include/uibase/imodrepositorybridge.h
	../include/uibase/imoinfo.h
	../include/uibase/installationtester.h
	../include/uibase/iplugin.h
	../include/uibase/iplugindiagnose.h
	../include/uibase/ipluginfilemapper.h
	../include/uibase/iplugingame.h
	../include/uibase/iplugingamefeatures.h
	../include/uibase/iplugininstaller.h
	../include/uibase/iplugininstallercustom.h
	../include/uibase/iplugininstallersimple.h
	../include/uibase/ipluginlist.h
	../include/uibase/ipluginmodpage.h
	../include/uibase/ipluginpreview.h
	../include/uibase/ipluginproxy.h
	../include/uibase/iplugintool.h
	../include/uibase/iprofile.h
	../include/uibase/isavegame.h
	../include/uibase/isavegameinfowidget.h

	ifiletree.cpp
	imodrepositorybridge.cpp
	imoinfo.cpp
	iplugininstaller.cpp
)

source_group(src/tutorials FILES
	../include/uibase/tutorabledialog.h
	../include/uibase/tutorialcontrol.h
	../include/uibase/tutorialmanager.h
	tutorabledialog.cpp
	tutorialcontrol.cpp
	tutorialmanager.cpp
)

source_group(src/widgets FILES
	../include/uibase/expanderwidget.h
	../include/uibase/filterwidget.h
	../include/uibase/finddialog.h
	../include/uibase/lineeditclear.h
	../include/uibase/linklabel.h
	../include/uibase/questionboxmemory.h
	../include/uibase/sortabletreewidget.h
	../include/uibase/taskprogressmanager.h
	../include/uibase/textviewer.h
	../include/uibase/widgetutility.h

	expanderwidget.cpp
	finddialog.cpp
	lineeditclear.cpp
	linklabel.cpp
	questionboxmemory.cpp
	sortabletreewidget.cpp
	taskprogressmanager.cpp
	textviewer.cpp
	widgetutility.cpp
    filterwidget.cpp
)

get_target_property(game_features uibase HEADER_SET_HEADERS)
list(FILTER game_features INCLUDE REGEX ".*game_features.*")
source_group(src/game_features FILES ${game_features})

get_target_property(formatters uibase HEADER_SET_HEADERS)
list(FILTER formatters INCLUDE REGEX ".*formatters.*")
source_group(src/formatters FILES ${formatters})
