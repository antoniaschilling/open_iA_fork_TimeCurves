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
#include "iAParameterInfluenceView.h"

#include "iAFiberCharData.h"
#include "iAFiberData.h" // for getAvailableDissimilarityMeasureNames
#include "iASensitivityInfo.h"
#include "iAStackedBarChart.h"
#include "qthelper/iAClickableLabel.h"

#include <charts/iAChartWidget.h>
#include <charts/iAHistogramData.h>
#include <charts/iAPlotTypes.h>
#include <charts/iASPLOMData.h>
#include <iAColorTheme.h>
#include <iALog.h>

#include <QAction>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QScrollArea>

namespace// merge with iASensitivityinfo!
{
	enum ColumnIndices { colParamName = 0, colMin = 1, colMax = 2, colStep = 3, colStackedBar = 4
		//, colHistogram = 5 
	};
	int LayoutSpacing = 0; int LayoutMargin = 4;
	const QString DefaultStackedBarColorTheme("Brewer Accent (max. 8)");
}

iAParameterInfluenceView::iAParameterInfluenceView(iASensitivityInfo* sensInf) :
	m_sensInf(sensInf),
	m_measureIdx(0),
	m_aggrType(0),
	m_selectedRow(-1),
	m_selectedCol(-1),
	m_paramListLayout(new QGridLayout())
{
	setLayout(new QHBoxLayout);
	layout()->setContentsMargins(1, 0, 1, 0);
	auto paramScrollArea = new QScrollArea();
	paramScrollArea->setWidgetResizable(true);
	auto paramList = new QWidget();
	paramScrollArea->setWidget(paramList);
	paramScrollArea->setContentsMargins(0, 0, 0, 0);
	layout()->addWidget(paramScrollArea);

	paramList->setLayout(m_paramListLayout);
	m_paramListLayout->setSpacing(LayoutSpacing);
	m_paramListLayout->setContentsMargins(LayoutMargin, LayoutMargin, LayoutMargin, LayoutMargin);
	m_paramListLayout->setColumnStretch(colParamName, 1);
	m_paramListLayout->setColumnStretch(colMin, 1);
	m_paramListLayout->setColumnStretch(colMax, 1);
	m_paramListLayout->setColumnStretch(colStep, 1);
	m_paramListLayout->setColumnStretch(colStackedBar, 10);
	//m_paramListLayout->setColumnStretch(colHistogram, 10);

	auto colorTheme = iAColorThemeManager::instance().theme(DefaultStackedBarColorTheme);
	m_stackedHeader = new iAStackedBarChart(colorTheme, true);
	connect(m_stackedHeader, &iAStackedBarChart::barDblClicked, this, &iAParameterInfluenceView::stackedBarDblClicked);
	// TODO: Unify/Group stacked bar widgets here / in iAFIAKERController into a class
	// which encapsulates updating weights, showing columns, unified data interface (table?)
	// for all characteristics, add column to stacked bar charts

	for (int charactIdx = 0; charactIdx < sensInf->m_charSelected.size(); ++charactIdx)
	{
		addColumnAction(outCharacteristic, charactIdx, charactIdx == 0);
	}
	addColumnAction(outFiberCount, -1, false);
	for (int dissimMeasIdx = 0; dissimMeasIdx < sensInf->m_resultDissimMeasures.size(); ++dissimMeasIdx)
	{
		addColumnAction(outDissimilarity, dissimMeasIdx, false);
	
	}
	//LOG(lvlDebug, QString("Adding lines for %1 characteristics").arg(sensInf->m_charSelected.size()));

	// headers:
	addHeaderLabel(m_paramListLayout, colParamName, "Parameter");
	addHeaderLabel(m_paramListLayout, colMin, "Min");
	addHeaderLabel(m_paramListLayout, colMax, "Max");
	addHeaderLabel(m_paramListLayout, colStep, "Step");
	m_paramListLayout->addWidget(m_stackedHeader, 0, colStackedBar);
	//addHeaderLabel(m_paramListLayout, colHistogram, "Difference Distribution");

	for (int paramIdx = 0; paramIdx < sensInf->m_variedParams.size(); ++paramIdx)
	{
		QString paramName = sensInf->m_paramNames[sensInf->m_variedParams[paramIdx]];
		m_stackedCharts.push_back(new iAStackedBarChart(colorTheme, false,
			paramIdx == sensInf->m_variedParams.size() - 1, true, "Var. from " + paramName));
		connect(m_stackedHeader, &iAStackedBarChart::weightsChanged, m_stackedCharts[paramIdx], &iAStackedBarChart::setWeights);
		m_stackedCharts[paramIdx]->setProperty("paramIdx", paramIdx);
		connect(m_stackedCharts[paramIdx], &iAStackedBarChart::clicked, this, &iAParameterInfluenceView::paramChangedSlot);
		connect(m_stackedHeader, &iAStackedBarChart::normalizeModeChanged, m_stackedCharts[paramIdx], &iAStackedBarChart::setNormalizeMode);
		connect(m_stackedHeader, &iAStackedBarChart::switchedStackMode, m_stackedCharts[paramIdx], &iAStackedBarChart::setDoStack);
		auto const& paramVec = sensInf->m_paramValues[sensInf->m_variedParams[paramIdx]];
		double minVal = *std::min_element(paramVec.begin(), paramVec.end()),
			maxVal = *std::max_element(paramVec.begin(), paramVec.end());
		iAClickableLabel* labels[4];
		labels[colParamName] = new iAClickableLabel(paramName);
		labels[colMin] = new iAClickableLabel(QString::number(minVal));
		labels[colMax] = new iAClickableLabel(QString::number(maxVal));
		labels[colStep] = new iAClickableLabel(QString::number(sensInf->paramStep[paramIdx]));
		for (int i = colParamName; i <= colStep; ++i)
		{
			labels[i]->setProperty("paramIdx", paramIdx);
			m_paramListLayout->addWidget(labels[i], 1 + paramIdx, i);
			connect(labels[i], &iAClickableLabel::clicked, this, &iAParameterInfluenceView::paramChangedSlot);
		}
		m_paramListLayout->addWidget(m_stackedCharts[paramIdx], 1 + paramIdx, colStackedBar);

		//m_diffChart.push_back(new iAChartWidget(this, "Characteristics distribution", ));
		//m_paramListLayout->addWidget(m_diffChart[paramIdx], 1 + paramIdx, colHistogram);
	}
	// default stacked bar content/settings:
	m_stackedHeader->setDoStack(false);
	m_stackedHeader->setNormalizeMode(false);
	for (auto chart : m_stackedCharts)
	{
		chart->setNormalizeMode(false);
		chart->setDoStack(false);
	}
	addStackedBar(outCharacteristic, 0);
}

