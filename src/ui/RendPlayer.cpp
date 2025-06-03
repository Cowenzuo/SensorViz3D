#include "RendPlayer.h"

RendPlayer::RendPlayer(QWidget* parent)
	: QWidget(parent)
	, ui(new Ui::RendPlayerClass())
{
	ui->setupUi(this);
	connect(ui->hSliderTimestamp, &QSlider::valueChanged, this, &RendPlayer::timestampChanged);
	connect(ui->btnPlayer, &QPushButton::toggled, this, &RendPlayer::btnPlayerToggled);
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

void RendPlayer::btnPlayerToggled(bool checked)
{
	if (checked)
	{
		_timer->start();
	}
	else {
		_timer->stop();
	}
}

void RendPlayer::btnStopClicked()
{
	ui->hSliderTimestamp->setValue(0);
}

void RendPlayer::onTimeout()
{
	auto currentvalue =ui->hSliderTimestamp->value();
	if (currentvalue< ui->hSliderTimestamp->maximum())
	{
		ui->hSliderTimestamp->setValue(++currentvalue);
	}
	else
	{
		ui->btnPlayer->setChecked(false);
		_timer->stop();
	}

}
