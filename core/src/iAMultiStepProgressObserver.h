/*************************************  open_iA  ************************************ *
* **********   A tool for visual analysis and processing of 3D CT images   ********** *
* *********************************************************************************** *
* Copyright (C) 2016-2020  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan, Ar. &  Al. *
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

#include "open_iA_Core_export.h"

#include "iAProgress.h"

#include <vtkCommand.h>

#include <QObject>

//! Enables Observing the progress of multiple vtk algorithms executed sequentially via signals.
// TODO: should be merged/consolidated with iAProgress!
class open_iA_Core_API iAMultiStepProgressObserver : public QObject, public vtkCommand
{
	Q_OBJECT
public:
	iAMultiStepProgressObserver(double overallSteps);
	void setCompletedSteps(int steps);
	void observe(vtkAlgorithm* caller);
	iAProgress* progressObject();
private:
	virtual void Execute(vtkObject *caller, unsigned long, void*);
	double m_currentStep;
	double m_overallSteps;
	iAProgress m_progress;
};