#include "WindowManagerHelper.h"

#include <QApplication>
#include <QHoverEvent>
#include <QScreen>
#include <QWidget>
#include <QWindow>

WindowManagerHelper::WindowManagerHelper(QWidget *parent) : QObject(parent), d_ptr(new WindowManagerHelperPrivate(this))
{
}

WindowManagerHelper::~WindowManagerHelper()
{
}

void WindowManagerHelper::setControlledWidget(QWidget *widget)
{
    Q_D(WindowManagerHelper);
    d->setControlledWidget(widget);
}

QWidget *WindowManagerHelper::controlledWidget() const
{
    Q_D(const WindowManagerHelper);
    return d->_controlledWidget;
}

void WindowManagerHelper::setMoveEnabled(bool enabled)
{
    Q_D(WindowManagerHelper);
    d->_moveEnabled = enabled;
}

bool WindowManagerHelper::moveEnabled() const
{
    Q_D(const WindowManagerHelper);
    return d->_moveEnabled;
}

void WindowManagerHelper::setResizable(bool resizable)
{
    Q_D(WindowManagerHelper);
    d->setResizable(resizable);
}

bool WindowManagerHelper::resizable() const
{
    Q_D(const WindowManagerHelper);
    return d->_resizable;
}

void WindowManagerHelper::setResizeBorderWidth(int width)
{
    Q_D(WindowManagerHelper);
    d->setResizeBorderWidth(qBound(1, width, 20));
}

int WindowManagerHelper::resizeBorderWidth() const
{
    Q_D(const WindowManagerHelper);
    return d->_resizeBorderWidth;
}

void WindowManagerHelper::setDbClickMaximizeEnabled(bool enabled)
{
    Q_D(WindowManagerHelper);
    d->_dbClickMaximizeEnabled = enabled;
}

bool WindowManagerHelper::dbClickMaximizeEnabled() const
{
    Q_D(const WindowManagerHelper);
    return d->_dbClickMaximizeEnabled;
}

void WindowManagerHelper::setTitleHeight(int height)
{
#ifdef Q_OS_WIN
    Q_D(WindowManagerHelper);
    d->_titleBarHeight = height;
#endif
}

int WindowManagerHelper::titleHeight() const
{
    Q_D(const WindowManagerHelper);
    return d->_titleBarHeight;
}

void WindowManagerHelper::addDragControllingWidget(QWidget *widget)
{
    Q_D(WindowManagerHelper);
    d->_dragControllingWidgets.append(widget);
}

WindowManagerHelperPrivate::WindowManagerHelperPrivate(WindowManagerHelper *parent) : q_ptr(parent), _controlledWidget(nullptr), _resizable(true), _moveEnabled(true), _dbClickMaximizeEnabled(true), _titleBarHeight(-1)
{
    Q_Q(WindowManagerHelper);
}

WindowManagerHelperPrivate::~WindowManagerHelperPrivate()
{
}

void WindowManagerHelperPrivate::setControlledWidget(QWidget *widget)
{
    if (widget == nullptr || _controlledWidget == widget)
    {
        return;
    }
    _controlledWidget = widget;

    widget->setWindowFlags(widget->windowFlags() | Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint | Qt::WindowTitleHint);
    widget->setAttribute(Qt::WA_Hover);
    _resizeBorderWidth = 8; // 可缩放的边距

    _mousePressed = false; // 标记鼠标是否按下
    _mouseStyle = NORMAL;  // 鼠标样式
    _dragXPadding = 0;     // 拖动时记录x边距
    _dragYPadding = 0;     // 拖动时记录y边距
    widget->installEventFilter(this);
}

void WindowManagerHelperPrivate::setResizable(bool resizable)
{
    _resizable = resizable;
}

void WindowManagerHelperPrivate::setResizeBorderWidth(int width)
{
    _resizeBorderWidth = width;
}

