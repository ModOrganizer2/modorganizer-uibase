cmake_minimum_required(VERSION 3.16)

find_package(Qt6 CONFIG REQUIRED COMPONENTS Network Qml Quick QuickWidgets Widgets)
find_package(spdlog CONFIG REQUIRED)
find_package(Boost REQUIRED CONFIG COMPONENTS thread)

add_library(uibase SHARED)
mo2_configure_target(uibase WARNINGS ON EXTERNAL_WARNINGS ON TRANSLATIONS OFF)
set_target_properties(uibase PROPERTIES MO2_TARGET_TYPE "uibase")

target_include_directories(uibase PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/game_features>
	$<INSTALL_INTERFACE:include/game_features>)

target_compile_definitions(uibase PRIVATE -DUIBASE_EXPORT SPDLOG_USE_STD_FORMAT)

target_link_libraries(uibase
	PUBLIC Qt6::Widgets Qt6::Network Qt6::QuickWidgets
	PRIVATE Boost::headers Boost::thread spdlog::spdlog_header_only Qt6::Qml Qt6::Quick)

mo2_add_filter(NAME src/interfaces GROUPS
	ifiletree
	imoinfo
	installationtester
	iplugin
	iplugindiagnose
	ipluginfilemapper
	iplugingame
	iplugingamefeatures
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

mo2_add_filter(NAME src/tutorials GROUPS
	tutorabledialog
	tutorialcontrol
	tutorialmanager
)

mo2_add_filter(NAME src/widgets GROUPS
	finddialog
	lineeditclear
	questionboxmemory
	sortabletreewidget
	taskprogressmanager
	textviewer
	expanderwidget
    filterwidget
	linklabel
	widgetutility
)

get_target_property(game_features uibase SOURCES)
list(FILTER game_features INCLUDE REGEX ".*game_features.*")
list(TRANSFORM game_features REPLACE ".*game_features[/\\](.*)[.]h" "game_features/\\1")

mo2_add_filter(NAME src/game_features GROUPS
	${game_features}
)

get_target_property(formatters uibase SOURCES)
list(FILTER formatters INCLUDE REGEX ".*formatters.*")
list(TRANSFORM formatters REPLACE ".*formatters[/\\](.*)[.]h" "formatters/\\1")

mo2_add_filter(NAME src/formatters GROUPS
	${formatters}
)

if (NOT UIBASE_INSTALL)
	mo2_install_target(uibase)
else()
	# installation
	get_target_property(uibase_headers uibase SOURCES)
	list(FILTER uibase_headers INCLUDE REGEX ".*\\.h")
	list(FILTER uibase_headers EXCLUDE REGEX ".*uibase_autogen.*")
	list(FILTER uibase_headers EXCLUDE REGEX ".*pch.h")
	target_sources(uibase
		PUBLIC
			FILE_SET HEADERS
			BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
			FILES ${uibase_headers}
	)

	install(TARGETS uibase EXPORT uibaseTargets FILE_SET HEADERS)
	install(EXPORT uibaseTargets
		FILE mo2-uibase-targets.cmake
		NAMESPACE mo2::
		DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mo2-uibase
	)
endif()
