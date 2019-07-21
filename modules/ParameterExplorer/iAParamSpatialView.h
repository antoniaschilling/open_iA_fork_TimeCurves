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

#include <io/iAITKIO.h>

#include <vtkSmartPointer.h>

#include <QMap>
#include <QWidget>

class iADiagramFctWidget;
class iAParamTableView;
class iAHistogramCreator;
class iAHistogramData;
class iAImageWidget;
class iAPlot;

class vtkImageData;

class QSpinBox;
class QToolButton;

class iAParamSpatialView: public QWidget
{
	Q_OBJECT
public:
	iAParamSpatialView(iAParamTableView* table, QString const & basePath, iADiagramFctWidget* chartWidget, int binCount);
	void setImage(size_t id);
	void ToggleSettings(bool visible);
private slots:
	void SlicerModeButtonClicked(bool checked);
	void SliceChanged(int slice);
	void HistogramReady();
private:
	void SwitchToHistogram(int id);
	iAParamTableView* m_table;
	QString m_basePath;
	QMap<size_t, vtkSmartPointer<vtkImageData>> m_imageCache;
	QVector<iAITKIO::ImagePointer> m_loadedImgs; // to stop itk from unloading
	int m_curMode;
	int m_sliceNr[3];
	QVector<QToolButton*> slicerModeButton;
	QSpinBox* m_sliceControl;
	iAImageWidget* m_imageWidget;
	QWidget* m_settings;
	QWidget* m_imageContainer;
	bool m_sliceNrInitialized;
	iADiagramFctWidget* m_chartWidget;
	QSharedPointer<iAPlot> m_curHistogramPlot;
	QVector<QSharedPointer<iAHistogramCreator> > m_histogramCreaters;
	QMap<int, QSharedPointer<iAHistogramData>> m_histogramCache;
	int m_binCount;
};
