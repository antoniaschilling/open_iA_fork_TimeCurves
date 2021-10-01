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
#include "iASensitivityInfo.h"

// charts
#include <iAChartWithFunctionsWidget.h>	// only for calling update() on mdichild's histogram() !
#include <iASPLOMData.h>
#include <iAScatterPlotWidget.h>
#include <iAScatterPlotViewData.h>

// base:
#include <iAFileUtils.h>
#include <iAColorTheme.h>
#include <iAJobListView.h>
#include <iALog.h>
#include <iALUT.h>
#include <iAMathUtility.h>
#include <iAQVTKWidget.h>
#include <iARunAsync.h>
#include <iAStackedBarChart.h>    // for add HeaderLabel
#include <iAStringHelper.h>
#include <iAToolsVTK.h>
#include <iAVec3.h>

// guibase:
#include <iAMdiChild.h>
#include <iAModality.h>
#include <iAModalityList.h>
#include <iAModalityTransfer.h>
#include <qthelper/iAQTtoUIConnector.h>
#include <qthelper/iAWidgetSettingsMapper.h>

// qthelper:
#include <iADockWidgetWrapper.h>

// objectvis:
#include <iA3DColoredPolyObjectVis.h>

// FeatureScout
#include "iACsvVectorTableCreator.h"

// FIAKER
#include "iAAlgorithmInfo.h"
#include "iAFiberResult.h"
#include "iAFiberResultUIData.h"
#include "iAFiberData.h"
#include "iAMeasureSelectionDlg.h"
#include "iAMultidimensionalScaling.h"
#include "iAParameterInfluenceView.h"
#include "iACharacteristicsMeasureDlg.h"
#include "ui_DissimilarityMatrix.h"
#include "ui_SensitivitySettings.h"

#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkTable.h>

// for mesh differences:
#include "iARendererViewSync.h"
#include <vtkActor.h>
#include <vtkCornerAnnotation.h>
#include <vtkRendererCollection.h>
#include <vtkTextProperty.h>

// for sampled points display:
#include <vtkVertexGlyphFilter.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPointData.h>
#include <vtkProperty.h>

#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QRadioButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QSplitter>
#include <QTableView>
#include <QtConcurrent>
#include <QTextStream>
#include <QVBoxLayout>

#include <array>
#include <set>

namespace
{
	const int LayoutSpacing = 4;
	const QString DefaultStackedBarColorTheme("Brewer Accent (max. 8)");
	QStringList const& AggregationNames()
	{
		static QStringList Names = QStringList() << "Mean left+right" << "Left only" << "Right only" << "Mean of all neighbours in STAR";
		return Names;
	}

	QColor ParamColor(150, 150, 255, 255);
	QColor OutputColor(255, 200, 200, 255);
	QColor ScatterPlotPointColor(80, 80, 80, 128);

	const int DefaultAggregationMeasureIdx = 3;
	/*
	void logMeshSize(QString const& name, vtkSmartPointer<vtkPolyData> mesh)
	{
		const double* b1 = mesh->GetBounds();
		LOG(lvlDebug, QString(name + ": %1, %2, %3, %4 (mesh: %5 cells, %6 points)")
				.arg(b1[0]).arg(b1[1]).arg(b1[2]).arg(b1[3])
				.arg(mesh->GetNumberOfCells())
				.arg(mesh->GetNumberOfPoints()));
	}
	*/
}

const QString iASensitivityInfo::DefaultResultColorMap("Brewer Set2 (max. 8)");

// Factor out as generic CSV reading class also used by iACsvIO?
bool readParameterCSV(QString const& fileName, QString const& encoding, QString const& columnSeparator,
	iACsvTableCreator& tblCreator, size_t resultCount, int skipColumns)
{
	LOG(lvlDebug, QString("Reading file %1, skip %2 columns").arg(fileName).arg(skipColumns));
	if (!QFile::exists(fileName))
	{
		LOG(lvlError, "Error loading csv file, file does not exist.");
		return false;
	}
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		LOG(lvlError, QString("Unable to open file '%1': %2").arg(fileName).arg(file.errorString()));
		return false;
	}
	QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(5, 99, 0)
	in.setCodec(encoding.toStdString().c_str());
#else
	auto encOpt = QStringConverter::encodingForName(encoding.toStdString().c_str());
	QStringConverter::Encoding enc = encOpt.has_value() ? encOpt.value() : QStringConverter::Utf8;
	in.setEncoding(enc);
#endif
	auto headers = in.readLine().split(columnSeparator);
	for (int i = 0; i < skipColumns; ++i)
	{
		headers.removeAt(headers.size() - 1);
	}
	tblCreator.initialize(headers, resultCount);
	size_t row = 0;
	while (!in.atEnd() && row < resultCount)
	{
		QString line = in.readLine();
		if (line.trimmed().isEmpty()) // skip empty lines
		{
			continue;
		}
		tblCreator.addRow(row,
			stringToVector<std::vector<double>, double>(line, columnSeparator, headers.size(), false));
		++row;
	}
	// check for extra content at end - but skip empty lines:
	while (!in.atEnd())
	{
		QString line = in.readLine();
		if (!line.trimmed().isEmpty())
		{
			LOG(lvlError,
				QString("Line %1: Expected no more lines, or only empty ones, but got line '%2' instead!")
					.arg(row)
					.arg(line));
			return false;
		}
		++row;
	}
	return true;
}

void iASensitivityInfo::abort()
{
	m_data->abort();
	m_aborted = true;
}