void iAParameterInfluenceView::addColumnAction(int objectType, int charactIdx, bool checked)
{
	auto charactShowAction = new QAction(columnName(objectType, charactIdx), nullptr);
	charactShowAction->setProperty("objectType", objectType);
	charactShowAction->setProperty("charactIdx", charactIdx);
	charactShowAction->setCheckable(true);
	charactShowAction->setChecked(checked);
	QObject::connect(charactShowAction, &QAction::triggered, this, &iAParameterInfluenceView::showStackedBar);
	m_stackedHeader->contextMenu()->addAction(charactShowAction);
}

void iAParameterInfluenceView::setMeasure(int newMeasure)
{
	m_measureIdx = newMeasure;
	updateStackedBars();
	emit parameterChanged();
}

void iAParameterInfluenceView::setAggregation(int newAggregation)
{
	m_aggrType = newAggregation;
	updateStackedBars();
	emit parameterChanged();
}

int iAParameterInfluenceView::selectedMeasure() const { return m_measureIdx; }
int iAParameterInfluenceView::selectedAggrType() const { return m_aggrType; }
int iAParameterInfluenceView::selectedRow() const { return m_selectedRow; }
int iAParameterInfluenceView::selectedCol() const { return m_selectedCol; }

void iAParameterInfluenceView::setColorTheme(iAColorTheme const * colorTheme)
{
	m_stackedHeader->setColorTheme(colorTheme);
	for (auto stackedChart : m_stackedCharts)
	{
		stackedChart->setColorTheme(colorTheme);
	}
}

