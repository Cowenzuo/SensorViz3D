#include "RendPlayer.h"

RendPlayer::RendPlayer(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::RendPlayerClass())
{
	ui->setupUi(this);
	connect(ui->hSliderTimestamp, &QSlider::valueChanged, this, &RendPlayer::timestampChanged);
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