QSharedPointer<iASensitivityInfo> iASensitivityInfo::create(iAMdiChild* child,
	QSharedPointer<iAFiberResultsCollection> data, QDockWidget* nextToDW, int histogramBins, int skipColumns,
	std::vector<iAFiberResultUIData> const& resultUIs, vtkRenderWindow* main3DWin, QString parameterSetFileName,
	QVector<int> const & charSelected, QVector<int> const & charDiffMeasure, iASettings const & projectFile)
{
	if (parameterSetFileName.isEmpty())
	{
		parameterSetFileName = QFileDialog::getOpenFileName(child,
			"Sensitivity: Parameter Sets file", data->folder,
			"Comma-Separated Values (*.csv);;All files (*)");
	}
	if (parameterSetFileName.isEmpty())
	{
		LOG(lvlInfo, "Empty parameter set filename / aborted.");
		return QSharedPointer<iASensitivityInfo>();
	}
	iACsvVectorTableCreator tblCreator;
	if (!readParameterCSV(parameterSetFileName, "UTF-8", ",", tblCreator, data->result.size(), skipColumns))
	{
		return QSharedPointer<iASensitivityInfo>();
	}
	auto const & paramValues = tblCreator.table();
	auto const& paramNames = tblCreator.header();
	// csv assumed to contain header line (names of parameters), and one row per parameter set;
	// parameter set contains an ID as first column and a filename as last row
	if (paramValues.size() <= 1 || paramValues[0].size() <= 3)
	{
		LOG(lvlError, QString("Invalid parameter set file: expected at least 2 data rows (actual: %1) "
			"and at least 2 columns (ID and one parameter; actual: %2")
			.arg(paramValues.size() > 0 ? paramValues[0].size() : -1)
			.arg(paramValues.size())
		);
		return QSharedPointer<iASensitivityInfo>();
	}
	// data in m_paramValues is indexed [col(=parameter index)][row(=parameter set index)]
	QSharedPointer<iASensitivityInfo> sens(
		new iASensitivityInfo(data, parameterSetFileName, skipColumns, paramNames, paramValues, child, nextToDW, resultUIs, main3DWin));

	// find min/max, for all columns
	sens->data().m_paramMin.resize(static_cast<int>(paramValues.size()));
	sens->data().m_paramMax.resize(static_cast<int>(paramValues.size()));
	//LOG(lvlInfo, QString("Parameter values size: %1x%2").arg(paramValues.size()).arg(paramValues[0].size()));
	for (size_t p = 0; p < paramValues.size(); ++p)
	{
		sens->data().m_paramMin[p] = *std::min_element(paramValues[p].begin(), paramValues[p].end());
		sens->data().m_paramMax[p] = *std::max_element(paramValues[p].begin(), paramValues[p].end());
	}

	// countOfVariedParams = number of parameters for which min != max (except column 0, which is the ID):
	for (size_t p = 1; p < sens->data().m_paramMin.size(); ++p)
	{
		if (sens->data().m_paramMin[p] != sens->data().m_paramMax[p])
		{
			sens->data().m_variedParams.push_back(static_cast<int>(p));
		}
	}
	if (sens->data().m_variedParams.size() == 0)
	{
		LOG(lvlError, "Invalid sampling: No parameter was varied!");
		return QSharedPointer<iASensitivityInfo>();
	}

	//LOG(lvlInfo, QString("Found the following parameters to vary (number: %1): %2")
	//	.arg(sensitivity->m_variedParams.size())
	//	.arg(joinAsString(sensitivity->m_variedParams, ",", [&paramNames](int const& i) { return paramNames[i]; })));

	// find out how many additional parameter sets were added per STAR:
	//   - go to first value row; take value of first varied parameter as v
	//   - go down rows, as long as either
	//        first varied parameter has same value as v
	//        or distance of current value of first varied parameter is a multiple
	//        of the distance between its first row value and second row value
	double checkValue0 = paramValues[sens->data().m_variedParams[0]][0];
	const double RemainderCheckEpsilon = 1e-12;
	double curCheckValue = paramValues[sens->data().m_variedParams[0]][1];
	double diffCheck = std::abs(curCheckValue - checkValue0);
	//LOG(lvlDebug, QString("checkValue0=%1, curCheckValue=%2, diffCheck=%3").arg(checkValue0).arg(curCheckValue).arg(diffCheck));
	double remainder = 0;
	size_t row = 2;
	// first, continue, as long as first varied parameter is a multiple of diffCheck away from checkValue0:
	while (row < paramValues[sens->data().m_variedParams[0]].size() &&
		!dblApproxEqual(curCheckValue, checkValue0) &&
		(remainder < RemainderCheckEpsilon || 	// "approximately a multiple" is not so easy with double
		(std::abs(diffCheck - remainder) < RemainderCheckEpsilon))) // remainder could also be close to but smaller than diffCheck
	{
		curCheckValue = paramValues[sens->data().m_variedParams[0]][row];
		remainder = std::abs(std::fmod(std::abs(curCheckValue - checkValue0), diffCheck));
		//LOG(lvlDebug, QString("Row %1: curCheckValue=%2, remainder=%3")
		//	.arg(row).arg(curCheckValue).arg(remainder));
		++row;
	}
	// then, continue, as long as first varied parameter is (approximately) the same as checkValue0
	while (row < paramValues[sens->data().m_variedParams[0]].size() &&
		dblApproxEqual(curCheckValue, checkValue0) )
	{
		curCheckValue = paramValues[sens->data().m_variedParams[0]][row];
		//LOG(lvlDebug, QString("Row %1: curCheckValue=%2").arg(row).arg(curCheckValue));
		++row;
	}

	sens->data().m_starGroupSize = static_cast<int>(row - 1);
	sens->data().m_numOfSTARSteps = (sens->data().m_starGroupSize - 1) / sens->data().m_variedParams.size();
	if (paramValues[0].size() % sens->data().m_starGroupSize != 0)
	{
		LOG(lvlError, QString("Expected a number of STAR groups of size %1; "
			"but %2 (the number of samples) isn't divisible by that number!")
			.arg(sens->data().m_starGroupSize).arg(paramValues[0].size()));
		return QSharedPointer<iASensitivityInfo>();
	}

	LOG(lvlDebug, QString("In %1 parameter sets, found %2 varying parameters, in STAR groups of %3 (parameter branch size: %4)")
		.arg(paramValues[0].size())
		.arg(sens->data().m_variedParams.size())
		.arg(sens->data().m_starGroupSize)
		.arg(sens->data().m_numOfSTARSteps)
	);
	//LOG(lvlInfo,QString("Determined that there are groups of size: %1; number of STAR points per parameter: %2")
	//	.arg(sensitivity->m_starGroupSize)
	//	.arg(sensitivity->m_numOfSTARSteps)
	//);

	// select output features to compute sensitivity for:
	// - the loaded and computed ones (length, orientation, ...)
	// - dissimilarity measure(s)
	if (charSelected.isEmpty())
	{
		iACharacteristicsMeasureDlg dlg(data);
		if (dlg.exec() != QDialog::Accepted)
		{
			return QSharedPointer<iASensitivityInfo>();
		}
		sens->data().m_charSelected = dlg.selectedCharacteristics();
		sens->data().m_charDiffMeasure = dlg.selectedDiffMeasures();
	}
	else
	{
		sens->data().m_charSelected = charSelected;
		sens->data().m_charDiffMeasure = charDiffMeasure;
	}
	for (int j = sens->data().m_charSelected.size() - 1; j >= 0; --j)
	{
		int charIdx = sens->data().m_charSelected[j];
		// make sure of all histograms for the same characteristic have the same range
		if (data->spmData->paramRange(charIdx)[0] == data->spmData->paramRange(charIdx)[1])
		{
			LOG(lvlInfo,
				QString("Characteristic %1 does not vary, excluding from analysis!")
					.arg(data->spmData->parameterName(charIdx)));
			sens->data().m_charSelected.remove(j);
		}
	}
	//sens->data().dissimMeasure = dlg.selectedMeasures();
	sens->data().m_histogramBins = histogramBins;
	if (sens->data().m_charSelected.size() == 0 || sens->data().m_charDiffMeasure.size() == 0)
	{
		QMessageBox::warning(child, "Sensitivity", "You have to select at least one characteristic and at least one measure!", QMessageBox::Ok);
		return QSharedPointer<iASensitivityInfo>();
	}
	if (!QFile::exists(sens->data().dissimilarityMatrixCacheFileName()))
	{
		iAMeasureSelectionDlg selectMeasure;
		if (selectMeasure.exec() != QDialog::Accepted)
		{
			return QSharedPointer<iASensitivityInfo>();
		}
		sens->data().m_resultDissimMeasures = selectMeasure.measures();
		sens->data().m_resultDissimOptimMeasureIdx = selectMeasure.optimizeMeasureIdx();
	}
	iAProgress* sensP = new iAProgress();
	sens->m_projectToLoad = projectFile;
	auto sensitivityComputation = runAsync(
		[sens, sensP] { sens->data().compute(sensP); },
		[sens, sensP] { sens->createGUI(); delete sensP; },
		child);
	iAJobListView::get()->addJob("Computing sensitivity data",
		sensP, sensitivityComputation, sens.data());
	return sens;
}

iASensitivityInfo::iASensitivityInfo(QSharedPointer<iAFiberResultsCollection> data,
	QString const& parameterFileName, int skipColumns, QStringList const& paramNames,
	std::vector<std::vector<double>> const & paramValues, iAMdiChild* child, QDockWidget* nextToDW,
	std::vector<iAFiberResultUIData> const & resultUIs, vtkRenderWindow* main3DWin) :
	m_data(new iASensitivityData(data, paramNames, paramValues)),
	m_parameterFileName(parameterFileName),
	m_skipColumns(skipColumns),
	m_child(child),
	m_nextToDW(nextToDW),
	m_resultUIs(resultUIs),
	m_main3DWin(main3DWin),
	m_aborted(false)
{
}

typedef iAQTtoUIConnector<QWidget, Ui_SensitivitySettings> iASensitivitySettingsUI;

class iASensitivitySettingsView: public iASensitivitySettingsUI
{
	iAWidgetMap m_settingsWidgetMap;
	iAQRadioButtonVector m_rgChartType;
	const QString ProjectMeasure = "SensitivityCharacteristicsMeasure";
	const QString ProjectAggregation = "SensitivityAggregation";
	const QString ProjectDissimilarity = "SensitivityDissimilarity";
	const QString ProjectChartType = "SensitivityChartType";
	const QString ProjectColorScale = "SensitivitySPColorScale";
	const QString ProjectUnselectedSTARLines = "SensitivityUnselectedSTARLines";

public:
	iASensitivitySettingsView(iASensitivityInfo* sensInf)
	{
		cmbboxMeasure->addItems(DistributionDifferenceMeasureNames());
		cmbboxAggregation->addItems(AggregationNames());
		cmbboxAggregation->setCurrentIndex(DefaultAggregationMeasureIdx);

		QStringList dissimilarities;
		for (auto dissim : sensInf->data().m_resultDissimMeasures)
		{
			dissimilarities << getAvailableDissimilarityMeasureNames()[dissim.first];
		}
		cmbboxDissimilarity->addItems(dissimilarities);

		cmbboxSPColorMap->addItems(iALUT::GetColorMapNames());
		cmbboxSPColorMap->setCurrentText("Brewer single hue 5c grays");

		cmbboxSpatialOverviewColorMap->addItems(iALUT::GetColorMapNames());	// TODO: filter for linear (non-diverging) color maps
		cmbboxSpatialOverviewColorMap->setCurrentText("Matplotlib: Plasma");

		cmbboxSPHighlightColorMap->addItems(iAColorThemeManager::instance().availableThemes());
		cmbboxSPHighlightColorMap->setCurrentText(iASensitivityInfo::DefaultResultColorMap);

		connect(cmbboxMeasure, QOverload<int>::of(&QComboBox::currentIndexChanged), sensInf, &iASensitivityInfo::changeDistributionMeasure);
		connect(cmbboxAggregation, QOverload<int>::of(&QComboBox::currentIndexChanged), sensInf, &iASensitivityInfo::changeAggregation);
		connect(cmbboxCharDiff, QOverload<int>::of(&QComboBox::currentIndexChanged), sensInf, &iASensitivityInfo::changedCharDiffMeasure);

		connect(cmbboxDissimilarity, QOverload<int>::of(&QComboBox::currentIndexChanged), sensInf, &iASensitivityInfo::updateDissimilarity);
		connect(cmbboxSPColorMap, QOverload<int>::of(&QComboBox::currentIndexChanged), sensInf, &iASensitivityInfo::updateSPDifferenceColors);
		connect(cmbboxSPHighlightColorMap, QOverload<int>::of(&QComboBox::currentIndexChanged), sensInf,
			&iASensitivityInfo::updateSPHighlightColors);
		connect(cmbboxSpatialOverviewColorMap, QOverload<int>::of(&QComboBox::currentIndexChanged), sensInf,
			&iASensitivityInfo::updateSpatialOverviewColors);
		cmbboxAlgoInfoMode->setCurrentIndex(iAAlgorithmInfo::DefaultDisplayMode);
		connect(cmbboxAlgoInfoMode, QOverload<int>::of(&QComboBox::currentIndexChanged), sensInf, &iASensitivityInfo::algoInfoModeChanged);
		connect(cbAlgoMaxPerOut, &QCheckBox::stateChanged, sensInf, &iASensitivityInfo::algoInfoNormPerOutChanged);

		connect(cbUnselectedSTARLines, &QCheckBox::stateChanged, sensInf, &iASensitivityInfo::updateSPDifferenceColors);

		connect(rbBar, &QRadioButton::toggled, sensInf, &iASensitivityInfo::histoChartTypeToggled);
		connect(rbLines, &QRadioButton::toggled, sensInf, &iASensitivityInfo::histoChartTypeToggled);

		cmbboxAggregation->setMinimumWidth(80);
		cmbboxMeasure->setMinimumWidth(80);
		cmbboxDissimilarity->setMinimumWidth(80);

		m_rgChartType.push_back(rbBar);
		m_rgChartType.push_back(rbLines);
		m_settingsWidgetMap.insert(ProjectMeasure, cmbboxMeasure);
		m_settingsWidgetMap.insert(ProjectAggregation, cmbboxAggregation);
		m_settingsWidgetMap.insert(ProjectDissimilarity, cmbboxDissimilarity);
		m_settingsWidgetMap.insert(ProjectChartType, &m_rgChartType);
		m_settingsWidgetMap.insert(ProjectColorScale, cmbboxSPColorMap);
		m_settingsWidgetMap.insert(ProjectUnselectedSTARLines, cbUnselectedSTARLines);
	}
	void loadSettings(iASettings const & s)
	{
		::loadSettings(s, m_settingsWidgetMap);
	}
	void saveSettings(QSettings& s)
	{
		::saveSettings(s, m_settingsWidgetMap);
	}
	int dissimMeasIdx() const
	{
		return cmbboxDissimilarity->currentIndex();
	}
	QString spColorMap() const
	{
		return cmbboxSPColorMap->currentText();
	}
};


