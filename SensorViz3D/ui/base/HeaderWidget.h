#ifndef OPE_HEADER_WIDGET_H
#define OPE_HEADER_WIDGET_H

#include <QLabel>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class HeaderWidgetClass;
};
QT_END_NAMESPACE

/// 主窗口顶部widget
class HeaderWidget : public QWidget {
  Q_OBJECT

 public:
  HeaderWidget(QWidget *parent = nullptr);

  ~HeaderWidget() override;

  /// 设置标题是否可见
  void setTitleVisible(bool visible);

  /// 设置标题
  void setTitle(const QString &title);

  /// 设置菜单按钮是否可见
  void setMenuButtonVisible(bool visible);

  /// 设置菜单按钮是否选中
  void setMenuButtonChecked(bool checked);

  /// 获取菜单按钮触发时菜单弹出位置
  QPoint menuPopupPos() const;

  /// 处理窗口正常状态时的操作
  void handleWindowNormalState();

  /// 处理窗口最大化状态时的操作
  void handleWindowMaximizedState();

  bool hitTestCaption(const QPoint &pos);

 protected:
  bool eventFilter(QObject *watched, QEvent *event) override;

 private:
  // 初始化信号槽连接
  void initSignalSlotConns();

 signals:
  /// 最小化按钮点击信号
  void minBtnClicked();

  /// 最大化按钮点击信号
  void maxBtnClicked();

  /// 还原按钮点击信号
  void restoreBtnClicked();

  /// 关闭按钮点击信号
  void closeBtnClicked();

  /// 菜单按钮触发信号
  void menuButtonTriggered();

  /// 用户中心按钮点击信号
  void userCenterBtnClicked();

 private:
  Ui::HeaderWidgetClass *ui;
  QLabel *_seedPlanPopupLabel{};
};

#endif
