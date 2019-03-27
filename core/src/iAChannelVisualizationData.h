/*************************************  open_iA  ************************************ *
* **********   A tool for visual analysis and processing of 3D CT images   ********** *
* *********************************************************************************** *
* Copyright (C) 2016-2019  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan, Ar. &  Al. *
*                          Amirkhanov, J. Weissenböck, B. Fröhler, M. Schiwarth       *
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

#include <QColor>

#include <vtkSmartPointer.h>

class QWidget;
class QString;

class vtkImageActor;
class vtkImageData;
class vtkImageMapToColors;
class vtkImageReslice;
class vtkPiecewiseFunction;
class vtkScalarsToColors;
class vtkTransform;
class vtkLookupTable;
class vtkMarchingContourFilter;
class vtkPolyDataMapper;
class vtkActor;

class iAChannelVisualizationData;



class open_iA_Core_API iAChannelVisualizationData
{
public:
	static const size_t Maximum3DChannels = 3;

	iAChannelVisualizationData();
	virtual ~iAChannelVisualizationData();

	virtual void Reset();

	void SetOpacity(double opacity);
	double GetOpacity() const;

	bool IsEnabled() const;
	void SetEnabled(bool enabled);

	bool Uses3D() const;
	void Set3D(bool enabled);

	void SetImage(vtkSmartPointer<vtkImageData> image);
	void SetColorTF(vtkScalarsToColors* cTF);
	void SetOpacityTF(vtkPiecewiseFunction* oTF);

	void SetName(QString name);
	QString GetName() const;

	// check if this can be somehow refactored (not needed for each kind of channel):
	// begin
	void SetColor(QColor const & col);
	QColor GetColor() const;

	bool IsSimilarityRenderingEnabled() const;
	void SetSimilarityRenderingEnabled(bool enabled);
	// end

	vtkSmartPointer<vtkImageData> GetImage();
	vtkPiecewiseFunction * GetOTF();
	vtkScalarsToColors* GetCTF();
private:
	bool enabled;
	double opacity;
	bool threeD;
	QColor color;
	bool similarityRenderingEnabled;
	vtkSmartPointer<vtkImageData>       image;
	vtkPiecewiseFunction*               piecewiseFunction;
	vtkScalarsToColors*                 colorTransferFunction;
	QString                             m_name;
};

void open_iA_Core_API ResetChannel(iAChannelVisualizationData* chData, vtkSmartPointer<vtkImageData> image, vtkScalarsToColors* ctf, vtkPiecewiseFunction* otf);
