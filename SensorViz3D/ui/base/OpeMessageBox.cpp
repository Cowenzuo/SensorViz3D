#include "OpeMessageBox.h"

#include <QApplication>
#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyle>

OpeMessageBoxPrivate::OpeMessageBoxPrivate(OpeMessageBox* q) : q_ptr(q), _retButton(QMessageBox::Cancel), _clickedButton(nullptr)
{
	q_ptr->setResizable(false);

	QWidget* bgWgt = new QWidget;
	bgWgt->setObjectName("dialog_bg");
	QVBoxLayout* bgLayout = new QVBoxLayout(q_ptr);
	bgLayout->setContentsMargins(0, 0, 0, 0);
	bgLayout->addWidget(bgWgt);

	_headerWidget = new QWidget;
	_headerWidget->setObjectName("dialog_header_widget");
	q_ptr->addDragControllingWidget(_headerWidget);
	// 图标
	_iconLabel = new QLabel;
	_iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	_iconLabel->setObjectName("msgbox_icon_label");
	_iconLabel->setFixedSize(24, 24);
	_iconLabel->setVisible(false);
	_titleLabel = new QLabel;
	_titleLabel->setIndent(0);
	_titleLabel->setObjectName("dialog_title_label");
	_closeButton = new QPushButton;
	//_closeButton->setFocusPolicy(Qt::NoFocus);
	_closeButton->setObjectName("dialog_close_button");

	_centralWidget = new QWidget;

	QHBoxLayout* headerLayout = new QHBoxLayout(_headerWidget);
	headerLayout->setContentsMargins(0, 0, 0, 0);
	headerLayout->setSpacing(4);
	headerLayout->addWidget(_iconLabel);
	headerLayout->addWidget(_titleLabel);
	headerLayout->addStretch();
	headerLayout->addWidget(_closeButton);

	QVBoxLayout* vlayout = new QVBoxLayout(bgWgt);
	vlayout->setContentsMargins(12, 8, 12, 16);
	vlayout->setSpacing(0);
	vlayout->addWidget(_headerWidget);
	vlayout->addWidget(_centralWidget);

	vlayout->setStretch(0, 0);
	vlayout->setStretch(1, 1);

	connect(_closeButton, &QPushButton::clicked, q_ptr, &OpeMessageBox::close);

	// 正文
	_textLabel = new QLabel;
	_textLabel->setOpenExternalLinks(true);
	_textLabel->setWordWrap(true);
	_textLabel->setAlignment(Qt::AlignCenter);
	_textLabel->setObjectName("msgbox_content");

	_contentWidget = nullptr;

	// 按钮组
	QHBoxLayout* btnBoxLayout = new QHBoxLayout;
	btnBoxLayout->setContentsMargins(0, 0, 0, 0);
	_btnBox = new QDialogButtonBox;
	_btnBox->setCenterButtons(false);
	_btnBox->layout()->setSpacing(8);
	_btnBox->layout()->setContentsMargins(0, 0, 0, 0);
	btnBoxLayout->addStretch();
	btnBoxLayout->addWidget(_btnBox);

	connect(_btnBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButtonClicked(QAbstractButton*)));

	_bodyLayout = new QVBoxLayout(_centralWidget);
	_bodyLayout->setContentsMargins(0, 0, 0, 0);
	_bodyLayout->setSpacing(0);

	_bodyLayout->addWidget(_textLabel);
	_bodyLayout->addLayout(btnBoxLayout);
}

OpeMessageBoxPrivate::~OpeMessageBoxPrivate()
{
}

void OpeMessageBoxPrivate::replaceContent(QWidget* widget)
{
	auto oldWidget = _textLabel ? _textLabel : _contentWidget;
	_bodyLayout->replaceWidget(oldWidget, widget);

	if (_contentWidget)
	{
		_contentWidget->hide();
		_contentWidget->deleteLater();
		_contentWidget = nullptr;
	}

	if (_textLabel)
	{
		_textLabel->hide();
		_textLabel->deleteLater();
		_textLabel = nullptr;
	}

	_contentWidget = widget;
}