// TODO: needed? remove!
class iAParameterListView : public QWidget
{
public:
	iAParameterListView(QStringList const& paramNames,
		std::vector<std::vector<double>> const& paramValues,
		QVector<int> variedParams,
		iADissimilarityMatrixType const& dissimMatrix)
	{
		auto paramScrollArea = new QScrollArea();
		paramScrollArea->setWidgetResizable(true);
		auto paramList = new QWidget();
		paramScrollArea->setWidget(paramList);
		paramScrollArea->setContentsMargins(0, 0, 0, 0);
		auto paramListLayout = new QGridLayout();
		paramListLayout->setSpacing(LayoutSpacing);
		paramListLayout->setContentsMargins(1, 0, 1, 0);
		paramListLayout->setColumnStretch(0, 1);
		paramListLayout->setColumnStretch(1, 2);

		enum ParamColumns
		{
			NameCol = 0,
			MatrixCol
		};
		addHeaderLabel(paramListLayout, NameCol, "Parameter", QSizePolicy::Fixed);
		addHeaderLabel(paramListLayout, MatrixCol, "Sensitivity Matrix", QSizePolicy::Expanding);
		for (auto p : variedParams)
		{
			auto paramNameLabel = new QLabel(paramNames[p]);
			paramNameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
			paramListLayout->addWidget(paramNameLabel, p + 1, NameCol);

			auto paramMatrix = new iAMatrixWidget(dissimMatrix, paramValues, false, false);
			paramMatrix->setSortParameter(p);
			paramMatrix->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
			paramMatrix->setData(0);
			paramMatrix->setLookupTable(iALUT::Build(paramMatrix->range(), iALUT::GetColorMapNames()[0], 255, 255));
			m_matrixPerParam.push_back(paramMatrix);
			paramListLayout->addWidget(paramMatrix, p + 1, MatrixCol);
		}
		setLayout(paramListLayout);
	}

	void dissimMatrixMeasureChanged(int idx)
	{
		for (auto paramMatrix : m_matrixPerParam)
		{
			paramMatrix->setData(idx);
			paramMatrix->update();
		}
	}

	void dissimMatrixColorMapChanged(int idx)
	{
		for (auto paramMatrix : m_matrixPerParam)
		{
			paramMatrix->setLookupTable(iALUT::Build(paramMatrix->range(), iALUT::GetColorMapNames()[idx], 255, 255));
			paramMatrix->update();
		}
	}
private:
	std::vector<iAMatrixWidget*> m_matrixPerParam;
};


class iAColorMapWidget: public QWidget
{
public:
	iAColorMapWidget()
		: m_lut(new iALookupTable())
	{	// create default lookup table:
		m_lut->allocate(2);
		m_lut->setColor(0, ScatterPlotPointColor);
		m_lut->setColor(1, ScatterPlotPointColor);
		m_lut->setRange(0, 1);
	}
	void setColorMap(QSharedPointer<iALookupTable> lut)
	{
		m_lut = lut;
	}
private:
	const int ScalarBarPadding = 5;
	QSharedPointer<iALookupTable> m_lut;
	void paintEvent(QPaintEvent* ev) override
	{
		Q_UNUSED(ev);
		QPainter p(this);
		QString minStr = dblToStringWithUnits(m_lut->getRange()[0], 9);
		QString maxStr = dblToStringWithUnits(m_lut->getRange()[1], 9);
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
		int textWidth = std::max(p.fontMetrics().horizontalAdvance(minStr), p.fontMetrics().horizontalAdvance(maxStr));
#else
		int textWidth = std::max(p.fontMetrics().width(minStr), p.fontMetrics().width(maxStr));
#endif
		int scalarBarWidth = geometry().width() - 2 * ScalarBarPadding - textWidth;
		// Draw scalar bar (duplicated from iAQSplom!)
		QPoint topLeft(ScalarBarPadding+textWidth, ScalarBarPadding);

		QRect colorBarRect(topLeft.x(), topLeft.y(), scalarBarWidth, height() - 2 * ScalarBarPadding);
		QLinearGradient grad(topLeft.x(), topLeft.y(), topLeft.x(), topLeft.y() + colorBarRect.height());
		QMap<double, QColor>::iterator it;
		for (size_t i = 0; i < m_lut->numberOfValues(); ++i)
		{
			double rgba[4];
			m_lut->getTableValue(i, rgba);
			QColor color(rgba[0] * 255, rgba[1] * 255, rgba[2] * 255, rgba[3] * 255);
			double key = 1 - (static_cast<double>(i) / (m_lut->numberOfValues() - 1));
			grad.setColorAt(key, color);
		}
		p.fillRect(colorBarRect, grad);
		p.drawRect(colorBarRect);
		// Draw color bar / name of parameter used for coloring
		int colorBarTextX = topLeft.x() - (textWidth + ScalarBarPadding);
		p.drawText(colorBarTextX, topLeft.y() + p.fontMetrics().height(), maxStr);
		p.drawText(colorBarTextX, height() - ScalarBarPadding, minStr);
	}
};

struct iAPolyDataRenderer
{
	vtkSmartPointer<vtkRenderer> renderer;
	std::vector<vtkSmartPointer<vtkPolyData>> data;
	std::vector<vtkSmartPointer<vtkActor>> actor;
	vtkSmartPointer<vtkCornerAnnotation> text;

	vtkSmartPointer<vtkPolyData> diffPoints;
	vtkSmartPointer<vtkPolyDataMapper> diffPtMapper;
	vtkSmartPointer<vtkActor> diffActor;
};

class iASensitivityGUI : public iASensitivityViewState
{
public:
	iASensitivityGUI(iASensitivityInfo* sensInf) :
		m_sensInf(sensInf),
		m_paramInfluenceView(nullptr),
		m_settings(nullptr),
		m_paramSP(nullptr),
		m_mdsSP(nullptr),
		m_colorMapWidget(nullptr),
		m_dwParamInfluence(nullptr),
		m_matrixWidget(nullptr),
		m_parameterListView(nullptr),
		m_algoInfo(nullptr),
		m_diff3DWidget(nullptr),
		m_diff3DRenderManager(/*sharedCamera = */true),
		m_diff3DEmptyRenderer(vtkSmartPointer<vtkRenderer>::New()),
		m_diff3DEmptyText(vtkSmartPointer<vtkCornerAnnotation>::New())
	{
		m_diff3DEmptyText->SetLinearFontScaleFactor(2);
		m_diff3DEmptyText->SetNonlinearFontScaleFactor(1);
		m_diff3DEmptyText->SetMaximumFontSize(18);
		m_diff3DEmptyText->SetText(2, "No Fiber/Result selected");
		auto textColor = qApp->palette().color(QPalette::Text);
		m_diff3DEmptyText->GetTextProperty()->SetColor(textColor.redF(), textColor.greenF(), textColor.blueF());
		auto bgColor = qApp->palette().color(QPalette::Window);
		m_diff3DEmptyRenderer->SetBackground(bgColor.redF(), bgColor.greenF(), bgColor.blueF());
		m_diff3DEmptyRenderer->AddViewProp(m_diff3DEmptyText);
	}
	//! sensitivity information
	iASensitivityInfo* m_sensInf;

	//! Param Influence List
	iAParameterInfluenceView* m_paramInfluenceView;

	//! Overall settings
	iASensitivitySettingsView* m_settings;

	//! scatter plot for the parameter space plot of all results
	iAScatterPlotWidget* m_paramSP;
	//! scatter plot for the MDS 2D plot of all results
	iAScatterPlotWidget* m_mdsSP;

	//! lookup table for points in scatter plot
	QSharedPointer<iALookupTable> m_lut;
	iAColorMapWidget* m_colorMapWidget;

	iADockWidgetWrapper* m_dwParamInfluence;

	// table used in parameter space / MDS scatter plot:
	QSharedPointer<iASPLOMData> m_mdsData;
	// indices of the columns added in addition to parameters;
	size_t spColIdxMDSX, spColIdxMDSY, spColIdxID, spColIdxDissimilarity, spColIdxFilter;

	iADissimilarityMatrixType m_dissimilarityMatrix;
	iAMatrixWidget* m_matrixWidget;
	iAParameterListView* m_parameterListView;
	iAAlgorithmInfo* m_algoInfo;

	iAQVTKWidget* m_diff3DWidget;
	iADockWidgetWrapper* m_dwDiff3D;
	iARendererViewSync m_diff3DRenderManager;
	std::vector<QSharedPointer<iAPolyDataRenderer>> m_diff3DRenderers;

