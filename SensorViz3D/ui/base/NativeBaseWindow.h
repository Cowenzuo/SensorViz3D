#ifndef OPE_NATIVE_BASE_WINDOW_H
#define OPE_NATIVE_BASE_WINDOW_H

#include <QMainWindow>

/// 使用系统原生API的窗口基类
class NativeBaseWindow : public QMainWindow {
  Q_OBJECT

 public:
  NativeBaseWindow(QWidget* parent = nullptr);

  ~NativeBaseWindow();

 protected:
  bool nativeEvent(const QByteArray& eventType, void* message, long* result) override;

  virtual bool hitTestCaption(const QPoint& pos);

 private slots:
  void onScreenChanged(QScreen* screen);

 private:
  bool _resizable;
  int _resizeBorderWidth;
};

#endif