void OpeMessageBoxPrivate::onButtonClicked(QAbstractButton* pButton)
{
	_retButton = (QMessageBox::StandardButton)_btnBox->standardButton(pButton);
	emit q_ptr->buttonClicked(pButton);

	q_ptr->done(_retButton);
	_clickedButton = pButton;
	q_ptr->close();
}

QPushButton* OpeMessageBoxPrivate::addButton(QDialogButtonBox::StandardButton button)
{
	return _btnBox->addButton(button);
}

QPushButton* OpeMessageBoxPrivate::addButton(const QString& text, QMessageBox::ButtonRole role)
{
	return _btnBox->addButton(text, (QDialogButtonBox::ButtonRole)role);
}

void OpeMessageBoxPrivate::setIcon(const QMessageBox::Icon icon)
{
	// 图标
	QPixmap pixmap;
	switch (icon)
	{
	case QMessageBox::NoIcon:
		break;
	case QMessageBox::Information: {
		pixmap = QPixmap(":/image/common/messagebox/success.png");
	}
								 break;
	case QMessageBox::Warning: {
		pixmap = QPixmap(":/image/common/messagebox/warning.png");
	}
							 break;
	case QMessageBox::Critical: {
		pixmap = QPixmap(":/image/common/messagebox/error.png");
	}
							  break;
	case QMessageBox::Question: {
		pixmap = QPixmap(":/image/common/messagebox/question.png");
	}
							  break;
	default:
		break;
	}
	if (pixmap.isNull())
	{
		_iconLabel->setVisible(false);
	}
	else
	{
		_iconLabel->setVisible(true);
		_iconLabel->setPixmap(pixmap.scaled(24, 24));
	}
}

void OpeMessageBoxPrivate::showMessageBox(const QMessageBox::Icon icon, const QString& title, const QString& strText, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
	// 标题
	q_ptr->setWindowTitle(title);
	_titleLabel->setText(title);

	// 图标
	setIcon(icon);

	// 文字
	_textLabel->setText(strText);

	// 添加按钮
	uint mask = QMessageBox::FirstButton;
	while (mask <= QMessageBox::LastButton)
	{
		uint sb = buttons & mask;
		mask <<= 1;
		if (!sb)
		{
			continue;
		}
		addButton((QDialogButtonBox::StandardButton)sb);
	}

	retranslateText();

	// 默认按钮
	if (defaultButton != QMessageBox::NoButton)
	{
		_btnBox->button((QDialogButtonBox::StandardButton)defaultButton)->setFocus();
	}

	q_ptr->adjustSize();
}

void OpeMessageBoxPrivate::showMessageBox(const QPixmap& pixmap, const QString& strTitle, const QString& strText, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
	// 标题
	q_ptr->setWindowTitle(strTitle);

	// 图标
	_iconLabel->setPixmap(pixmap);

	// 文字
	_textLabel->setText(strText);

	// 添加按钮
	uint mask = QMessageBox::FirstButton;
	while (mask <= QMessageBox::LastButton)
	{
		uint sb = buttons & mask;
		mask <<= 1;
		if (!sb)
		{
			continue;
		}
		addButton((QDialogButtonBox::StandardButton)sb);
	}

	// 将英文改为中文
	retranslateText();

	// 默认按钮
	if (defaultButton != QMessageBox::NoButton)
	{
		_btnBox->button((QDialogButtonBox::StandardButton)defaultButton)->setFocus();
	}
}

