#pragma once

#include "app/ProjectData.h"

namespace RWMAT
{
	//ExtraData
	bool readMatFile(
		const QString& filepath,
		const QStringList& sensorNames,
		const QStringList& sensorValid,
		int singleDataCols,
		RawData& fp);

};

