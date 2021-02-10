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

#include "ui_PCView.h"

#include <iAVtkWidgetFwd.h>
#include <qthelper/iAQTtoUIConnector.h>

#include <vtkSmartPointer.h>

typedef iAQTtoUIConnector<QDockWidget, Ui_PCView> PCViewConnector;

class vtkContextView;
class vtkChartParallelCoordinates;

class QTableWidget;

class iAPCView : public PCViewConnector
{
	Q_OBJECT

public:
	iAPCView( QWidget * parent = 0, Qt::WindowFlags f = 0 );
	~iAPCView();

public slots:
	void SetData( const QTableWidget * data );

protected:
	void ChartModified();

protected:
	vtkSmartPointer<vtkContextView> m_view;
	vtkSmartPointer<vtkChartParallelCoordinates> m_chart;
	iAVtkOldWidget * m_widget;
};