void OpeMessageBoxPrivate::retranslateText()
{
	QPushButton* okBtn = _btnBox->button(QDialogButtonBox::Ok);
	if (okBtn != nullptr)
	{
		okBtn->setText("确认");
	}

	QPushButton* pOpenBtn = _btnBox->button(QDialogButtonBox::Open);
	if (nullptr != pOpenBtn)
	{
		pOpenBtn->setText("打开");
	}

	QPushButton* saveBtn = _btnBox->button(QDialogButtonBox::Save);
	if (saveBtn != nullptr)
	{
		saveBtn->setText("保存");
	}

	QPushButton* cancelBtn = _btnBox->button(QDialogButtonBox::Cancel);
	if (nullptr != cancelBtn)
	{
		cancelBtn->setText("取消");
	}

	QPushButton* pCloseBtn = _btnBox->button(QDialogButtonBox::Close);
	if (nullptr != pCloseBtn)
	{
		pCloseBtn->setText("关闭");
	}

	QPushButton* pDiscardBtn = _btnBox->button(QDialogButtonBox::Discard);
	if (nullptr != pDiscardBtn)
	{
		pDiscardBtn->setText("不保存");
	}

	QPushButton* pApplyBtn = _btnBox->button(QDialogButtonBox::Apply);
	if (nullptr != pApplyBtn)
	{
		pApplyBtn->setText("应用");
	}

	QPushButton* pResetBtn = _btnBox->button(QDialogButtonBox::Reset);
	if (nullptr != pResetBtn)
	{
		pResetBtn->setText("重置");
	}

	QPushButton* pRestoreDefaultsBtn = _btnBox->button(QDialogButtonBox::RestoreDefaults);
	if (nullptr != pRestoreDefaultsBtn)
	{
		pRestoreDefaultsBtn->setText("恢复默认");
	}

	QPushButton* pHelpBtn = _btnBox->button(QDialogButtonBox::Help);
	if (nullptr != pHelpBtn)
	{
		pHelpBtn->setText("帮助");
	}

	QPushButton* pSaveAllBtn = _btnBox->button(QDialogButtonBox::SaveAll);
	if (nullptr != pSaveAllBtn)
	{
		pSaveAllBtn->setText("保存所有");
	}

	QPushButton* pYesBtn = _btnBox->button(QDialogButtonBox::Yes);
	if (pYesBtn != nullptr)
	{
		pYesBtn->setText("是");
	}

	QPushButton* pYesToAllBtn = _btnBox->button(QDialogButtonBox::YesToAll);
	if (nullptr != pYesToAllBtn)
	{
		pYesToAllBtn->setText("全部是");
	}

	QPushButton* pNoBtn = _btnBox->button(QDialogButtonBox::No);
	if (pNoBtn != nullptr)
	{
		pNoBtn->setText("否");
	}

	QPushButton* pNoToAll = _btnBox->button(QDialogButtonBox::NoToAll);
	if (nullptr != pNoToAll)
	{
		pNoToAll->setText("全部否");
	}

	QPushButton* pAbortBtn = _btnBox->button(QDialogButtonBox::Abort);
	if (nullptr != pAbortBtn)
	{
		pAbortBtn->setText("中断");
	}

	QPushButton* pRetryBtn = _btnBox->button(QDialogButtonBox::Retry);
	if (nullptr != pRetryBtn)
	{
		pRetryBtn->setText("重试");
	}

	QPushButton* pIgnoreBtn = _btnBox->button(QDialogButtonBox::Ignore);
	if (nullptr != pIgnoreBtn)
	{
		pIgnoreBtn->setText("忽略");
	}
}

OpeMessageBox::OpeMessageBox(QWidget* parent) : BaseDialog(parent), d_ptr(new OpeMessageBoxPrivate(this))
{
	setAttribute(Qt::WA_TranslucentBackground);
}

OpeMessageBox::~OpeMessageBox()
{
}

void OpeMessageBox::replaceTextLabel(QWidget* widget)
{
	d_ptr->replaceContent(widget);
}

void OpeMessageBox::setText(const QString& text)
{
	d_ptr->_textLabel->setText(text);
}

void OpeMessageBox::setTitle(const QString& title)
{
	d_ptr->_titleLabel->setText(title);
}

