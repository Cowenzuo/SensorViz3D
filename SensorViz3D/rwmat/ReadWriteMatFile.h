#pragma once

#include "app/ProjectData.h"

namespace RWMAT
{
	//FPData
	bool readMatFile(const QString& filepath, const QStringList& sensorNames, RawData& fp);

};

