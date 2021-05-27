#pragma warning(disable: 4251)  // neds to have dll-interface
#pragma warning(disable: 4355)  // this used in initializer list
#pragma warning(disable: 4371)  // layout may have changed
#pragma warning(disable: 4514)  // unreferenced inline function removed
#pragma warning(disable: 4571)  // catch semantics changed
#pragma warning(disable: 4619)  // no warning X
#pragma warning(disable: 4623)  // default constructor deleted
#pragma warning(disable: 4625)  // copy constructor deleted
#pragma warning(disable: 4626)  // copy assignment operator deleted
#pragma warning(disable: 4710)  // function not inlined
#pragma warning(disable: 4820)  // padding
#pragma warning(disable: 4866)  // left-to-right evaluation order
#pragma warning(disable: 4868)  // left-to-right evaluation order
#pragma warning(disable: 5026)  // move constructor deleted
#pragma warning(disable: 5027)  // move assignment operator deleted
#pragma warning(disable: 5045)  // spectre mitigation

#pragma warning(push, 3)
#pragma warning(disable: 4365)  // signed/unsigned mismatch
#pragma warning(disable: 4774)  // bad format string
#pragma warning(disable: 4946)  // reinterpret_cast used between related classes
#pragma warning(disable: 4800)  // implicit conversion

#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING 1

// std
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>
#include <wchar.h>

// fmt
#include <fmt/format.h>

// windows
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif
#include <ShlObj.h>
#include <shobjidl.h>
#include <Windows.h>

// Qt
#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QBitmap>
#include <QBuffer>
#include <QCommandLinkButton>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDesktopWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDropEvent>
#include <QFile>
#include <QFileInfo>
#include <QFlags>
#include <QGraphicsObject>
#include <QIcon>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QMetaEnum>
#include <QMetaMethod>
#include <qmetaobject.h>
#include <QMetaObject>
#include <QMetaType>
#include <QMouseEvent>
#include <QMutex>
#include <QMutexLocker>
#include <QNetworkReply>
#include <QObject>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWidget>
#include <QRect>
#include <QRegExp>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QSettings>
#include <QShortcutEvent>
#include <QShowEvent>
#include <QString>
#include <QStringList>
#include <QStyle>
#include <QTableWidget>
#include <QTabWidget>
#include <QtDebug>
#include <QTemporaryFile>
#include <QTextCodec>
#include <QTextEdit>
#include <QTextStream>
#include <QTime>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QtWinExtras/QtWin>
#include <QUrl>
#include <QUrlQuery>
#include <QVariant>
#include <QVariantMap>
#include <QVersionNumber>
#include <QVBoxLayout>
#include <QWidget>

#undef _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#pragma warning(pop)
