#include "HeaderWidget.h"

#include <QMouseEvent>

#include "ui_HeaderWidget.h"

HeaderWidget::HeaderWidget(QWidget *parent) : QWidget(parent), ui(new Ui::HeaderWidgetClass()) {
  ui->setupUi(this);
  ui->restoreBtn->setVisible(false);
  ui->titleLbl->setVisible(true);
  setMenuButtonVisible(true);

  initSignalSlotConns();
}

HeaderWidget::~HeaderWidget() {
  delete ui;
}

void HeaderWidget::setTitleVisible(bool visible) {
  ui->titleLbl->setVisible(visible);
}

void HeaderWidget::setTitle(const QString &title) {
  ui->titleLbl->setText(title);
}

void HeaderWidget::setMenuButtonVisible(bool visible) {
  ui->menuBtn->setVisible(visible);
}

void HeaderWidget::setMenuButtonChecked(bool checked) {
  ui->menuBtn->setChecked(checked);
}

QPoint HeaderWidget::menuPopupPos() const {
  int offsetY = 0;
  QPoint pos = ui->menuBtn->mapToGlobal(QPoint(0, ui->menuBtn->height() + offsetY));
  return pos;
}

void HeaderWidget::handleWindowNormalState() {
  QMouseEvent mouseEvt(QEvent::Leave, ui->maxBtn->mapFromGlobal(QCursor::pos()), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
  qApp->sendEvent(ui->maxBtn, &mouseEvt);
  ui->maxBtn->setVisible(true);
  ui->restoreBtn->setVisible(false);
}

void HeaderWidget::handleWindowMaximizedState() {
  QMouseEvent mouseEvt(QEvent::Leave, ui->restoreBtn->mapFromGlobal(QCursor::pos()), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
  qApp->sendEvent(ui->restoreBtn, &mouseEvt);
  ui->maxBtn->setVisible(false);
  ui->restoreBtn->setVisible(true);
}

bool HeaderWidget::hitTestCaption(const QPoint &pos) {
  QWidget *clickedWidget = childAt(pos);
  if (!clickedWidget) {
    return true;
  }
  if (qobject_cast<QPushButton *>(clickedWidget)) {
    return false;
  }
  return true;
}
bool HeaderWidget::eventFilter(QObject *watched, QEvent *event) {
  switch (event->type()) {
    case QEvent::Enter:
      break;
    case QEvent::Leave:
      break;
    case QEvent::MouseMove:
      break;
    default:
      break;
  }
  return QWidget::eventFilter(watched, event);
}

void HeaderWidget::initSignalSlotConns() {
  connect(ui->minBtn, &QPushButton::clicked, this, &HeaderWidget::minBtnClicked);
  connect(ui->maxBtn, &QPushButton::clicked, this, &HeaderWidget::maxBtnClicked);
  connect(ui->restoreBtn, &QPushButton::clicked, this, &HeaderWidget::restoreBtnClicked);
  connect(ui->closeBtn, &QPushButton::clicked, this, &HeaderWidget::closeBtnClicked);
  connect(ui->menuBtn, &QPushButton::clicked, this, &HeaderWidget::menuButtonTriggered);
}