void OpeMessageBox::setCloseButtonEnabled(bool enabled)
{
	d_ptr->_closeButton->setVisible(enabled);
}

void OpeMessageBox::setIcon(const QPixmap& pixmap)
{
	if (!pixmap.isNull())
	{
		d_ptr->_iconLabel->setPixmap(pixmap);
		d_ptr->_iconLabel->setVisible(true);
	}
	else
	{
		d_ptr->_iconLabel->setVisible(false);
	}
}

void OpeMessageBox::setIcon(const QMessageBox::Icon icon)
{
	d_ptr->setIcon(icon);
}

QPushButton* OpeMessageBox::addButton(const QString& text, QMessageBox::ButtonRole role)
{
	return d_ptr->addButton(text, role);
}

QPushButton* OpeMessageBox::addButton(QMessageBox::StandardButton button)
{
	if (button == QMessageBox::NoButton)
	{
		return nullptr;
	}
	QPushButton* pushButton = d_ptr->addButton((QDialogButtonBox::StandardButton)button);
	// 将英文改为中文
	d_ptr->retranslateText();
	return pushButton;
}

void OpeMessageBox::setDefaultButton(QMessageBox::StandardButton defaultButton)
{
	// 默认按钮
	if (defaultButton != QMessageBox::NoButton)
	{
		d_ptr->_btnBox->button((QDialogButtonBox::StandardButton)defaultButton)->setFocus();
	}
}

void OpeMessageBox::setDefaultButton(QPushButton* button)
{
	if (button)
	{
		button->setFocus();
	}
}

QAbstractButton* OpeMessageBox::clickedButton() const
{
	return d_ptr->_clickedButton;
}

void OpeMessageBox::about(QWidget* parent, const QString& title, const QString& text)
{
	OpeMessageBox messageBox(parent);
	messageBox.d_ptr->showMessageBox(QMessageBox::NoIcon, title, text, QMessageBox::Ok, QMessageBox::Ok);
	messageBox.exec();
}

QMessageBox::StandardButton OpeMessageBox::info(QWidget* parent, const QString& title, const QString& text, QMessageBox::StandardButtons buttons /*= Ok*/, QMessageBox::StandardButton defaultButton /*= NoButton*/)
{
	OpeMessageBox messageBox(parent);
	messageBox.d_ptr->showMessageBox(QMessageBox::Information, title, text, buttons, defaultButton);
	messageBox.exec();
	return messageBox.d_ptr->_retButton;
}

QMessageBox::StandardButton OpeMessageBox::question(QWidget* parent, const QString& title, const QString& text, QMessageBox::StandardButtons buttons /*= StandardButtons(Yes | No)*/, QMessageBox::StandardButton defaultButton /*= NoButton*/)
{
	OpeMessageBox messageBox(parent);
	messageBox.d_ptr->showMessageBox(QMessageBox::Question, title, text, buttons, defaultButton);
	messageBox.exec();
	return messageBox.d_ptr->_retButton;
}

QMessageBox::StandardButton OpeMessageBox::warning(QWidget* parent, const QString& title, const QString& text, QMessageBox::StandardButtons buttons /*= Ok*/, QMessageBox::StandardButton defaultButton /*= NoButton*/)
{
	OpeMessageBox messageBox(parent);
	messageBox.d_ptr->showMessageBox(QMessageBox::Warning, title, text, buttons, defaultButton);
	messageBox.exec();
	return messageBox.d_ptr->_retButton;
}

QMessageBox::StandardButton OpeMessageBox::error(QWidget* parent, const QString& title, const QString& text, QMessageBox::StandardButtons buttons /*= Ok*/, QMessageBox::StandardButton defaultButton /*= NoButton*/)
{
	OpeMessageBox messageBox(parent);
	messageBox.d_ptr->showMessageBox(QMessageBox::Critical, title, text, buttons, defaultButton);
	messageBox.exec();
	return messageBox.d_ptr->_retButton;
}
