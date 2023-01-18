/*************************************  open_iA  ************************************ *
* **********   A tool for visual analysis and processing of 3D CT images   ********** *
* *********************************************************************************** *
* Copyright (C) 2016-2022  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan, Ar. &  Al. *
*                 Amirkhanov, J. Weissenböck, B. Fröhler, M. Schiwarth, P. Weinberger *
* *********************************************************************************** *
* This program is free software: you can redistribute it and/or modify it under the   *
* terms of the GNU General Public License as published by the Free Software           *
* Foundation, either version 3 of the License, or (at your option) any later version. *
*                                                                                     *
* This program is distributed in the hope that it will be useful, but WITHOUT ANY     *
* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A     *
* PARTICULAR PURPOSE.  See the GNU General Public License for more details.           *
*                                                                                     *
* You should have received a copy of the GNU General Public License along with this   *
* program.  If not, see http://www.gnu.org/licenses/                                  *
* *********************************************************************************** *
* Contact: FH OÖ Forschungs & Entwicklungs GmbH, Campus Wels, CT-Gruppe,              *
*          Stelzhamerstraße 23, 4600 Wels / Austria, Email: c.heinzl@fh-wels.at       *
* ************************************************************************************/
#include "iAParsFileIO.h"

#include "iAFileUtils.h"    // for tryFixFileName
#include "iAToolsVTK.h"     // for ByteOrder

#include "iARawFileIO.h"
#include "iASettingsFileHelper.h"  // for readSettingsFile

#include <iALog.h>
#include <iAStringHelper.h>

#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QTextStream>

namespace
{
	// split string value into a given generic type of number of numeric values (throwing exception on any error)
	template <typename T>
	QVector<T> getNumFromSettings(QMap<QString, QString> const& settings, QString const & key, int numVals)
	{
		auto valList = settings[key].split(" ");
		if (valList.size() != numVals)
		{
			throw std::runtime_error(
				QString("Pars file: Invalid value for key '%1': should have %2 numbers but has %3 (value: %4)")
				.arg(key).arg(numVals).arg(valList.size()).arg(settings[key]).toStdString());
		}
		QVector<T> result;
		for (auto s: valList)
		{
			bool ok;
			result.push_back(iAConverter<T>::toT(s, &ok));
			if (!ok)
			{
				throw std::runtime_error(
					QString("Pars file: Invalid value for key '%1'; should have contained numbers but part '%2' contains something invalid (value: %3)!")
					.arg(key).arg(s).arg(settings[key]).toStdString());
			}
		}
		return result;
	}
}

iAParsFileIO::iAParsFileIO() : iAFileIO(iADataSetType::Volume, iADataSetType::None)
{}

std::shared_ptr<iADataSet> iAParsFileIO::loadData(QString const& fileName, QVariantMap const& paramValues, iAProgress const& progress)
{
	QVariantMap rawFileParams;
	Q_UNUSED(paramValues);
	auto fileSettings = readSettingsFile(fileName);
	auto detsize = getNumFromSettings<int>(fileSettings, "det_size", 2);
	auto reco_n_proj = getNumFromSettings<int>(fileSettings, "reco_n_proj", 1);
	rawFileParams[iARawFileIO::SizeStr] = QVariant::fromValue(QVector<int>{ detsize[0], detsize[1], reco_n_proj[0] });
	auto detpitch = getNumFromSettings<double>(fileSettings, "det_pitch", 2);
	auto SD = getNumFromSettings<double>(fileSettings, "geo_SD", 1)[0];
	auto SO = getNumFromSettings<double>(fileSettings, "geo_SO", 1)[0];
	auto magn = SD / SO;

	QVector<double> spacing{ detpitch[0] / magn, detpitch[1] / magn };
	spacing.push_back(std::min(spacing[0], spacing[1]));
	rawFileParams[iARawFileIO::SpacingStr] = QVariant::fromValue(spacing);

	auto proj_datatype = fileSettings["proj_datatype"];
	rawFileParams[iARawFileIO::DataTypeStr] = mapVTKTypeToReadableDataType(
		proj_datatype == "intensity" ? VTK_UNSIGNED_SHORT : VTK_FLOAT);

	// default values for things not configurable in pars file:
	rawFileParams[iARawFileIO::HeadersizeStr] = 0;
	rawFileParams[iARawFileIO::ByteOrderStr] = ByteOrder::LittleEndianStr;
	rawFileParams[iARawFileIO::OriginStr] = QVariant::fromValue(QVector<double>{0.0, 0.0, 0.0});

	auto rawFileName = tryFixFileName(fileSettings["proj_filename_template_1"], QFileInfo(fileName).canonicalPath());
	iARawFileIO io;
	return io.load(rawFileName, rawFileParams, progress);
}

QString iAParsFileIO::name() const
{
	return "PARS file";
}

QStringList iAParsFileIO::extensions() const
{
	return QStringList{ "pars" };
}
