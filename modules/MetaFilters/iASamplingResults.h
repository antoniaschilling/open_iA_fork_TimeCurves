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
#pragma once

#include "MetaFilters_export.h"

#include <iAAttributes.h>

#include <QSharedPointer>
#include <QString>
#include <QVector>

class iASingleResult;

class MetaFilters_API iASamplingResults
{
public:
	iASamplingResults(
		QSharedPointer<iAAttributes> attr,
		QString const & samplingMethod,
		QString const & path,
		QString const & executable,
		QString const & additionalArguments,
		QString const & name,
		int id
	);
	static QSharedPointer<iASamplingResults> load(QString const & metaFileName, int datasetID);
	bool store(QString const& rangeFileName, QString const& parameterSetFileName,
		QString const& derivedOutputFileName);
	int size() const;
	QSharedPointer<iASingleResult> get(int i) const;
	void addResult(QSharedPointer<iASingleResult> result);
	QVector<QSharedPointer<iASingleResult> > const & members() const;
	void setMembers(QVector<QSharedPointer<iASingleResult> > const& members);
	QSharedPointer<iAAttributes> attributes() const;
	QString name() const;
	QString fileName() const;
	QString path(int id) const;
	QString path() const;
	QString executable() const;
	QString additionalArguments() const;
	int id() const;
	bool storeAttributes(int type, QString const & fileName, bool id);
private:
	QSharedPointer<iAAttributes> m_attributes;
	QVector<QSharedPointer<iASingleResult> > m_results;
	QString m_name;           //!< name of this sampling
	QString m_parameterSetFile;//!<the name of the file containing the parameter sets
	QString m_derivedOutputFile;//!<the name of the file containing the derived outputs
	QString m_samplingMethod; //!< the name of the applied sampling method (Latin Hypercube, Random, ...)
	QString m_rangeFileName;  //!< the name of the file containing parameter ranges that were sampled
	QString m_path;           //!< base filename for the sampling results
	QString m_executable;     //!< executable used to create this sampling
	QString m_additionalArguments; //!< additional parameters passed to the executable
	int m_id;

	bool loadInternal(QString const & parameterSetFileName, QString const & derivedOutputFileName);
};

typedef QSharedPointer<iASamplingResults> SamplingResultPtr;
typedef QSharedPointer<QVector<SamplingResultPtr> > SamplingVectorPtr;
