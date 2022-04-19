cmake_minimum_required(VERSION 3.16)

find_package(Qt5 COMPONENTS Widgets Network Qml Quick QuickWidgets WinExtras REQUIRED)
find_package(fmt REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread)

mo2_glob_files(input_files)

add_library(uibase SHARED ${input_files})

mo2_configure_target(uibase)

target_compile_definitions(uibase PRIVATE -DUIBASE_EXPORT)
target_include_directories(uibase PRIVATE ${Boost_INCLUDE_DIRS} ${SPDLOG_ROOT}/include)
target_link_libraries(uibase
	PUBLIC Qt5::Widgets Qt5::Network Qt5::QuickWidgets
	PRIVATE Qt5::WinExtras Qt5::Qml Qt5::Quick fmt::fmt Version)

mo2_install_target(uibase)

mo2_add_filter(NAME src/interfaces GROUPS
	ifiletree
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
