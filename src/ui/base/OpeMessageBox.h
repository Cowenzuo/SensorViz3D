#ifndef OPE_MESSAGE_BOX_H
#define OPE_MESSAGE_BOX_H

#include "BaseDialog.h"

#include <QDialogButtonBox>
#include <QMessageBox>

class OpeMessageBoxPrivate;

/// 消息提示框，用法同QMessageBox
class OpeMessageBox : public BaseDialog
{
	Q_OBJECT

public:
	OpeMessageBox(QWidget* parent = nullptr);

	~OpeMessageBox();

	/// 关于
	/// \parent 父窗口
	/// \title 标题
	/// \text 文本内容
	static void about(QWidget* parent, const QString& title, const QString& text);

	/// 提示信息
	/// \parent 父窗口
	/// \title 标题
	/// \buttons 按钮
	/// \defaultButton 默认按钮
	static QMessageBox::StandardButton info(QWidget* parent, const QString& title, const QString& text, QMessageBox::StandardButtons buttons = QMessageBox::Ok, QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

	/// 警告
	/// \parent [in] 父窗口
	/// \title [in] 标题
	/// \buttons [in] 按钮
	/// \defaultButton [in] 默认按钮
	static QMessageBox::StandardButton warning(QWidget* parent, const QString& title, const QString& text, QMessageBox::StandardButtons buttons = QMessageBox::Ok, QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

	/// 错误
	/// \parent 父窗口
	/// \title 标题
	/// \buttons 按钮
	/// \defaultButton 默认按钮
	static QMessageBox::StandardButton error(QWidget* parent, const QString& title, const QString& text, QMessageBox::StandardButtons buttons = QMessageBox::Ok, QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

	/// 询问
	/// \parent [in] 父窗口
	/// \title [in] 标题
	/// \buttons [in] 按钮
	/// \defaultButton [in] 默认按钮
	static QMessageBox::StandardButton question(QWidget* parent, const QString& title, const QString& text, QMessageBox::StandardButtons buttons = QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
		QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

	void replaceTextLabel(QWidget* widget);

	void setText(const QString& text);

	void setTitle(const QString& title);

	void setCloseButtonEnabled(bool enabled);

	void setIcon(const QPixmap& pixmap);

	void setIcon(const QMessageBox::Icon icon);

	QPushButton* addButton(const QString& text, QMessageBox::ButtonRole role);

	QPushButton* addButton(QMessageBox::StandardButton button);

	void setDefaultButton(QMessageBox::StandardButton defaultButton);

	void setDefaultButton(QPushButton* button);

	QAbstractButton* clickedButton() const;

signals:
	/// 按钮点击发出信号。
	void buttonClicked(QAbstractButton* button);

private:
	Q_DECLARE_PRIVATE(OpeMessageBox)
		Q_DISABLE_COPY(OpeMessageBox)
		QScopedPointer<OpeMessageBoxPrivate> d_ptr;
};

class OpeMessageBoxPrivate : public QObject
{
	Q_OBJECT

public:
	OpeMessageBoxPrivate(OpeMessageBox*);
	~OpeMessageBoxPrivate();

	void replaceContent(QWidget* widget);

	QPushButton* addButton(QDialogButtonBox::StandardButton button);

	QPushButton* addButton(const QString& text, QMessageBox::ButtonRole role);

	void setIcon(const QMessageBox::Icon icon);

	void showMessageBox(const QMessageBox::Icon icon, const QString& title, const QString& text, QMessageBox::StandardButtons, QMessageBox::StandardButton);

	void showMessageBox(const QPixmap& pixmap, const QString& title, const QString& text, QMessageBox::StandardButtons, QMessageBox::StandardButton);

	void retranslateText();

private slots:
	void onButtonClicked(QAbstractButton*);

public:
	OpeMessageBox* q_ptr;

	QWidget* _headerWidget;
	QWidget* _centralWidget;
	QLabel* _titleLabel;
	QPushButton* _closeButton;

	QVBoxLayout* _bodyLayout;

	QLabel* _iconLabel;
	QWidget* _contentWidget;
	QLabel* _textLabel;
	QDialogButtonBox* _btnBox;
	QMessageBox::StandardButton _retButton;
	QAbstractButton* _clickedButton;
};

#endif // !OPE_MESSAGE_BOX_H
