cmake_minimum_required(VERSION 3.16)

add_library(uibase SHARED)
mo2_configure_uibase(uibase
	WARNINGS ON
	EXTERNAL_WARNINGS ON
	TRANSLATIONS OFF
	PUBLIC_DEPENDS Qt::Widgets Qt::Network Qt::QuickWidgets
	PRIVATE_DEPENDS boost boost::thread Qt::Qml Qt::Quick spdlog)
target_compile_definitions(uibase PRIVATE -DUIBASE_EXPORT)
mo2_install_target(uibase)

mo2_add_filter(NAME src/interfaces GROUPS
	idownloadmanager
	extension
	ifiletree
	iinstallationmanager
	imodinterface
	imodlist
	imodrepositorybridge
	imoinfo
	ipluginlist
	iprofile
)

mo2_add_filter(NAME src/plugins GROUPS
	executableinfo
	filemapping
	iplugin
	iplugindiagnose
	ipluginfilemapper
	iplugingame
	iplugingamefeatures
	iplugininstaller
	iplugininstallercustom
	iplugininstallersimple
	ipluginmodpage
	ipluginpreview
	ipluginproxy
	iplugintool
	isavegame
	isavegameinfowidget
	pluginrequirements
	pluginsetting
)

mo2_add_filter(NAME src/tutorials GROUPS
	tutorabledialog
	tutorialcontrol
	tutorialmanager
)

mo2_add_filter(NAME src/versioning GROUPS
	version
	versionrange
	versioninfo
)


mo2_add_filter(NAME src/utility GROUPS
	delayedfilewriter
	diagnosisreport
	eventfilter
	errorcodes
	exceptions
	filesystemutilities
	guessedvalue
	json
	log
	memoizedlock
	moassert
	registry
	report
	safewritefile
	scopeguard
	steamutility
	utility
)

mo2_add_filter(NAME src/widgets GROUPS
	expanderwidget
	finddialog
	lineeditclear
	linklabel
	questionboxmemory
	sortabletreewidget
	taskprogressmanager
	textviewer
	widgetutility
    filterwidget
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
	formatters
	${formatters}
)
