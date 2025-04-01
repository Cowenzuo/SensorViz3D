#ifndef OPE_WINDOW_HELPER_H
#define OPE_WINDOW_HELPER_H

#include <QObject>
#include <QPoint>
#include <QScopedPointer>

class WindowManagerHelperPrivate;

/// 窗口管理辅助类，控制无边框窗口的拖动和缩放
class WindowManagerHelper : public QObject
{
    Q_OBJECT

  public:
    WindowManagerHelper(QWidget *parent);

    ~WindowManagerHelper();

    /// 设置要控制的widget
    void setControlledWidget(QWidget *widget);

    QWidget *controlledWidget() const;

    /// 设置是否允许拖动
    void setMoveEnabled(bool enabled);

    bool moveEnabled() const;

    /// 设置是否可缩放
    void setResizable(bool resizable);

    bool resizable() const;

    /// 设置可缩放区域的边框宽度
    /// \param width 边框宽度
    void setResizeBorderWidth(int width);

    int resizeBorderWidth() const;

    /// 设置是否允许双击最大化窗口
    void setDbClickMaximizeEnabled(bool enabled);

    bool dbClickMaximizeEnabled() const;

    /// 设置标题栏高度，窗口在该高度范围内才能拖动
    /// \param height 高度
    void setTitleHeight(int height);

    int titleHeight() const;

    void addDragControllingWidget(QWidget *widget);

  private:
    Q_DISABLE_COPY(WindowManagerHelper)
    Q_DECLARE_PRIVATE(WindowManagerHelper)
    QScopedPointer<WindowManagerHelperPrivate> d_ptr; /// d指针
};

class WindowManagerHelperPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(WindowManagerHelper)

    enum MouseStyle
    {
        NORMAL,
        LEFT,
        TOP,
        RIGHT,
        BOTTOM,
        TOPLEFT,
        TOPRIGHT,
        BOTTOMRIGHT,
        BOTTOMLEFT
    };

  public:
    WindowManagerHelperPrivate(WindowManagerHelper *parent);

    ~WindowManagerHelperPrivate() override;

    void setControlledWidget(QWidget *widget);

    void setResizable(bool resizable);

    void setResizeBorderWidth(int width);

    void changeMouseStyle(const QPoint &pos);

    bool isInTitleBar(const QPoint &p);

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

  public:
    WindowManagerHelper *q_ptr; // q指针
    QWidget *_controlledWidget; // 受控制的widget
    QList<QWidget *> _dragControllingWidgets;
    bool _resizable;              // 标记是否可缩放
    bool _moveEnabled;            // 是否可鼠标拖动窗口
    bool _dbClickMaximizeEnabled; // 是否允许双击最大化

    int _resizeBorderWidth; // 可缩放的边距
    int _titleBarHeight;    // 标题栏高度

    bool _mousePressed;     // 标记鼠标是否按下
    QPoint _mousePressPos;  // 标记鼠标按下的坐标
    MouseStyle _mouseStyle; // 鼠标样式
    int _dragXPadding;      // 拖动时记录x边距
    int _dragYPadding;      // 拖动时记录y边距
    QPoint _topLeftPos;     // 记录左上角位置
    QPoint _bottomRightPos; // 记录右下角位置
};
#endif // FX_WINDOW_HELPER_H