void iAParameterInfluenceView::showStackedBar()
{
	auto source = qobject_cast<QAction*>(QObject::sender());
	int objectType = source->property("objectType").toInt();
	int charactIdx = source->property("charactIdx").toInt();
	if (source->isChecked())
	{
		addStackedBar(objectType, charactIdx);
	}
	else
	{
		removeStackedBar(objectType, charactIdx);
	}
}

void iAParameterInfluenceView::selectStackedBar(int outputType, int outTypeIdx)
{
	int barIdx = m_stackedHeader->barIndex(columnName(outputType, outTypeIdx));
	if (barIdx == -1)
	{
		return;
	}
	m_selectedCol = barIdx;
	m_stackedHeader->setSelectedBar(m_selectedCol);
	for (auto stackedChart: m_stackedCharts)
	{
		stackedChart->setSelectedBar(m_selectedCol);
	}
}

void iAParameterInfluenceView::stackedBarDblClicked(int barIdx)
{
	QString barName = m_stackedHeader->barName(barIdx);
	QPair<int, int> selected;
	for (auto outTypeIdx : m_visibleCharacts)
	{
		if (columnName(outTypeIdx.first, outTypeIdx.second) == barName)
		{
			selected = outTypeIdx;
			break;
		}
	}
	selectStackedBar(selected.first, selected.second);
	emit outputSelected(selected.first, selected.second);
}

void iAParameterInfluenceView::paramChangedSlot()
{
	auto source = qobject_cast<QWidget*>(QObject::sender());
	setSelectedParam(source->property("paramIdx").toInt());
}

void iAParameterInfluenceView::setSelectedParam(int param)
{
	m_selectedRow = param;
	for (int paramIdx = 0; paramIdx < m_sensInf->m_variedParams.size(); ++paramIdx)
	{
		// QPalette::Highlight / QPalette::HighlightedText
		QColor color = palette().color(paramIdx == m_selectedRow ? QPalette::Mid : backgroundRole());
		//QColor textColor = palette().color(paramIdx == m_selectedRow ?  : QPalette::Text);
		for (int col = colParamName; col <= colStep; ++col)
		{
			m_paramListLayout->itemAtPosition(paramIdx+1, col)->widget()->setStyleSheet(
				"QLabel { background-color : " + color.name() + "; }");
		}
		m_stackedCharts[paramIdx]->setBackgroundColor(color);
		m_stackedCharts[paramIdx]->update();
	}
	emit parameterChanged();
}

void iAParameterInfluenceView::updateStackedBars()
{
	for (auto col : m_visibleCharacts)
	{
		auto const& d =
		 ((col.first == outCharacteristic)  ? m_sensInf->aggregatedSensitivities[col.second][m_measureIdx] :
		     ((col.first == outFiberCount)  ? m_sensInf->aggregatedSensitivitiesFiberCount
		 /*(col.first == outDissimilarity)*/: m_sensInf->aggregatedSensDissim[col.second]))[m_aggrType];
		// TODO: unify with addStackedBar
		auto title(columnName(col.first, col.second));
		double maxValue = std::numeric_limits<double>::lowest();
		for (int paramIdx = 0; paramIdx < m_sensInf->m_variedParams.size(); ++paramIdx)
		{
			if (d[paramIdx] > maxValue)
			{
				maxValue = d[paramIdx];
			}
		}
		for (int paramIdx = 0; paramIdx < m_sensInf->m_variedParams.size(); ++paramIdx)
		{
			m_stackedCharts[paramIdx]->updateBar(title, d[paramIdx], maxValue);
			updateStackedBarHistogram(title, paramIdx, col.first, col.second);
		}
	}
	for (int paramIdx = 0; paramIdx < m_sensInf->m_variedParams.size(); ++paramIdx)
	{
		m_stackedCharts[paramIdx]->update();
	}
	updateChartY();
}

QString iAParameterInfluenceView::columnName(int outType, int outIdx) const
{
	return  +
		(outType == outCharacteristic) ? QString("Variation ") + m_sensInf->charactName(outIdx) :
		((outType == outFiberCount) ? "Variation Fiber Count"
		/*(outType == outDissimilarity)*/	:
			getAvailableDissimilarityMeasureNames()[m_sensInf->m_resultDissimMeasures[outIdx].first]);
}