	vtkSmartPointer<vtkRenderer> m_diff3DEmptyRenderer;
	vtkSmartPointer<vtkCornerAnnotation> m_diff3DEmptyText;

	void updateScatterPlotLUT()
	{
		//LOG(lvlDebug, "\nNEW LUT:");
		//std::set<int> hiGrp;
		//std::set<std::pair<int, int> > hiGrpParam;
		int measureIdx = m_settings->dissimMeasIdx();
		QSet<int> hiParam;
		std::set<size_t> hiGrpAll;
		auto const& hp = m_paramSP->viewData()->highlightedPoints();
		for (auto ptIdx : hp)
		{
			size_t groupID = ptIdx / m_sensInf->data().m_starGroupSize;
			if (ptIdx % m_sensInf->data().m_starGroupSize == 0)
			{
				//LOG(lvlDebug, QString("Selected GROUP: %1").arg(groupID));
				//hiGrp.insert(groupID);
			}
			else
			{
				int paramID = ((ptIdx % m_sensInf->data().m_starGroupSize) - 1) / m_sensInf->data().m_numOfSTARSteps;
				//LOG(lvlDebug, QString("Selected PARAM: %1, %2").arg(groupID).arg(paramID));
				//hiGrpParam.insert(std::make_pair(groupID, paramID));
				hiParam.insert(paramID);
			}
			hiGrpAll.insert(groupID);
		}
		m_paramInfluenceView->setHighlightedParams(hiParam);
		/*
		for (size_t i = 0; i < resultCount; ++i)
		{
			int groupID = static_cast<int>(i / starGroupSize);
			bool highlightGroup = hiGrp.find(groupID) != hiGrp.end();
			QColor c;
			if (i % starGroupSize == 0)
			{
				highlightGroup = highlightGroup ||
					// highlight center also if any of its param subgroups is highlighted:
					std::find_if(hiGrpParam.begin(), hiGrpParam.end(), [groupID](std::pair<int, int> a)
					{
						return a.first == groupID;
					}) != hiGrpParam.end();
				int colVal = highlightGroup ? 0 : 64;
				c = QColor(colVal, colVal, colVal, highlightGroup ? 255 : 128);
				//LOG(lvlDebug, QString("Point %1 (group=%2) : Color=%3, %4, %5, %6")
				//	.arg(i).arg(groupID).arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha()));
			}
			else
			{
				int paramID = ((i % starGroupSize) - 1) / numOfSTARSteps;
				bool highlightParam = hiGrpParam.find(std::make_pair(groupID, paramID)) != hiGrpParam.end();
				int colVal = (highlightParam || highlightGroup) ? 128 : 192;
				int blueVal = 192;
				c = QColor(colVal, colVal, blueVal, (highlightParam || highlightGroup) ? 192 : 128);
				//LOG(lvlDebug, QString("Point %1 (group=%2, paramID=%3) : Color=%4, %5, %6, %7")
				//	.arg(i).arg(groupID).arg(paramID).arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha()));
			}
			m_lut->setColor(i, c);
		}
		m_scatterPlot->setLookupTable(m_lut, m_mdsData->numParams() - 1);
		*/

		for (size_t curResultIdx = 0; curResultIdx < m_sensInf->data().m_data->result.size(); ++curResultIdx)
		{
			size_t refResultIdx;
			if (selectedResults().size() == 1)
			{  // color by difference to currently selected result
				refResultIdx = selectedResults()[0];
			}
			else
			{  // color by difference to STAR center
				refResultIdx = curResultIdx - (curResultIdx % m_sensInf->data().m_starGroupSize);
			}
			m_mdsData->data()[spColIdxDissimilarity][curResultIdx] =
				m_sensInf->data().m_resultDissimMatrix[static_cast<int>(curResultIdx)][static_cast<int>(refResultIdx)]
					.avgDissim[measureIdx];
		}
		m_mdsData->updateRanges();
		//auto rng = m_mdsData->paramRange(m_mdsData->numParams() - SPDissimilarityOffset);
		double rng[2];
		rng[0] = m_sensInf->data().m_resultDissimRanges.size() > 0 ? m_sensInf->data().m_resultDissimRanges[measureIdx].first : 0;
		rng[1] = m_sensInf->data().m_resultDissimRanges.size() > 0 ? m_sensInf->data().m_resultDissimRanges[measureIdx].second : 1;
		*m_lut.data() = iALUT::Build(rng, m_settings->spColorMap(), 255, 0);
		m_paramSP->setLookupTable(m_lut, spColIdxDissimilarity);
		m_mdsSP->setLookupTable(m_lut, spColIdxDissimilarity);

		m_colorMapWidget->setColorMap(m_paramSP->lookupTable());
		m_colorMapWidget->update();

		m_paramSP->viewData()->clearLines();
		m_mdsSP->viewData()->clearLines();

		// we want to build a separate line for each parameter (i.e. in each branch "direction" of the STAR
		// easiest way is to collect all parameter values in a group (done in the vector of size_t/double pairs),
		// then sort this by the parameter values (since we don't know else how many are smaller or larger than
		// the center value), then take the point indices from this ordered vector to form the line.
		size_t groupCount = m_sensInf->data().m_data->result.size() / m_sensInf->data().m_starGroupSize;
		bool unselectedStarLines = m_settings->cbUnselectedSTARLines->isChecked();
		for (size_t groupID=0; groupID < groupCount; ++groupID)
		{
			auto groupStart = groupID * m_sensInf->data().m_starGroupSize;
			if (!unselectedStarLines && hiGrpAll.find(groupID) == hiGrpAll.end())
			{
				continue;
			}
			for (size_t parIdx = 0; parIdx < m_sensInf->data().m_variedParams.size(); ++parIdx)
			{
				int lineSize = 1;
				if (hiGrpAll.find(groupID) != hiGrpAll.end())
				{
					if (parIdx == m_paramSP->paramIndices()[0])
					{
						lineSize = 5;
					}
					else if (parIdx == m_paramSP->paramIndices()[1])
					{
						lineSize = 3;
					}
				}
				using PtData = std::pair<size_t, double>;
				std::vector<PtData> linePtParVal;
				double centerValue = m_mdsData->paramData(parIdx)[groupStart];
				linePtParVal.push_back(std::make_pair(groupStart, centerValue));
				auto paraStart = groupStart + 1 + parIdx * m_sensInf->data().m_numOfSTARSteps;
				for (int inParaIdx = 0; inParaIdx < m_sensInf->data().m_numOfSTARSteps; ++inParaIdx)
				{
					size_t ptIdx = paraStart + inParaIdx;
					double paramValue = m_mdsData->paramData(parIdx)[ptIdx];
					linePtParVal.push_back(std::make_pair(ptIdx, paramValue));
				}
				std::sort(linePtParVal.begin(), linePtParVal.end(), [](PtData const& a, PtData const& b)
					{
						return a.second < b.second;
					});
				std::vector<size_t> linePoints(linePtParVal.size());
				for (size_t i = 0; i < linePoints.size(); ++i)
				{
					linePoints[i] = linePtParVal[i].first;
				}
				m_paramSP->viewData()->addLine(linePoints, QColor(), lineSize);
				if (hiGrpAll.find(groupID) != hiGrpAll.end())
				{
					m_mdsSP->viewData()->addLine(linePoints, QColor(), lineSize);
				}
			}
		}
	}
	std::vector<size_t> const & selectedResults() const override
	{
		if (!m_paramSP)
		{
			static std::vector<size_t> dummy;
			return dummy;
		}
		return m_paramSP->viewData()->highlightedPoints();
	}

	iAColorTheme const* selectedResultColorTheme() const override
	{
		return iAColorThemeManager::instance().theme(m_settings->cmbboxSPHighlightColorMap->currentText());
	}
};

using iADissimilarityMatrixDockContent = iAQTtoUIConnector<QWidget, Ui_DissimilarityMatrix>;

QWidget* iASensitivityInfo::setupMatrixView(QVector<int> const& measures)
{
	iADissimilarityMatrixDockContent* dissimDockContent = new iADissimilarityMatrixDockContent();
	auto measureNames = getAvailableDissimilarityMeasureNames();
	QStringList computedMeasureNames;
	for (int m = 0; m < measures.size(); ++m)
	{
		computedMeasureNames << measureNames[measures[m]];
	}
	dissimDockContent->cbMeasure->addItems(computedMeasureNames);
	dissimDockContent->cbParameter->addItems(m_data->m_paramNames);
	dissimDockContent->cbColorMap->addItems(iALUT::GetColorMapNames());
	connect(dissimDockContent->cbMeasure, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &iASensitivityInfo::dissimMatrixMeasureChanged);
	connect(dissimDockContent->cbParameter, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &iASensitivityInfo::dissimMatrixParameterChanged);
	connect(dissimDockContent->cbColorMap, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &iASensitivityInfo::dissimMatrixColorMapChanged);                          // showAxes currently buggy - crashes!
	m_gui->m_matrixWidget = new iAMatrixWidget(m_data->m_resultDissimMatrix, m_data->m_paramValues, true, false);
	m_gui->m_matrixWidget->setSortParameter(0);
	m_gui->m_matrixWidget->setData(0);
	m_gui->m_matrixWidget->setLookupTable(iALUT::Build(m_gui->m_matrixWidget->range(), iALUT::GetColorMapNames()[0], 255, 255));
	dissimDockContent->matrix->layout()->addWidget(m_gui->m_matrixWidget);
	return dissimDockContent;
}

