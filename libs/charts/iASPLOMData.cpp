/*************************************  open_iA  ************************************ *
* **********   A tool for visual analysis and processing of 3D CT images   ********** *
* *********************************************************************************** *
* Copyright (C) 2016-2021  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan, Ar. &  Al. *
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
#include "iASPLOMData.h"

const size_t iASPLOMData::NoDataIdx = std::numeric_limits<size_t>::max();

iASPLOMData::iASPLOMData()
{
}

void iASPLOMData::setParameterNames(std::vector<QString> const & names, size_t rowReserve)
{
	m_paramNames = names;
	m_dataPoints.clear();
	for (size_t i = 0; i < m_paramNames.size(); ++i)
	{
		std::vector<double> column;
		if (rowReserve > 0)
		{
			column.reserve(rowReserve);
		}
		m_dataPoints.push_back(column);
	}
}

void iASPLOMData::addParameter(QString& name)
{
	m_paramNames.push_back(name);
	m_ranges.push_back(std::vector<double>(2, 0));
	m_dataPoints.push_back(std::vector<double>(numPoints()));
}

std::vector<std::vector<double>> & iASPLOMData::data()
{
	return m_dataPoints;
}

std::vector<QString> & iASPLOMData::paramNames()
{
	return m_paramNames;
}

const std::vector<std::vector<double>> & iASPLOMData::data() const
{
	return m_dataPoints;
}

const std::vector<double> & iASPLOMData::paramData(size_t paramIndex) const
{
	return m_dataPoints[paramIndex];
}

QString iASPLOMData::parameterName(size_t paramIndex) const
{
	return m_paramNames[paramIndex];
}

size_t iASPLOMData::paramIndex(QString const & paramName) const
{
	for (unsigned long i = 0; i < numParams(); ++i)
	{
		if (m_paramNames[i] == paramName)
		{
			return i;
		}
	}
	return NoDataIdx;
}

size_t iASPLOMData::numParams() const
{
	return m_paramNames.size();
}

size_t iASPLOMData::numPoints() const
{
	return m_dataPoints.size() < 1 ? 0 : m_dataPoints[0].size();
}

double const* iASPLOMData::paramRange(size_t paramIndex) const
{
	return m_ranges[paramIndex].data();
}

void iASPLOMData::updateRanges()
{
	m_ranges.resize(m_dataPoints.size());
	for (size_t param = 0; param < m_dataPoints.size(); ++param)
	{
		updateRangeInternal(param);
	}
	for (size_t param = 0; param < m_dataPoints.size(); ++param)
	{
		emit dataChanged(param);
	}
}

void iASPLOMData::updateRanges(std::vector<size_t> paramIndices)
{
	for (size_t param : paramIndices)
	{
		updateRangeInternal(param);
	}
	for (size_t param : paramIndices)
	{
		emit dataChanged(param);
	}
}

void iASPLOMData::updateRange(size_t paramIndex)
{
	updateRangeInternal(paramIndex);
	emit dataChanged(paramIndex);
}

void iASPLOMData::updateRangeInternal(size_t paramIndex)
{
	if (paramIndex >= m_dataPoints.size())
	{
		return;
	}
	m_ranges[paramIndex].resize(2);
	m_ranges[paramIndex][0] = std::numeric_limits<double>::max();
	m_ranges[paramIndex][1] = std::numeric_limits<double>::lowest();
	for (size_t row = 0; row < m_dataPoints[paramIndex].size(); ++row)
	{
		double value = m_dataPoints[paramIndex][row];
		if (value < m_ranges[paramIndex][0])
		{
			m_ranges[paramIndex][0] = value;
		}
		if (value > m_ranges[paramIndex][1])
		{
			m_ranges[paramIndex][1] = value;
		}
	}
}
