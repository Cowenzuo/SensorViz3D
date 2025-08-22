#include "RendPlayer.h"

RendPlayer::RendPlayer(QWidget* parent)
	: QWidget(parent)
	, ui(new Ui::RendPlayerClass())
{
	ui->setupUi(this);
	connect(ui->hSliderTimestamp, &QSlider::valueChanged, this, &RendPlayer::timestampChanged);
	connect(ui->btnPlayer, &QPushButton::toggled, this, &RendPlayer::btnPlayerToggled);
	connect(ui->btnPause, &QPushButton::toggled, this, &RendPlayer::btnPauseToggled);
	connect(ui->btnStop, &QPushButton::clicked, this, &RendPlayer::btnStopClicked);

	_timer->setInterval(100);
	connect(_timer, &QTimer::timeout, this, &RendPlayer::onTimeout);
}

RendPlayer::~RendPlayer()
{
	delete ui;
}

void RendPlayer::setRange(int min, int max)
{
	ui->hSliderTimestamp->setRange(min, max);
	ui->hSliderTimestamp->setValue(0);

	ui->btnPlayer->setChecked(false);
}

int RendPlayer::getCurrentTimpstamp()
{
	return ui->hSliderTimestamp->value();
}

void RendPlayer::updateTimer()
{
	switch (_state)
	{
	case PlayerState::PLAYING:
		_timer->start();
		break;
	case PlayerState::PLAYING_PAUSE:
		_timer->stop();
		break;
	case PlayerState::PAUSE:
		_timer->stop();
		break;
	case PlayerState::JUST_STOP:
		_timer->stop();
		break;
	default:
		break;
	}
}

void RendPlayer::btnPlayerToggled(bool checked)
{
	if (checked)
	{
		if (_state == PlayerState::PAUSE)
		{
			_state = PlayerState::PLAYING_PAUSE;
		}
		else
		{
			_state = PlayerState::PLAYING;
		}
	}
	else {
		if (_state == PlayerState::PLAYING_PAUSE)
		{
			_state = PlayerState::PAUSE;
		}
		else
		{
			_state = PlayerState::JUST_STOP;
		}
	}
	updateTimer();
}

void RendPlayer::btnPauseToggled(bool checked)
{
	if (checked)
	{
		if (_state == PlayerState::PLAYING)
		{
			_state = PlayerState::PLAYING_PAUSE;
		}
		else
		{
			_state = PlayerState::PAUSE;
		}
	}
	else {
		if (_state == PlayerState::PLAYING_PAUSE)
		{
			_state = PlayerState::PLAYING;
		}
		else
		{
			_state = PlayerState::JUST_STOP;
		}
	}
	updateTimer();
}

void RendPlayer::btnStopClicked()
{
	ui->hSliderTimestamp->setValue(0);
	ui->btnPlayer->setChecked(false);
	ui->btnPause->setChecked(false);
}

void RendPlayer::onTimeout()
{
	auto currentvalue = ui->hSliderTimestamp->value();
	if (currentvalue < ui->hSliderTimestamp->maximum())
	{
		ui->hSliderTimestamp->setValue(++currentvalue);
	}
	else
	{
		ui->btnPlayer->setChecked(false);
		_timer->stop();
	}

}