void iASensitivityInfo::dissimMatrixMeasureChanged(int idx)
{
	m_gui->m_matrixWidget->setData(idx);
	m_gui->m_matrixWidget->update();
	m_gui->m_parameterListView->dissimMatrixMeasureChanged(idx);
}

void iASensitivityInfo::dissimMatrixParameterChanged(int idx)
{
	m_gui->m_matrixWidget->setSortParameter(idx);
	m_gui->m_matrixWidget->update();
}

void iASensitivityInfo::dissimMatrixColorMapChanged(int idx)
{
	m_gui->m_matrixWidget->setLookupTable(iALUT::Build(m_gui->m_matrixWidget->range(), iALUT::GetColorMapNames()[idx], 255, 255));
	m_gui->m_matrixWidget->update();
	m_gui->m_parameterListView->dissimMatrixColorMapChanged(idx);
}

QString iASensitivityData::charactName(int selCharIdx) const
{
	return m_data->spmData->parameterName(m_charSelected[selCharIdx])
		.replace("[µm]", "")
		.replace("[µm²]", "")
		.replace("[µm³]", "")
		.replace("[°]", "");
}

namespace
{
	const QString ProjectParameterFile("ParameterSetsFile");
	const QString ProjectCharacteristics("Characteristics");
	const QString ProjectCharDiffMeasures("CharacteristicDifferenceMeasures");
	const QString ProjectSkipParameterCSVColumns("SkipParameterCSVColumns");
	const QString ProjectHistogramBins("DistributionHistogramBins"); // needs to match ProjectDistributionHistogramBins in iAFiAKErController.cpp
	//const QString ProjectResultDissimilarityMeasure("ResultDissimilarityMeasures");
}

void iASensitivityInfo::saveProject(QSettings& projectFile, QString  const& fileName)
{
	projectFile.setValue(ProjectParameterFile, MakeRelative(QFileInfo(fileName).absolutePath(), m_parameterFileName));
	projectFile.setValue(ProjectSkipParameterCSVColumns, m_skipColumns);
	projectFile.setValue(ProjectCharacteristics, joinNumbersAsString(m_data->m_charSelected, ","));
	projectFile.setValue(ProjectCharDiffMeasures, joinNumbersAsString(m_data->m_charDiffMeasure, ","));
	m_gui->m_settings->saveSettings(projectFile);

	// stored in cache file:
	//projectFile.setValue(ProjectResultDissimilarityMeasure, joinAsString(m_resultDissimMeasures, ",",
	//	[](std::pair<int, bool> const& a) {return QString::number(a.first)+":"+(a.second?"true":"false"); }));
}

bool iASensitivityInfo::hasData(iASettings const& settings)
{
	return settings.contains(ProjectParameterFile);
}


QSharedPointer<iASensitivityInfo> iASensitivityInfo::load(iAMdiChild* child,
	QSharedPointer<iAFiberResultsCollection> data, QDockWidget* nextToDW, iASettings const& projectFile,
	QString const& projectFileName, std::vector<iAFiberResultUIData> const& resultUIs, vtkRenderWindow* main3DWin)
{
	QString parameterSetFileName = MakeAbsolute(QFileInfo(projectFileName).absolutePath(), projectFile.value(ProjectParameterFile).toString());
	QVector<int> charsSelected = stringToVector<QVector<int>, int>(projectFile.value(ProjectCharacteristics).toString());
	QVector<int> charDiffMeasure = stringToVector<QVector<int>, int>(projectFile.value(ProjectCharDiffMeasures).toString());
	int skipColumns = projectFile.value(ProjectSkipParameterCSVColumns, 1).toInt();
	int histogramBins = projectFile.value(ProjectHistogramBins, 20).toInt();
	return iASensitivityInfo::create(child, data, nextToDW, histogramBins, skipColumns, resultUIs, main3DWin,
		parameterSetFileName, charsSelected, charDiffMeasure, projectFile);
}

class iASPParamPointInfo final: public iAScatterPlotPointInfo
{
public:
	iASPParamPointInfo(QSharedPointer<iASensitivityData> data) :
		m_data(data)
	{}
	QString text(const size_t paramIdx[2], size_t pointIdx) override
	{
		Q_UNUSED(paramIdx);
		QString result(QString("Fiber Count: %1<br/>").arg(m_data->m_data->result[pointIdx].fiberCount));
		for (int i = 0; i < m_data->m_variedParams.size(); ++i)
		{
			result +=
				m_data->m_paramNames[m_data->m_variedParams[i]] + ": " +
				QString::number(m_data->m_paramValues[m_data->m_variedParams[i]][pointIdx], 'f', 3) + "<br/>";
				/*
				m_data->parameterName(paramIdx[0]) + ": " +
				QString::number(m_data->paramData(paramIdx[0])[pointIdx]) + "<br>" +
				m_data->parameterName(paramIdx[1]) + ": " +
				QString::number(m_data->paramData(paramIdx[1])[pointIdx])
				*/
		}
		return result;
	}
private:
	QSharedPointer<iASensitivityData> m_data;
};

