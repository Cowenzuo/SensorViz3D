#include "NativeBaseWindow.h"

#include <dwmapi.h>
#include <windows.h>
#include <windowsx.h>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QWindow>

NativeBaseWindow::NativeBaseWindow(QWidget* parent) : QMainWindow(parent), _resizable(true), _resizeBorderWidth(8) {
  // 去掉标题栏
  HWND hwnd = (HWND)this->winId();
  DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
  ::SetWindowLong(hwnd, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION | WS_BORDER);

  // 阴影
  const MARGINS shadow = {1, 1, 1, 1};
  DwmExtendFrameIntoClientArea(HWND(winId()), &shadow);

  installEventFilter(this);

  connect(this->windowHandle(), &QWindow::screenChanged, this, &NativeBaseWindow::onScreenChanged);
}

NativeBaseWindow::~NativeBaseWindow() {
}

bool NativeBaseWindow::nativeEvent(const QByteArray& eventType, void* message, long* result) {
  MSG* msg = reinterpret_cast<MSG*>(message);

  switch (msg->message) {
    case WM_NCCALCSIZE: {
      // NCCALCSIZE_PARAMS &params = *reinterpret_cast<NCCALCSIZE_PARAMS *>(msg->lParam);
      // if (params.rgrc[0].top != 0)
      //     params.rgrc[0].top -= 1;

      *result = WVR_REDRAW;
      return true;
    }
    case WM_NCHITTEST: {
      *result = 0;

      const LONG border_width = _resizeBorderWidth;
      RECT winrect;
      GetWindowRect(HWND(winId()), &winrect);

      long x = GET_X_LPARAM(msg->lParam);
      long y = GET_Y_LPARAM(msg->lParam);

      if (_resizable) {
        bool resizeWidth = minimumWidth() != maximumWidth();
        bool resizeHeight = minimumHeight() != maximumHeight();

        if (resizeWidth) {
          if (x >= winrect.left && x < winrect.left + border_width) {
            *result = HTLEFT;
          }
          if (x < winrect.right && x >= winrect.right - border_width) {
            *result = HTRIGHT;
          }
        }
        if (resizeHeight) {
          if (y < winrect.bottom && y >= winrect.bottom - border_width) {
            *result = HTBOTTOM;
          }
          if (y >= winrect.top && y < winrect.top + border_width) {
            *result = HTTOP;
          }
        }
        if (resizeWidth && resizeHeight) {
          if (x >= winrect.left && x < winrect.left + border_width && y < winrect.bottom && y >= winrect.bottom - border_width) {
            *result = HTBOTTOMLEFT;
          }
          if (x < winrect.right && x >= winrect.right - border_width && y < winrect.bottom && y >= winrect.bottom - border_width) {
            *result = HTBOTTOMRIGHT;
          }
          if (x >= winrect.left && x < winrect.left + border_width && y >= winrect.top && y < winrect.top + border_width) {
            *result = HTTOPLEFT;
          }
          if (x < winrect.right && x >= winrect.right - border_width && y >= winrect.top && y < winrect.top + border_width) {
            *result = HTTOPRIGHT;
          }
        }
      }
      if (0 != *result) {
        return true;
      }

      QPoint pt = mapFromGlobal(QPoint(x, y));
      if (hitTestCaption(pt)) {
        *result = HTCAPTION;
        return true;
      }
    }
    case WM_GETMINMAXINFO: {
      if (::IsZoomed(msg->hwnd)) {
        RECT frame = {0, 0, 0, 0};
        AdjustWindowRectEx(&frame, WS_OVERLAPPEDWINDOW, FALSE, 0);
        frame.left = abs(frame.left);
        frame.top = abs(frame.bottom);
        this->setContentsMargins(frame.left, frame.top, frame.right, frame.bottom);
      } else {
        this->setContentsMargins(0, 0, 0, 0);
      }
      return false;
    }
    default:
      break;
  }

  return QWidget::nativeEvent(eventType, message, result);
}

bool NativeBaseWindow::hitTestCaption(const QPoint& pos) {
  return false;
}

// 解决窗口拖动切换到不同显示器时窗口刷新问题
void NativeBaseWindow::onScreenChanged(QScreen* screen) {
  SetWindowPos((HWND)winId(), nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE);
}
