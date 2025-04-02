#pragma once

#include "app/ProjectData.h"

namespace RWMAT
{
	//ExtraData
	bool readMatFile(
		RawData& fp,
		const QString& filepath,
		const QStringList& sensorNames,
		const QStringList& sensorValid,
		double minValue,
		double maxValue,
		ResType type
	);
};