void iASensitivityInfo::createGUI()
{
	if (m_aborted)
	{
		emit aborted();
		return;
	}
	m_gui.reset(new iASensitivityGUI(this));

	iAProgress* spatP = new iAProgress();
	auto spatialVariationComputation = runAsync([this, spatP] { data().computeSpatialOverview(spatP); },
		[this, spatP] {
			showSpatialOverview();
			delete spatP;
		},
		m_child);
	iAJobListView::get()->addJob("Computing spatial overview", spatP, spatialVariationComputation, this);

	m_gui->m_settings = new iASensitivitySettingsView(this);
	auto dwSettings = new iADockWidgetWrapper(m_gui->m_settings, "Sensitivity Settings", "foeSensitivitySettings");
	m_child->splitDockWidget(m_nextToDW, dwSettings, Qt::Horizontal);

	m_gui->m_paramInfluenceView = new iAParameterInfluenceView(m_data, m_gui, ParamColor, OutputColor);
	m_gui->m_dwParamInfluence =
		new iADockWidgetWrapper(m_gui->m_paramInfluenceView, "Parameter Influence", "foeParamInfluence");
	connect(m_gui->m_paramInfluenceView, &iAParameterInfluenceView::barAdded, this, &iASensitivityInfo::outputBarAdded);
	connect(
		m_gui->m_paramInfluenceView, &iAParameterInfluenceView::barRemoved, this, &iASensitivityInfo::outputBarRemoved);
	connect(m_gui->m_paramInfluenceView, &iAParameterInfluenceView::resultSelected, this,
		&iASensitivityInfo::parResultSelected);
	m_child->splitDockWidget(dwSettings, m_gui->m_dwParamInfluence, Qt::Vertical);

	QStringList algoInNames;
	for (auto p : m_data->m_variedParams)
	{
		algoInNames.push_back(m_data->m_paramNames[p]);
	}
	QStringList algoOutNames;
	for (int charIdx = 0; charIdx < m_data->m_charSelected.size(); ++charIdx)
	{
		algoOutNames << m_data->charactName(charIdx);
	}
	m_gui->m_algoInfo = new iAAlgorithmInfo("Fiber Reconstruction", algoInNames, algoOutNames, ParamColor, OutputColor);
	m_gui->m_algoInfo->addShownOut(0);  // equivalent to addStackedBar in iAParameterInfluenceView constructor!
	connect(m_gui->m_algoInfo, &iAAlgorithmInfo::inputClicked, m_gui->m_paramInfluenceView,
		&iAParameterInfluenceView::setSelectedParam);
	connect(m_gui->m_algoInfo, &iAAlgorithmInfo::outputClicked, m_gui->m_paramInfluenceView,
		&iAParameterInfluenceView::toggleCharacteristic);
	connect(m_gui->m_paramInfluenceView, &iAParameterInfluenceView::orderChanged, m_gui->m_algoInfo,
		&iAAlgorithmInfo::setInSortOrder);
	connect(m_gui->m_paramInfluenceView, &iAParameterInfluenceView::parameterChanged, [this]() {
		m_gui->m_algoInfo->setSelectedInput(m_gui->m_paramInfluenceView->selectedRow());
	});
	m_gui->m_algoInfo->setInSortOrder(m_gui->m_paramInfluenceView->paramIndicesSorted());
	auto dwAlgoInfo = new iADockWidgetWrapper(m_gui->m_algoInfo, "Algorithm Details", "foeAlgorithmInfo");
	m_child->splitDockWidget(dwSettings, dwAlgoInfo, Qt::Horizontal);

	QVector<int> measures;
	for (auto d : m_data->m_resultDissimMeasures)
	{
		measures.push_back(d.first);
	}
	QWidget* dissimDockContent = setupMatrixView(measures);
	auto dwDissimMatrix = new iADockWidgetWrapper(dissimDockContent, "Dissimilarity Matrix", "foeMatrix");
	m_child->splitDockWidget(m_gui->m_dwParamInfluence, dwDissimMatrix, Qt::Vertical);

	m_gui->m_parameterListView = new iAParameterListView(
		m_data->m_paramNames, m_data->m_paramValues, m_data->m_variedParams, m_data->m_resultDissimMatrix);
	auto dwParamView = new iADockWidgetWrapper(m_gui->m_parameterListView, "Parameter View", "foeParameters");
	m_child->splitDockWidget(m_gui->m_dwParamInfluence, dwParamView, Qt::Vertical);

	m_gui->m_mdsData = QSharedPointer<iASPLOMData>(new iASPLOMData());
	std::vector<QString> spParamNames;
	for (auto p : m_data->m_variedParams)
	{
		spParamNames.push_back(m_data->m_paramNames[p]);
	}
	m_gui->spColIdxMDSX = spParamNames.size();
	spParamNames.push_back("MDS X");
	m_gui->spColIdxMDSY = spParamNames.size();
	spParamNames.push_back("MDS Y");
	m_gui->spColIdxID = spParamNames.size();
	spParamNames.push_back("ID");
	m_gui->spColIdxDissimilarity = spParamNames.size();
	spParamNames.push_back("Dissimilarity");  // dissimilarity according to currently selected result
	m_gui->spColIdxFilter = spParamNames.size();
	spParamNames.push_back("Filter");  // just for filtering results only from current "STAR plane"
	size_t resultCount = m_data->m_data->result.size();
	m_gui->m_mdsData->setParameterNames(spParamNames, resultCount);
	for (size_t c = 0; c < spParamNames.size(); ++c)
	{
		m_gui->m_mdsData->data()[c].resize(resultCount);
	}
	for (int p = 0; p < m_data->m_variedParams.size(); ++p)
	{  // set parameter values:
		int origParamIdx = m_data->m_variedParams[p];
		for (size_t r = 0; r < resultCount; ++r)
		{
			m_gui->m_mdsData->data()[p][r] = m_data->m_paramValues[origParamIdx][r];
		}
	}
	for (size_t i = 0; i < resultCount; ++i)
	{
		m_gui->m_mdsData->data()[m_gui->spColIdxMDSX][i] = 0.0;           // MDS X
		m_gui->m_mdsData->data()[m_gui->spColIdxMDSY][i] = 0.0;           // MDS Y
		m_gui->m_mdsData->data()[m_gui->spColIdxID][i] = i;               // ID
		m_gui->m_mdsData->data()[m_gui->spColIdxDissimilarity][i] = 0.0;  // Dissimilarity
		m_gui->m_mdsData->data()[m_gui->spColIdxFilter][i] = 0.0;         // Filter
	}
	m_gui->m_mdsData->updateRanges();
	m_gui->m_lut.reset(new iALookupTable());
	m_gui->m_lut->setRange(0, m_data->m_data->result.size());
	m_gui->m_lut->allocate(m_data->m_data->result.size());

	m_gui->m_paramSP = new iAScatterPlotWidget(m_gui->m_mdsData, true);
	m_gui->m_paramSP->setPointRadius(4);
	m_gui->m_paramSP->setPickedPointFactor(1.5);
	m_gui->m_paramSP->setFixPointsEnabled(true);
	m_gui->m_paramSP->setHighlightColorTheme(
		iAColorThemeManager::instance().theme(m_gui->m_settings->cmbboxSPHighlightColorMap->currentText()));
	m_gui->m_paramSP->setHighlightDrawMode(iAScatterPlot::Enlarged | iAScatterPlot::CategoricalColor);
	m_gui->m_paramSP->setSelectionEnabled(false);
	auto sortedParams = m_gui->m_paramInfluenceView->paramIndicesSorted();
	m_gui->m_paramSP->setVisibleParameters(sortedParams[0], sortedParams[1]);
	m_gui->m_paramSP->setMinimumWidth(50);
	connect(m_gui->m_paramSP, &iAScatterPlotWidget::pointHighlighted, this, &iASensitivityInfo::spPointHighlighted);
	connect(m_gui->m_paramSP, &iAScatterPlotWidget::highlightChanged, this, &iASensitivityInfo::spHighlightChanged);
	connect(m_gui->m_paramSP, &iAScatterPlotWidget::visibleParamChanged, this, &iASensitivityInfo::spVisibleParamChanged);

	m_gui->m_mdsSP = new iAScatterPlotWidget(m_gui->m_mdsData, true);
	m_gui->m_mdsSP->setPointRadius(4);
	m_gui->m_mdsSP->setPickedPointFactor(1.5);
	m_gui->m_mdsSP->setFixPointsEnabled(true);
	m_gui->m_mdsSP->setHighlightColorTheme(
		iAColorThemeManager::instance().theme(m_gui->m_settings->cmbboxSPHighlightColorMap->currentText()));
	m_gui->m_mdsSP->setHighlightDrawMode(iAScatterPlot::Enlarged | iAScatterPlot::CategoricalColor);
	m_gui->m_mdsSP->setSelectionEnabled(false);
	m_gui->m_mdsSP->setVisibleParameters(m_gui->spColIdxMDSX, m_gui->spColIdxMDSY);
	m_gui->m_mdsSP->setMinimumWidth(50);
	connect(m_gui->m_mdsSP, &iAScatterPlotWidget::pointHighlighted, this, &iASensitivityInfo::spPointHighlighted);
	connect(m_gui->m_mdsSP, &iAScatterPlotWidget::highlightChanged, this, &iASensitivityInfo::spHighlightChanged);

	m_gui->m_colorMapWidget = new iAColorMapWidget();
	m_gui->m_colorMapWidget->setMinimumWidth(50);
	m_gui->m_colorMapWidget->setMaximumWidth(100);

	auto splitter = new QSplitter();
	splitter->setOrientation(Qt::Horizontal);
	splitter->setChildrenCollapsible(true);
	splitter->addWidget(m_gui->m_paramSP);
	splitter->addWidget(m_gui->m_mdsSP);
	splitter->addWidget(m_gui->m_mdsSP);
	splitter->addWidget(m_gui->m_colorMapWidget);

	auto dwSP = new iADockWidgetWrapper(splitter, "Constellation Charts", "foeParamSP");
	m_child->splitDockWidget(m_gui->m_dwParamInfluence, dwSP, Qt::Vertical);

	m_gui->updateScatterPlotLUT();

	auto ptInfo = QSharedPointer<iASPParamPointInfo>::create(m_data);
	m_gui->m_paramSP->setPointInfo(ptInfo);
	m_gui->m_mdsSP->setPointInfo(ptInfo);

	m_gui->m_diff3DWidget = new iAQVTKWidget();
	m_gui->m_dwDiff3D = new iADockWidgetWrapper(m_gui->m_diff3DWidget, "Difference 3D", "foeDiff3D");
	m_child->splitDockWidget(dwSettings, m_gui->m_dwDiff3D, Qt::Horizontal);
	m_gui->m_diff3DRenderManager.addToBundle(m_main3DWin->GetRenderers()->GetFirstRenderer());
	m_gui->m_diff3DWidget->renderWindow()->AddRenderer(m_gui->m_diff3DEmptyRenderer);
	m_gui->m_dwDiff3D->hide();

	spVisibleParamChanged();
	updateDissimilarity();
	changeAggregation(DefaultAggregationMeasureIdx);

	if (!m_projectToLoad.isEmpty())
	{
		m_gui->m_settings->loadSettings(m_projectToLoad);
	}
}

void iASensitivityInfo::showSpatialOverview()
{
	if (!m_data->m_spatialOverview && !m_data->m_averageFiberVoxel)	// the computation of any of the two images (or both) might have been aborted
	{
		return;
	}
	// show image
	QSharedPointer<iAModalityList> mods(new iAModalityList());
	if (m_data->m_spatialOverview)
	{
		mods->add(QSharedPointer<iAModality>::create("Avg unique fiber/voxel", data().spatialOverviewCacheFileName(),
			1, m_data->m_spatialOverview, iAModality::MainRenderer));
	}
	if (m_data->m_averageFiberVoxel)
	{
		mods->add(QSharedPointer<iAModality>::create("Mean objects (fibers/voxel)",
			data().averageFiberVoxelCacheFileName(), 1, m_data->m_averageFiberVoxel, iAModality::MainRenderer));
	}

	m_child->setModalities(mods);
	connect(m_child, &iAMdiChild::histogramAvailable, this, &iASensitivityInfo::setSpatialOverviewTF);
	for (int m = 0; m < mods->size(); ++m)
	{
		m_child->setHistogramModality(m);
	}
}

void iASensitivityInfo::setSpatialOverviewTF(int modalityIdx)
{
	auto mod = m_child->modality(modalityIdx);
	double const* range = mod->image()->GetScalarRange();
	vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
	iALUT::BuildLUT(lut, range, m_gui->m_settings->cmbboxSpatialOverviewColorMap->currentText(), 5, true);
	auto ctf = mod->transfer()->colorTF();
	auto otf = mod->transfer()->opacityTF();
	const double AlphaOverride = 0.2;
	const double MinPoint = 0.001;
	convertLUTToTF(lut, ctf, otf, AlphaOverride);
	double rgb0[4];
	ctf->GetColor(0.0, rgb0);
	ctf->AddRGBPoint(0.0, 0.0, 0.0, 0.0);
	ctf->AddRGBPoint(MinPoint, rgb0[0], rgb0[1], rgb0[2]);
	otf->AddPoint(0.0, 0.0);
	otf->AddPoint(MinPoint, AlphaOverride);
	mod->updateRenderer();
}

void iASensitivityInfo::updateSpatialOverviewColors()
{
	for (int m = 0; m < m_child->modalities()->size(); ++m)
	{
		setSpatialOverviewTF(m);
	}
	m_child->histogram()->update();
}

QVector<QVector<double>> iASensitivityInfo::currentAggregatedSensitivityMatrix()
{
	int charDiffMode = m_gui->m_settings->cmbboxCharDiff->currentIndex();
	int aggrType = m_gui->m_paramInfluenceView->selectedAggrType();
	int measure = m_gui->m_paramInfluenceView->selectedMeasure();

	QVector<QVector<double>> result(data().m_charSelected.size());
	for (int c=0; c<data().m_charSelected.size(); ++c)
	{
		result[c].resize(data().m_variedParams.size());
		for (int r = 0; r < data().m_variedParams.size(); ++r)
		{
			result[c][r] = (charDiffMode == 0) ? data().aggregatedSensitivities[c][measure][aggrType][r]
											   : data().aggregatedSensitivitiesPWDiff[c][aggrType][r];
		}
	}
	return result;
}