void WindowManagerHelperPrivate::changeMouseStyle(const QPoint &mousePos)
{
    Q_Q(WindowManagerHelper);
    if (!_resizable || _controlledWidget->windowState().testFlag(Qt::WindowMaximized) || _controlledWidget->windowState().testFlag(Qt::WindowFullScreen))
    {
        _mouseStyle = NORMAL;
        _controlledWidget->setCursor(Qt::ArrowCursor);
        return;
    }
    int minimumHeight = _controlledWidget->minimumHeight();
    int minimumWidth = _controlledWidget->minimumWidth();
    bool widthCanResize = minimumWidth < _controlledWidget->maximumWidth();
    bool heightCanResize = minimumHeight < _controlledWidget->maximumHeight();

    int iPosX = mousePos.x();
    int iPosY = mousePos.y();

    int iWidth = _controlledWidget->width();
    int iHeight = _controlledWidget->height();

    _topLeftPos = _controlledWidget->geometry().topLeft();
    _bottomRightPos = _controlledWidget->geometry().bottomRight();

    if (widthCanResize && heightCanResize)
    {
        // 左下
        if (iPosX >= 0 && iPosX <= _resizeBorderWidth && iPosY <= iHeight && iPosY >= iHeight - _resizeBorderWidth)
        {
            _dragXPadding = iPosX;
            _dragYPadding = iHeight - iPosY;
            _controlledWidget->setCursor(Qt::SizeBDiagCursor);
            _mouseStyle = BOTTOMLEFT;
            return;
        }
        // 右下
        if (iPosX <= iWidth && iPosX >= iWidth - _resizeBorderWidth && iPosY <= iHeight && iPosY >= iHeight - _resizeBorderWidth)
        {
            _dragXPadding = iWidth - iPosX;
            _dragYPadding = iHeight - iPosY;
            _controlledWidget->setCursor(Qt::SizeFDiagCursor);
            _mouseStyle = BOTTOMRIGHT;
            return;
        }
        // 左上
        if (iPosX >= 0 && iPosX <= _resizeBorderWidth && iPosY >= 0 && iPosY <= _resizeBorderWidth)
        {
            _dragXPadding = iPosX;
            _dragYPadding = iPosY;
            _controlledWidget->setCursor(Qt::SizeFDiagCursor);
            _mouseStyle = TOPLEFT;
            return;
        }
        // 右上
        if (iPosX <= iWidth && iPosX >= iWidth - _resizeBorderWidth && iPosY >= 0 && iPosY <= _resizeBorderWidth)
        {
            _dragXPadding = iWidth - iPosX;
            _dragYPadding = iPosY;
            _controlledWidget->setCursor(Qt::SizeBDiagCursor);
            _mouseStyle = TOPRIGHT;
            return;
        }
    }
    if (widthCanResize)
    {
        // 左
        if (iPosX >= 0 && iPosX <= _resizeBorderWidth)
        {
            _dragXPadding = iPosX;
            _controlledWidget->setCursor(Qt::SizeHorCursor);
            _mouseStyle = LEFT;
            return;
        }
        // 右
        if (iPosX <= iWidth && iPosX >= iWidth - _resizeBorderWidth)
        {
            _dragXPadding = iWidth - iPosX;
            _controlledWidget->setCursor(Qt::SizeHorCursor);
            _mouseStyle = RIGHT;
            return;
        }
    }
    if (heightCanResize)
    {
        // 下
        if (iPosY <= iHeight && iPosY >= iHeight - _resizeBorderWidth)
        {
            _dragYPadding = iHeight - iPosY;
            _controlledWidget->setCursor(Qt::SizeVerCursor);
            _mouseStyle = BOTTOM;
            return;
        }
        // 上
        if (iPosY >= 0 && iPosY <= _resizeBorderWidth)
        {
            _dragYPadding = iPosY;
            _controlledWidget->setCursor(Qt::SizeVerCursor);
            _mouseStyle = TOP;
            return;
        }
    }
    _controlledWidget->setCursor(Qt::ArrowCursor);
    _mouseStyle = NORMAL;
}

bool WindowManagerHelperPrivate::isInTitleBar(const QPoint &p)
{
    if (_dragControllingWidgets.size() == 0 && _titleBarHeight < 0)
    {
        return true;
    }

    for (QWidget *controllingWidget : _dragControllingWidgets)
    {
        if (controllingWidget->rect().contains(controllingWidget->mapFrom(_controlledWidget, p)))
        {
            return true;
        }
    }

    return p.y() <= _titleBarHeight;
}

