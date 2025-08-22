#pragma once

#include <QWidget>
#include <QTimer>
#include "ui_RendPlayer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class RendPlayerClass; };
QT_END_NAMESPACE
enum class PlayerState { 
	PLAYING,
	PLAYING_PAUSE,
	PAUSE,
	JUST_STOP,
};

class RendPlayer : public QWidget
{
	Q_OBJECT
public:
	RendPlayer(QWidget* parent = nullptr);
	~RendPlayer();

	void setRange(int min, int max);

	int getCurrentTimpstamp();

	void updateTimer();

	Q_SLOT void btnPlayerToggled(bool checked);
	Q_SLOT void btnPauseToggled(bool checked);
	Q_SLOT void btnStopClicked();
	Q_SLOT void onTimeout();

	Q_SIGNAL void timestampChanged(int index);

private:
	Ui::RendPlayerClass* ui;
	QTimer* _timer{ new QTimer };
	PlayerState _state{ PlayerState::JUST_STOP };
};
