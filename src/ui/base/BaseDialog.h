#pragma once

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

class WindowManagerHelper;

/// 对话框顶部widget
class BaseDialogHeaderWidget : public QWidget
{
    Q_OBJECT

  public:
    BaseDialogHeaderWidget(QWidget *parent = nullptr);

    ~BaseDialogHeaderWidget();

    void setCloseButtonVisible(bool visible);

    Q_SLOT void setTitle(const QString &title);

    Q_SIGNAL void closeButtonClicked();

  private:
    QLabel *_titleLabel;
    QPushButton *_closeButton;
};

/// 基础对话框基类
class BaseDialog : public QDialog
{
    Q_OBJECT

  public:
    BaseDialog(QWidget *parent = nullptr);

    ~BaseDialog();

    void setResizable(bool resizable);

    void setDbClickMaximizeEnabled(bool enabled);

    /// 绑定顶部标题栏widget，只允许在该区域拖动对话框
    /// \param headerWidget 顶部widget
    void bindHeaderWidget(BaseDialogHeaderWidget *headerWidget);

    void addDragControllingWidget(QWidget *widget);

  protected:
    void closeEvent(QCloseEvent *event) override;

    void paintEvent(QPaintEvent *event) override;

  private:
    WindowManagerHelper *_helper;
    QVBoxLayout *_layout;
};