bool WindowManagerHelperPrivate::eventFilter(QObject *watched, QEvent *event)
{
    Q_Q(WindowManagerHelper);
    if (_controlledWidget == nullptr || watched != _controlledWidget || !_controlledWidget->isWindow())
    {
        return QObject::eventFilter(watched, event);
    }

    switch (event->type())
    {
    // [1] Hover移动 begin
    case QEvent::HoverMove: {
        QHoverEvent *pHovEvt = dynamic_cast<QHoverEvent *>(event);
        // 如果鼠标没有按下，则根据当前鼠标位置设置鼠标样式
        if (!_mousePressed)
        {
            changeMouseStyle(pHovEvt->pos());
        }
        else
        {
            QPoint globalMousePos = QCursor::pos();

            // 鼠标是正常样式时，拖动界面
            if (_mouseStyle == WindowManagerHelperPrivate::NORMAL)
            {
                if (!_moveEnabled)
                {
                    return QObject::eventFilter(watched, event);
                }

                auto screen = QApplication::screenAt(globalMousePos);
                QRect availGeo = screen ? screen->availableGeometry() : QApplication::primaryScreen()->availableGeometry();
                if (_controlledWidget->windowState().testFlag(Qt::WindowMaximized) || _controlledWidget->windowState().testFlag(Qt::WindowFullScreen))
                {
                    QRect normalGeo = _controlledWidget->normalGeometry();
                    _controlledWidget->showNormal();
                    _mousePressPos.setX(_mousePressPos.x() * 1.0 * normalGeo.width() / availGeo.width());
                    if (_mousePressPos.y() >= normalGeo.height())
                    {
                        _mousePressPos.setY(_mousePressPos.y() * 1.0 * normalGeo.height() / availGeo.height());
                    }
                }
                // FIXME: 逻辑有问题
                else
                {
                    //_controlledWidget->setGeometry(m_normalGeo);
                }
                globalMousePos.setX(qBound(availGeo.x(), globalMousePos.x(), availGeo.x() + availGeo.width()));
                globalMousePos.setY(qBound(availGeo.y(), globalMousePos.y(), availGeo.y() + availGeo.height()));
                _controlledWidget->move(globalMousePos - _mousePressPos);

                return QObject::eventFilter(watched, event);
            }
            // 否则，进行缩放
            if (!_resizable)
            {
                return QObject::eventFilter(watched, event);
            }

            int iMinimumHeight = _controlledWidget->minimumHeight();
            int iMinimumWidth = _controlledWidget->minimumWidth();
            switch (_mouseStyle)
            {
            case WindowManagerHelperPrivate::BOTTOM: {
                if (globalMousePos.y() - _topLeftPos.y() > iMinimumHeight)
                {
                    _bottomRightPos.setY(globalMousePos.y() + _dragYPadding);
                }
                else
                {
                    _bottomRightPos.setY(_topLeftPos.y() + iMinimumHeight);
                }
            }
            break;

            case WindowManagerHelperPrivate::RIGHT: {
                if (globalMousePos.x() - _topLeftPos.x() > iMinimumWidth)
                {
                    _bottomRightPos.setX(globalMousePos.x() + _dragXPadding);
                }
                else
                {
                    _bottomRightPos.setX(_topLeftPos.x() + iMinimumWidth);
                }
            }
            break;
            case WindowManagerHelperPrivate::BOTTOMRIGHT: {
                if (globalMousePos.x() - _topLeftPos.x() > iMinimumWidth)
                {
                    _bottomRightPos.setX(globalMousePos.x() + _dragXPadding);
                }
                else
                {
                    _bottomRightPos.setX(_topLeftPos.x() + iMinimumWidth);
                }
                if (globalMousePos.y() - _topLeftPos.y() > iMinimumHeight)
                {
                    _bottomRightPos.setY(globalMousePos.y() + _dragYPadding);
                }
                else
                {
                    _bottomRightPos.setY(_topLeftPos.y() + iMinimumHeight);
                }
            }
            break;

            case WindowManagerHelperPrivate::TOP: {
                if (_bottomRightPos.y() - globalMousePos.y() > iMinimumHeight)
                {
                    _topLeftPos.setY(globalMousePos.y() - _dragYPadding);
                }
                else
                {
                    _topLeftPos.setY(_bottomRightPos.y() - iMinimumHeight);
                }
            }
            break;

            case WindowManagerHelperPrivate::LEFT: {
                if (_bottomRightPos.x() - globalMousePos.x() > iMinimumWidth)
                {
                    _topLeftPos.setX(globalMousePos.x() - _dragXPadding);
                }
                else
                {
                    _topLeftPos.setX(_bottomRightPos.x() - iMinimumWidth);
                }
            }
            break;

            case WindowManagerHelperPrivate::TOPLEFT: {
                if (_bottomRightPos.y() - globalMousePos.y() > iMinimumHeight)
                {
                    _topLeftPos.setY(globalMousePos.y() - _dragYPadding);
                }
                else
                {
                    _topLeftPos.setY(_bottomRightPos.y() - iMinimumHeight);
                }
                if (_bottomRightPos.x() - globalMousePos.x() > iMinimumWidth)
                {
                    _topLeftPos.setX(globalMousePos.x() - _dragXPadding);
                }
                else
                {
                    _topLeftPos.setX(_bottomRightPos.x() - iMinimumWidth);
                }
            }
            break;

            case WindowManagerHelperPrivate::TOPRIGHT: {
                if (_bottomRightPos.y() - globalMousePos.y() > iMinimumHeight)
                {
                    _topLeftPos.setY(globalMousePos.y() - _dragYPadding);
                }
                else
                {
                    _topLeftPos.setY(_bottomRightPos.y() - iMinimumHeight);
                }
                if (globalMousePos.x() - _topLeftPos.x() > iMinimumWidth)
                {
                    _bottomRightPos.setX(globalMousePos.x() + _dragXPadding);
                }
                else
                {
                    _bottomRightPos.setX(_topLeftPos.x() + iMinimumWidth);
                }
            }
            break;

            case WindowManagerHelperPrivate::BOTTOMLEFT: {
                if (_bottomRightPos.x() - globalMousePos.x() > iMinimumWidth)
                {
                    _topLeftPos.setX(globalMousePos.x() - _dragXPadding);
                }
                else
                {
                    _topLeftPos.setX(_bottomRightPos.x() - iMinimumWidth);
                }
                if (globalMousePos.y() - _topLeftPos.y() > iMinimumHeight)
                {
                    _bottomRightPos.setY(globalMousePos.y() + _dragYPadding);
                }
                else
                {
                    _bottomRightPos.setY(_topLeftPos.y() + iMinimumHeight);
                }
            }
            break;

            default:
                break;
            }
            _controlledWidget->setGeometry(QRect(_topLeftPos, _bottomRightPos));
        }
    }
    break;
    // [1] 鼠标移动 end

    // [2] 鼠标按下 begin
    case QEvent::MouseButtonPress: {
        QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton)
        {
            if (_mouseStyle == WindowManagerHelperPrivate::NORMAL)
            {
                QPoint pressPt = mouseEvent->pos();
                // 处于标题栏或者其他可拖拽区域
                if (isInTitleBar(pressPt))
                {
                    _mousePressed = true;
                    _mousePressPos = mouseEvent->pos();
                }
            }
            else
            {
                _mousePressed = true;
                _mousePressPos = mouseEvent->pos();
            }
        }
    }
    break;
    // [2] 鼠标按下 end

    // [3] 鼠标松开 begin
    case QEvent::MouseButtonRelease: {
        _mousePressed = false;
        QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(event);
        changeMouseStyle(mouseEvent->pos());
    }
    break;
    // [3] 鼠标松开 end

    // [4] 鼠标双击 begin
    case QEvent::MouseButtonDblClick: {
        if (_resizable && _dbClickMaximizeEnabled)
        {
            QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(event);
            QPoint pressPt = mouseEvent->pos();
            // 鼠标双击标题栏，进行最大化和正常大小切换
            if (isInTitleBar(pressPt))
            {
                if (_controlledWidget->windowState().testFlag(Qt::WindowMaximized))
                {
                    _controlledWidget->showNormal();
                }
                else if (!_controlledWidget->windowState().testFlag(Qt::WindowFullScreen))
                {
                    _controlledWidget->showMaximized();
                }
            }
        }
    }
    break;
        // [4] 鼠标双击 end

    case QEvent::Leave:
    case QEvent::HoverLeave: {
        _mousePressed = false;
        _mouseStyle = NORMAL;
        _controlledWidget->setCursor(Qt::ArrowCursor);
    }
    break;

    case QEvent::WindowStateChange: {
        if (_controlledWidget->isMaximized())
        {
            QScreen *screen = _controlledWidget->windowHandle()->screen();
            if (screen && screen != QApplication::primaryScreen())
            {
                _controlledWidget->setGeometry(screen->availableGeometry());
            }
        }
        break;
    }

	default:
		break;
	}
	return QObject::eventFilter(watched, event);
}
