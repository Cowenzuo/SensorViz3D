#pragma once

#include <QWidget>
#include "ui_RendPlayer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class RendPlayerClass; };
QT_END_NAMESPACE

class RendPlayer : public QWidget
{
	Q_OBJECT

public:
	RendPlayer(QWidget *parent = nullptr);
	~RendPlayer();

	void setRange(int min, int max);

private:
	Ui::RendPlayerClass *ui;
};