void iASensitivityInfo::changeAggregation(int newAggregation)
{
	m_gui->m_paramInfluenceView->setAggregation(newAggregation);
	m_gui->m_algoInfo->setMatrix(currentAggregatedSensitivityMatrix());
}

void iASensitivityInfo::changeDistributionMeasure(int newMeasure)
{
	m_gui->m_paramInfluenceView->setDistributionMeasure(newMeasure);
	m_gui->m_algoInfo->setMatrix(currentAggregatedSensitivityMatrix());
}

void iASensitivityInfo::changedCharDiffMeasure(int newMeasure)
{
	m_gui->m_paramInfluenceView->setCharDiffMeasure(newMeasure);
	m_gui->m_algoInfo->setMatrix(currentAggregatedSensitivityMatrix());
}

void iASensitivityInfo::outputBarAdded(int outType, int outIdx)
{
	if (outType == outCharacteristic)
	{
		m_gui->m_algoInfo->addShownOut(outIdx);
		m_gui->m_algoInfo->update();
	}
}
void iASensitivityInfo::outputBarRemoved(int outType, int outIdx)
{
	if (outType == outCharacteristic)
	{
		m_gui->m_algoInfo->removeShownOut(outIdx);
		m_gui->m_algoInfo->update();
	}
}

void iASensitivityInfo::fiberSelectionChanged(std::vector<std::vector<size_t>> const & selection)
{
	m_baseFiberSelection = selection;
	m_currentFiberSelection = selection;

	size_t selectedFibers = 0, resultCount = 0;
	for (size_t resultID = 0; resultID < selection.size(); ++resultID)
	{
		selectedFibers += selection[resultID].size();
		resultCount += (selection[resultID].size() > 0) ? 1 : 0;
	}
	LOG(lvlDebug, QString("New fiber selection: %1 selected fibers in %2 results").arg(selectedFibers).arg(resultCount));

	updateDifferenceView();
}

void iASensitivityInfo::updateDissimilarity()
{
	int dissimIdx = m_gui->m_settings->cmbboxDissimilarity->currentIndex();
	iAMatrixType distMatrix(m_data->m_data->result.size(), std::vector<double>(m_data->m_data->result.size()));
	//LOG(lvlDebug, "Distance Matrix:");
	for (qvectorsizetype r1 = 0; r1 < static_cast<qvectorsizetype>(distMatrix.size()); ++r1)
	{
		QString line;
		for (qvectorsizetype r2 = 0; r2 < static_cast<qvectorsizetype>(distMatrix.size()); ++r2)
		{
			distMatrix[r1][r2] = m_data->m_resultDissimMatrix[r1][r2].avgDissim[dissimIdx];
			line += " " + QString::number(distMatrix[r1][r2], 'f', 2).rightJustified(5);
		}
		//LOG(lvlDebug, QString("%1:%2").arg(r1).arg(line));
	}
	auto mds = computeMDS(distMatrix, 2, 100);
	//LOG(lvlDebug, "MDS:");
	for (size_t i = 0; i < mds.size(); ++i)
	{
		for (size_t c = 0; c < mds[0].size(); ++c)
		{
			m_gui->m_mdsData->data()[m_gui->spColIdxMDSX + c][i] = mds[i][c];
		}
		//LOG(lvlDebug, QString("%1: %2, %3").arg(i).arg(mds[i][0]).arg(mds[i][1]));
	}
	m_gui->m_mdsData->updateRanges();
}

void iASensitivityInfo::spPointHighlighted(size_t resultIdx, bool state)
{
	int paramID = -1;
	auto sender = qobject_cast<iAScatterPlotWidget*>(QObject::sender());
	auto const& hp = sender->viewData()->highlightedPoints();
	auto t = iAColorThemeManager::instance().theme(m_gui->m_settings->cmbboxSPHighlightColorMap->currentText());
	QColor resultColor = t->color(hp.size() - 1);
	if (!state)
	{
		auto theme = iAColorThemeManager::instance().theme(m_gui->m_settings->cmbboxSPHighlightColorMap->currentText());
		m_gui->m_paramInfluenceView->updateHighlightColors(hp, theme);
	}
	if (state && resultIdx % m_data->m_starGroupSize != 0)
	{	// specific parameter "branch" selected:
		paramID = ((resultIdx % m_data->m_starGroupSize) - 1) / m_data->m_numOfSTARSteps;
	}
	m_gui->m_paramInfluenceView->setResultSelected(resultIdx, state, resultColor);
	m_gui->m_paramInfluenceView->setSelectedParam(paramID);
	emit resultSelected(resultIdx, state);

	iAScatterPlotWidget* otherSP = (sender == m_gui->m_paramSP) ? m_gui->m_mdsSP : m_gui->m_paramSP;
	if (state)
	{
		otherSP->viewData()->addHighlightedPoint(resultIdx);
	}
	else
	{
		otherSP->viewData()->removeHighlightedPoint(resultIdx);
	}

	if (m_currentFiberSelection.size() == 0)
	{	// before first selection is made...
		return;
	}

	// in any case, remove all eventually selected fibers in result from current selection:
	m_currentFiberSelection[resultIdx].clear();
	if (state)
	{	// add fibers matching the selection in m_baseFiberSelection to current selection:
		auto it = std::find(m_data->m_resultDissimMeasures.begin(), m_data->m_resultDissimMeasures.end(), std::make_pair(7, true));
		int measIdx = (it != m_data->m_resultDissimMeasures.end()) ? it - m_data->m_resultDissimMeasures.begin() : 0;
		for (size_t rSel = 0; rSel < m_baseFiberSelection.size(); ++rSel)
		{
			if (m_baseFiberSelection[rSel].size() == 0)
			{
				continue;
			}
			if (rSel == resultIdx)
			{
				m_currentFiberSelection[rSel] = m_baseFiberSelection[rSel];
				continue;
			}
			for (auto rSelFibID : m_baseFiberSelection[rSel])
			{
				auto& fiberDissim = m_data->m_resultDissimMatrix[static_cast<qvectorsizetype>(rSel)][static_cast<qvectorsizetype>(resultIdx)].fiberDissim[static_cast<qvectorsizetype>(rSelFibID)];
				if (fiberDissim.size() == 0)
				{
					continue;
				}
				auto& similarFibers = fiberDissim[measIdx];
				for (auto similarFiber : similarFibers)
				{
					/*
					LOG(lvlDebug,
						QString("        Fiber %1, dissimilarity: %2%3")
							.arg(similarFiber.index)
							.arg(similarFiber.dissimilarity)
							.arg(similarFiber.dissimilarity >= 1 ? " (skipped)" : ""));
					*/
					if (similarFiber.dissimilarity < 1 &&
						std::find(m_currentFiberSelection[resultIdx].begin(), m_currentFiberSelection[resultIdx].end(),
							similarFiber.index) == m_currentFiberSelection[resultIdx].end())
					{
						m_currentFiberSelection[resultIdx].push_back(similarFiber.index);
					}
				}
			}
		}
		std::sort(m_currentFiberSelection[resultIdx].begin(), m_currentFiberSelection[resultIdx].end());
	}
	// TODO: change detection - only trigger fibersToSelect if selection has changed?
	emit fibersToSelect(m_currentFiberSelection);
}

void iASensitivityInfo::histoChartTypeToggled(bool checked)
{
	if (checked)
	{
		m_gui->m_paramInfluenceView->setHistogramChartType(qobject_cast<QRadioButton*>(QObject::sender())->text());
	}
}

void iASensitivityInfo::parResultSelected(size_t resultIdx, Qt::KeyboardModifiers modifiers)
{
	m_gui->m_paramSP->toggleHighlightedPoint(resultIdx, modifiers);
	// mdsSP will get updated through signal triggered by paramSP
}

void iASensitivityInfo::updateSPDifferenceColors()
{
	m_gui->updateScatterPlotLUT();
}

void iASensitivityInfo::updateSPHighlightColors()
{
	QString colorThemeName = m_gui->m_settings->cmbboxSPHighlightColorMap->currentText();
	auto theme = iAColorThemeManager::instance().theme(colorThemeName);
	m_gui->m_paramSP->setHighlightColorTheme(theme);
	m_gui->m_mdsSP->setHighlightColorTheme(theme);
	m_gui->m_paramInfluenceView->updateHighlightColors(m_gui->m_paramSP->viewData()->highlightedPoints(), theme);
	emit resultColorsChanged(colorThemeName);
}

void iASensitivityInfo::spHighlightChanged()
{
	updateSPDifferenceColors();
	updateDifferenceView();
}

void iASensitivityInfo::spVisibleParamChanged()
{
	size_t const* visPar = m_gui->m_paramSP->paramIndices();
	for (size_t r = 0; r < m_data->m_data->result.size(); ++r)
	{
		size_t inGroupIdx = r % m_data->m_starGroupSize;
		bool visible =
			// the STAR centers are always visible:
			(inGroupIdx == 0) ||
			// if one of two shown "parameters" is MDS x/y, show all results:
			static_cast<int>(visPar[0]) >= m_data->m_variedParams.size() ||
			static_cast<int>(visPar[1]) >= m_data->m_variedParams.size();
		if (!visible)
		{	// otherwise, show result only if it's on a branch for one of the two shown parameters:
			size_t starBranchParamIdx = (inGroupIdx - 1) / m_data->m_numOfSTARSteps;
			visible = (starBranchParamIdx == visPar[0] || starBranchParamIdx == visPar[1]);
		}
		m_gui->m_mdsData->data()[m_gui->spColIdxFilter][r] = visible ? 1 : 0;
	}
	m_gui->m_paramSP->viewData()->addFilter(m_gui->spColIdxFilter, 1.0);
}

