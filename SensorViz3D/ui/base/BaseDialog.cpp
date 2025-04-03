#include "BaseDialog.h"

#include <QCloseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>

#include "WindowManagerHelper.h"

BaseDialogHeaderWidget::BaseDialogHeaderWidget(QWidget* parent) : QWidget(parent)
{
	setAttribute(Qt::WA_StyledBackground);
	this->setObjectName("base_dialog_header");
	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(12, 12, 12, 12);
	layout->setSpacing(0);
	_titleLabel = new QLabel;
	_titleLabel->setObjectName("base_dialog_title");
	_closeButton = new QPushButton;
	_closeButton->setObjectName("base_dialog_close");
	_closeButton->setFocusPolicy(Qt::NoFocus);
	layout->addWidget(_titleLabel);
	layout->addStretch();
	layout->addWidget(_closeButton);
	connect(_closeButton, &QPushButton::clicked, this, &BaseDialogHeaderWidget::closeButtonClicked);
}

BaseDialogHeaderWidget::~BaseDialogHeaderWidget()
{
}

void BaseDialogHeaderWidget::setCloseButtonVisible(bool visible)
{
	_closeButton->setVisible(visible);
}

void BaseDialogHeaderWidget::setTitle(const QString& title)
{
	_titleLabel->setText(title);
}

BaseDialog::BaseDialog(QWidget* parent) : QDialog(parent)
{
	_helper = new WindowManagerHelper(this);
	_helper->setControlledWidget(this);
	_helper->setDbClickMaximizeEnabled(false);
	// setAttribute(Qt::WA_StyledBackground);
	setAttribute(Qt::WA_TranslucentBackground);
}

BaseDialog::~BaseDialog()
{
}

void BaseDialog::setResizable(bool resizable)
{
	_helper->setResizable(resizable);
}

void BaseDialog::setDbClickMaximizeEnabled(bool enabled)
{
	_helper->setDbClickMaximizeEnabled(enabled);
}

void BaseDialog::bindHeaderWidget(BaseDialogHeaderWidget* headerWidget)
{
	headerWidget->setTitle(this->windowTitle());
	_helper->addDragControllingWidget(headerWidget);
	connect(headerWidget, &BaseDialogHeaderWidget::closeButtonClicked, this, &BaseDialog::close);
	connect(this, &BaseDialog::windowTitleChanged, headerWidget, &BaseDialogHeaderWidget::setTitle);
}

void BaseDialog::addDragControllingWidget(QWidget* widget)
{
	_helper->addDragControllingWidget(widget);
}

void BaseDialog::closeEvent(QCloseEvent* event)
{
	// WARNING：不要对BaseDialog设置属性Qt::WA_DeleteOnClose
	event->ignore();
	reject();
}

void BaseDialog::paintEvent(QPaintEvent* event)
{
	QDialog::paintEvent(event);

	QPainter painter(this);
	QStyleOption opt;
	opt.initFrom(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}
