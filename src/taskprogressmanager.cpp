#include "taskprogressmanager.h"
#include <QApplication>
#include <QWidget>
#include <QMainWindow>
#include "comdef.h"

namespace MOBase {

TaskProgressManager &TaskProgressManager::instance()
{
  static TaskProgressManager s_Instance;
  return s_Instance;
}

void TaskProgressManager::forgetMe(quint32 id)
{
  if (m_Taskbar == nullptr) {
    return;
  }
  auto iter = m_Percentages.find(id);
  if (iter != m_Percentages.end()) {
    m_Percentages.erase(iter);
  }
  showProgress();
}

void TaskProgressManager::updateProgress(quint32 id, qint64 value, qint64 max)
{
  QMutexLocker lock(&m_Mutex);
  if (m_Taskbar == nullptr) {
    return;
  }

  if (value == max) {
    auto iter = m_Percentages.find(id);
    if (iter != m_Percentages.end()) {
      m_Percentages.erase(iter);
    }
  } else {
    m_Percentages[id] = std::make_pair(QTime::currentTime(), (value * 100) / max);
  }

  showProgress();
}

quint32 TaskProgressManager::getId()
{
  QMutexLocker lock(&m_Mutex);
  return m_NextId++;
}


void TaskProgressManager::showProgress()
{
  if (!m_Percentages.empty()) {
    m_Taskbar->SetProgressState(m_WinId, TBPF_NORMAL);

    QTime now = QTime::currentTime();
    qint64 total = 0;
    int count = 0;

    for (auto iter = m_Percentages.begin(); iter != m_Percentages.end();) {
      if (iter->second.first.secsTo(now) < 15) {
        total += iter->second.second;
        ++iter;
        ++count;
      } else {
        // if there was no progress in 15 seconds remove this progress
        qDebug("no progress in 15 seconds (%d)", iter->second.first.secsTo(now));
        iter = m_Percentages.erase(iter);
      }
    }

    m_Taskbar->SetProgressValue(m_WinId, total, count * 100);
  } else {
    m_Taskbar->SetProgressState(m_WinId, TBPF_NOPROGRESS);
  }
}


bool TaskProgressManager::tryCreateTaskbar()
{
  // try to find our main window
  for (QWidget *widget : QApplication::topLevelWidgets()) {
    QMainWindow *mainWin = qobject_cast<QMainWindow*>(widget);
    if (mainWin != nullptr) {
      m_WinId = reinterpret_cast<HWND>(mainWin->winId());
    }
  }

  HRESULT result;
  if (m_WinId != nullptr) {
    result = CoCreateInstance(CLSID_TaskbarList, 0, CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(&m_Taskbar));
    if (result == S_OK) {
      return true;
    }
  }

  // if we got here we got no connection to the taskbar

  m_Taskbar = nullptr;

  if (m_CreateTries-- > 0) {
    QTimer::singleShot(1000, this, SLOT(tryCreateTaskbar()));
  } else {
    _com_error resString(result);
    qWarning("failed to create taskbar connection (this is to be expected on "
             "Windows XP): %s", resString.ErrorMessage());
  }
  return false;
}

TaskProgressManager::TaskProgressManager()
  : m_NextId(1), m_CreateTries(10), m_WinId(nullptr), m_Taskbar(nullptr)
{
}


} // namespace MOBase
