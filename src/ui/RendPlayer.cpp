#include "RendPlayer.h"

RendPlayer::RendPlayer(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::RendPlayerClass())
{
	ui->setupUi(this);
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