void iAParameterInfluenceView::updateStackedBarHistogram(QString const & barName, int paramIdx, int outType, int outIdx)
{
	int barIdx = m_stackedCharts[paramIdx]->barIndex(barName);
	auto chart = m_stackedCharts[paramIdx]->chart(barIdx);
	if (outType != outCharacteristic)
	{
		return;
	}
	chart->clearPlots();
	const int numBins = m_sensInf->m_histogramBins;
	// improve iAHistogramData to directly take QVector/std::vector data?
	double* myHisto = new double[numBins];
	for (int bin = 0; bin < numBins; ++bin)
	{
		myHisto[bin] = m_sensInf->charHistVarAgg[outIdx][m_aggrType][paramIdx][bin];
	}
	double cMin = m_sensInf->m_data->spmData->paramRange(m_sensInf->m_charSelected[outIdx])[0],
		   cMax = m_sensInf->m_data->spmData->paramRange(m_sensInf->m_charSelected[outIdx])[1];
	chart->addPlot(QSharedPointer<iAPlot>(new iABarGraphPlot(
		iAHistogramData::create(myHisto, numBins, (cMax - cMin) / numBins, cMin, cMax), QColor(80, 80, 80, 128))));
	chart->setXCaption(m_sensInf->charactName(outIdx));
	chart->update();
}

void iAParameterInfluenceView::updateChartY()
{
	double yMin = 0, yMax = std::numeric_limits<double>::lowest();
	for (auto stackedChart : m_stackedCharts)
	{
		yMax = std::max(yMax, stackedChart->maxYValue());
	}
	for (auto stackedChart : m_stackedCharts)
	{
		stackedChart->setChartYRange(yMin, yMax);
	}
}

void iAParameterInfluenceView::addStackedBar(int outType, int outIdx)
{
	m_visibleCharacts.push_back(qMakePair(outType, outIdx));
	auto title(columnName(outType, outIdx));
	LOG(lvlDebug, QString("Showing stacked bar for characteristic %1").arg(title));
	m_stackedHeader->addBar(title, 1, 1);
	auto const& d = (
		   (outType == outCharacteristic)  ? m_sensInf->aggregatedSensitivities[outIdx][m_measureIdx]:
		      ((outType == outFiberCount)  ? m_sensInf->aggregatedSensitivitiesFiberCount
		/*(col.first == outDissimilarity)*/: m_sensInf->aggregatedSensDissim[outIdx]))[m_aggrType];

	double maxValue = std::numeric_limits<double>::lowest();
	for (int paramIdx = 0; paramIdx < m_sensInf->m_variedParams.size(); ++paramIdx)
	{
		if (d[paramIdx] > maxValue)
		{
			maxValue = d[paramIdx];
		}
	}
	for (int paramIdx = 0; paramIdx < m_sensInf->m_variedParams.size(); ++paramIdx)
	{
		m_stackedCharts[paramIdx]->addBar(title, d[paramIdx], maxValue);
		updateStackedBarHistogram(title, paramIdx, outType, outIdx);
		m_stackedCharts[paramIdx]->setLeftMargin(m_stackedCharts[0]->chart(0)->leftMargin());
	}
	updateChartY();
	m_stackedHeader->setLeftMargin(m_stackedCharts[0]->chart(0)->leftMargin());
}

void iAParameterInfluenceView::removeStackedBar(int outType, int outIdx)
{
	int visibleIdx = m_visibleCharacts.indexOf(qMakePair(outType, outIdx));
	if (visibleIdx == -1)
	{
		LOG(lvlError, QString("Invalid state - called remove on non-added bar (outType=%1, outIdx=%2").arg(outType).arg(outIdx));
	}
	m_visibleCharacts.remove(visibleIdx);
	auto title(columnName(outType, outIdx));
	LOG(lvlDebug, QString("Removing stacked bar for characteristic %1").arg(title));
	m_stackedHeader->removeBar(title);
	for (auto stackedChart : m_stackedCharts)
	{
		stackedChart->removeBar(title);
	}
	updateChartY();
}