iASensitivityData& iASensitivityInfo::data()
{
	return *m_data.data();
}

void iASensitivityInfo::updateDifferenceView()
{
	//iATimeGuard timer("ShowDifference");
	auto renWin = m_gui->m_diff3DWidget->renderWindow();
	auto const& hp = m_gui->m_paramSP->viewData()->highlightedPoints();

	// TODO: reuse actors... / store what was previously shown and only update if something has changed?
	//LOG(lvlDebug, QString("%1 Results!").arg(hp.size()));
	for (auto r: m_gui->m_diff3DRenderers)
	{
		m_gui->m_diff3DRenderManager.removeFromBundle(r->renderer);
		renWin->RemoveRenderer(r->renderer);
	}
	//renWin->GetRenderers()->RemoveAllItems();	// should also work instead of above RemoveRenderer call
	m_gui->m_diff3DRenderers.clear();
	// TODO: determine "central" resultID to compare to / fixed comparison point determined by user?

	auto t = iAColorThemeManager::instance().theme(m_gui->m_settings->cmbboxSPHighlightColorMap->currentText());
	for (size_t i=0; i<hp.size(); ++i)
	{
		auto rID = hp[i];
		if (!m_resultUIs[rID].main3DVis)
		{
			LOG(lvlDebug, QString("Result %1: 3D vis not initialized!").arg(rID));
			continue;
		}
		auto resultData = QSharedPointer<iAPolyDataRenderer>::create();
		resultData->data = m_resultUIs[rID].main3DVis->extractSelectedObjects(QColor(128, 128, 128));
		if (resultData->data.size() == 0)
		{
			//LOG(lvlDebug, QString("Result %1: No selected fibers!").arg(rID));
			continue;
		}
		resultData->renderer = vtkSmartPointer<vtkRenderer>::New();
		auto bgColor(qApp->palette().color(QPalette::Window));
		resultData->renderer->SetBackground(bgColor.redF(), bgColor.greenF(), bgColor.blueF());
		resultData->renderer->SetViewport(
			static_cast<double>(i) / hp.size(), 0, static_cast<double>(i + 1) / hp.size(), 1);
		for (size_t f = 0; f < resultData->data.size(); ++f)
		{
			auto diffMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
			diffMapper->SetInputData(resultData->data[f]);
			resultData->actor.push_back(vtkSmartPointer<vtkActor>::New());
			resultData->actor[resultData->actor.size()-1]->SetMapper(diffMapper);
			resultData->actor[resultData->actor.size() - 1]->GetProperty()->SetOpacity(0.3);
			diffMapper->SetScalarModeToUsePointFieldData();
			resultData->actor[resultData->actor.size() - 1]->GetProperty()->SetColor(0.5, 0.5, 0.5);
			diffMapper->Update();
			resultData->renderer->AddActor(resultData->actor[f]);
		}
		auto txt = QString("Result %1").arg(rID);
		resultData->text = vtkSmartPointer<vtkCornerAnnotation>::New();
		resultData->text->SetLinearFontScaleFactor(2);
		resultData->text->SetNonlinearFontScaleFactor(1);
		resultData->text->SetMaximumFontSize(18);
		QColor color = t->color(i);
		resultData->text->GetTextProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
		resultData->text->SetText(2, txt.toStdString().c_str());
		// ToDo: add fiber id ;
		//auto textColor = qApp->palette().color(QPalette::Text);
		//resultData->text->GetTextProperty()->SetColor(textColor.redF(), textColor.greenF(), textColor.blueF());
		//cornerAnnotation->GetTextProperty()->BoldOn();
		resultData->renderer->AddViewProp(resultData->text);

		//m_diffActor->GetProperty()->SetPointSize(2);
		/*
		m_main3DWin->GetRenderers()->GetFirstRenderer()->AddActor(m_diffActor);
		m_main3DWin->Render();
		m_main3DWidget->update();
		*/

		// show differences - for now only for 1st fiber, to 1st fiber of 1st selected result:
		if (i > 0 && m_currentFiberSelection[hp[0]].size() > 0 && m_currentFiberSelection[rID].size() > 0)
		{
			auto refResID = hp[0];
			size_t refFiberID = m_currentFiberSelection[refResID][0];
			auto& ref = m_data->m_data->result[refResID];
			auto const& refMapping = *ref.mapping.data();
			auto refIt = ref.curveInfo.find(refFiberID);
			iAFiberData refFiber(ref.table, refFiberID, refMapping,
				refIt != ref.curveInfo.end() ? refIt->second : std::vector<iAVec3f>());

			auto& d = m_data->m_data->result[rID];
			auto const& mapping = *d.mapping.data();
			std::vector<iAVec3f> sampledPoints;
			size_t fiber0ID = m_currentFiberSelection[rID][0];
			auto it = d.curveInfo.find(fiber0ID);
			iAFiberData sampleFiber(
				d.table, fiber0ID, mapping, it != d.curveInfo.end() ? it->second : std::vector<iAVec3f>());

			vtkNew<vtkPolyData> ptData;
			vtkNew<vtkPoints> points;
			auto colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
			colors->SetNumberOfComponents(3);
			colors->SetName("Colors");

			// direction fiber -> refFiber
			// everything that's in the fiber but not in the reference -> color red
			samplePoints(sampleFiber, sampledPoints, 10000);
			size_t newPts = 0;
			for (size_t s = 0; s < sampledPoints.size(); ++s)
			{
				if (!pointContainedInFiber(sampledPoints[s], refFiber))
				{
					double pt[3];
					for (int c = 0; c < 3; ++c) pt[c] = sampledPoints[s][c];
					points->InsertNextPoint(pt);
					++newPts;
				}
			}
			unsigned char onlyInThisColor[3] = {    // Qt documentation states that red/green/blue deliver 0..255, so cast is OK
				static_cast<unsigned char>(t->color(i).red()),
				static_cast<unsigned char>(t->color(i).green()),
				static_cast<unsigned char>(t->color(i).blue())
			};
			for (size_t s = 0; s < newPts; ++s)
			{
				colors->InsertNextTypedTuple(onlyInThisColor);
			}

			// direction refFiber -> fiber
			// -> color blue
			samplePoints(refFiber, sampledPoints, 10000);
			newPts = 0;
			for (size_t s = 0; s < sampledPoints.size(); ++s)
			{
				if (!pointContainedInFiber(sampledPoints[s], sampleFiber))
				{
					double pt[3];
					for (int c = 0; c < 3; ++c) pt[c] = sampledPoints[s][c];
					points->InsertNextPoint(pt);
					++newPts;
				}
			}
			unsigned char onlyInRefColor[3] = {
				static_cast<unsigned char>(t->color(0).red()),
				static_cast<unsigned char>(t->color(0).green()),
				static_cast<unsigned char>(t->color(0).blue())
			};
			for (size_t s = 0; s < newPts; ++s)
			{
				colors->InsertNextTypedTuple(onlyInRefColor);
			}

			ptData->SetPoints(points.GetPointer());
			auto vertexFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
			vertexFilter->SetInputData(ptData.GetPointer());
			vertexFilter->Update();

			resultData->diffPoints = vtkSmartPointer<vtkPolyData>::New();
			resultData->diffPoints->DeepCopy(vertexFilter->GetOutput());
			resultData->diffPoints->GetPointData()->SetScalars(colors);
			resultData->diffPtMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
			resultData->diffPtMapper->SetInputData(resultData->diffPoints);
			resultData->diffActor = vtkSmartPointer<vtkActor>::New();
			resultData->diffActor->SetMapper(resultData->diffPtMapper);
			resultData->diffPtMapper->Update();
			resultData->diffActor->GetProperty()->SetPointSize(4);
			resultData->renderer->AddActor(resultData->diffActor);
		}
		renWin->AddRenderer(resultData->renderer);
		m_gui->m_diff3DRenderManager.addToBundle(resultData->renderer);
		m_gui->m_diff3DRenderers.push_back(resultData);
	}
	if (m_gui->m_diff3DRenderers.size() == 0 && !renWin->GetRenderers()->IsItemPresent(m_gui->m_diff3DEmptyRenderer))
	{
		m_gui->m_dwDiff3D->hide();
		renWin->AddRenderer(m_gui->m_diff3DEmptyRenderer);
	}
	else if (m_gui->m_diff3DRenderers.size() > 0 && renWin->GetRenderers()->IsItemPresent(m_gui->m_diff3DEmptyRenderer))
	{
		m_gui->m_dwDiff3D->show();
		renWin->RemoveRenderer(m_gui->m_diff3DEmptyRenderer);
	}
	renWin->Render();
	m_gui->m_diff3DWidget->update();
}

void iASensitivityInfo::styleChanged()
{
	auto textColor = qApp->palette().color(QPalette::Text);
	m_gui->m_diff3DEmptyText->GetTextProperty()->SetColor(textColor.redF(), textColor.greenF(), textColor.blueF());
	auto bgColor = qApp->palette().color(QPalette::Window);
	m_gui->m_diff3DEmptyRenderer->SetBackground(bgColor.redF(), bgColor.greenF(), bgColor.blueF());
	for (auto r : m_gui->m_diff3DRenderers)
	{
		r->renderer->SetBackground(bgColor.redF(), bgColor.greenF(), bgColor.blueF());
		r->text->GetTextProperty()->SetColor(textColor.redF(), textColor.greenF(), textColor.blueF());
	}
}

void iASensitivityInfo::algoInfoModeChanged(int mode)
{
	m_gui->m_algoInfo->setMode(mode);
}

void iASensitivityInfo::algoInfoNormPerOutChanged(int state)
{
	m_gui->m_algoInfo->setNormalizePerOutput(state == Qt::Checked);
}
