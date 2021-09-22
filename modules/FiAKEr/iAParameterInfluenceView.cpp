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
#include "iAParameterInfluenceView.h"

#include "iAFiberResult.h"
#include "iAFiberData.h" // for getAvailableDissimilarityMeasureNames
#include "iASensitivityData.h"
#include "iAStackedBarChart.h"
#include "iAClickableLabel.h"

#include <iAColorTheme.h>
#include <iALog.h>

// charts:
#include <iAChartWidget.h>
#include <iAHistogramData.h>
#include <iAPlotTypes.h>
#include <iASPLOMData.h>
#include <iAXYPlotData.h>

#include <QAction>
#include <QApplication>    // for qApp->palette()
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QScrollArea>

namespace
{
	enum ColumnIndices { colParamName = 0, colMin = 1, colMax = 2, colStep = 3, colStackedBar = 4
		//, colHistogram = 5
	};
	const int GridSpacing = 2;
	const int LayoutMargin = 4;
	const int RowsPerParam = 4;

	enum
	{
		RowStackedHeader = 0,
		RowStackedBar = 1,
		RowOutputChart = 2,
		RowParamChart = 3
	};
	const int LabelCount = 4;

	const QColor VariationHistogramColor(50, 50, 50, 255);
	const QColor AverageHistogramColor(150, 150, 150, 255);
	const QColor ParamSensitivityPlotColor(80, 80, 80, 255);

	//const QColor ParamRowSelectedBGColor(245, 245, 245);
	//const QColor ParamRowUnselectedBGColor(255, 255, 255);
}

class iAParTableRow
{
public:
	iAStackedBarChart* head;
	iAStackedBarChart* bars;
	QVector<iAChartWidget*> out;
	QVector<iAChartWidget*> par;
	iAClickableLabel* labels[LabelCount];
};

iAParameterInfluenceView::iAParameterInfluenceView(QSharedPointer<iASensitivityData> data,
	QSharedPointer<iASensitivityViewState> viewState, QColor const& paramColor, QColor const& outputColor) :
	m_data(data),
	m_viewState(viewState),
	m_measureIdx(0),
	m_aggrType(0),
	m_selectedParam(-1),
	m_selectedCol(-1),
	m_paramListLayout(new QGridLayout()),
	m_stackedBarTheme(new iASingleColorTheme("OneOutputColorTheme", outputColor)),
	m_table(data->m_variedParams.size()),
	m_sort(data->m_variedParams.size()),
	m_sortLastOut(-1),
	m_sortLastDesc(true),
	m_histogramChartType("Bars")    // needs to match value from radio buttons in SensitivitySettings.ui
{
	for (int i=0; i<m_sort.size(); ++i)
	{
		m_sort[i] = i;
	}
	for (int j=0; j<data->m_variedParams.size(); ++j)
	{
		m_table[j] = QSharedPointer<iAParTableRow>::create();
	}
	setLayout(new QHBoxLayout);
	layout()->setContentsMargins(1, 0, 1, 0);
	auto paramScrollArea = new QScrollArea();
	paramScrollArea->setWidgetResizable(true);
	auto paramList = new QWidget();
	paramScrollArea->setWidget(paramList);
	paramScrollArea->setContentsMargins(0, 0, 0, 0);
	layout()->addWidget(paramScrollArea);

	paramList->setLayout(m_paramListLayout);
	m_paramListLayout->setSpacing(GridSpacing);
	m_paramListLayout->setContentsMargins(LayoutMargin, LayoutMargin, LayoutMargin, LayoutMargin);
	// TODO: Unify/Group stacked bar widgets here / in iAFIAKERController into a class
	// which encapsulates updating weights, showing columns, unified data interface (table?)
	// for all characteristics, add column to stacked bar charts

	//LOG(lvlDebug, QString("Adding lines for %1 characteristics").arg(sensInf->m_charSelected.size()));

	// headers:
	addHeaderLabel(m_paramListLayout, colParamName, "Param.", QSizePolicy::Fixed);
	addHeaderLabel(m_paramListLayout, colMin, "Min", QSizePolicy::Fixed);
	addHeaderLabel(m_paramListLayout, colMax, "Max", QSizePolicy::Fixed);
	addHeaderLabel(m_paramListLayout, colStep, "Step", QSizePolicy::Fixed);

	for (int paramIdx = 0; paramIdx < data->m_variedParams.size(); ++paramIdx)
	{
		iAParTableRow * row = m_table[paramIdx].data();
		int rowIdx = 1 + RowsPerParam * paramIdx;
		row->head = new iAStackedBarChart(m_stackedBarTheme.data(), m_paramListLayout,
			rowIdx + RowStackedHeader, colStackedBar, true);
		row->head->setDoStack(false);
		row->head->setNormalizeMode(false);
		connect(row->head, &iAStackedBarChart::barDblClicked, this, &iAParameterInfluenceView::sortListByBar);
		QString paramName = data->m_paramNames[data->m_variedParams[paramIdx]];
		row->bars = new iAStackedBarChart(m_stackedBarTheme.data(), m_paramListLayout,
			rowIdx + RowStackedBar, colStackedBar, false,
			paramIdx == data->m_variedParams.size() - 1);
		row->bars->setNormalizeMode(false);
		row->bars->setDoStack(false);
		row->bars->setProperty("paramIdx", paramIdx);
		connect(row->bars, &iAStackedBarChart::clicked, this, &iAParameterInfluenceView::paramChangedSlot);
		connect(row->head, &iAStackedBarChart::weightsChanged      , this, &iAParameterInfluenceView::setBarWeights);
		connect(row->head, &iAStackedBarChart::normalizeModeChanged, this, &iAParameterInfluenceView::setBarNormalizeMode);
		connect(row->head, &iAStackedBarChart::switchedStackMode   , this, &iAParameterInfluenceView::setBarDoStack);
		auto const& paramVec = data->m_paramValues[data->m_variedParams[paramIdx]];
		double minVal = *std::min_element(paramVec.begin(), paramVec.end()),
			maxVal = *std::max_element(paramVec.begin(), paramVec.end());
		row->labels[colParamName] = new iAClickableLabel(paramName, true);
		row->labels[colParamName]->setStyleSheet("QLabel { background-color : " + paramColor.name() + "; }");
		row->labels[colMin] = new iAClickableLabel(QString::number(minVal, 'f', digitsAfterComma(data->paramStep[paramIdx])), false);
		row->labels[colMax] = new iAClickableLabel(QString::number(maxVal, 'f', digitsAfterComma(data->paramStep[paramIdx])), false);
		row->labels[colStep] = new iAClickableLabel(QString::number(data->paramStep[paramIdx]), false);
		for (int i = colParamName; i <= colStep; ++i)
		{
			row->labels[i]->setProperty("paramIdx", paramIdx);
			row->labels[i]->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
			connect(row->labels[i], &iAClickableLabel::clicked, this, &iAParameterInfluenceView::paramChangedSlot);
		}
		//m_paramListLayout->addWidget(m_stackedCharts[paramIdx], rowIdx, colStackedBar);

		//m_diffChart.push_back(new iAChartWidget(this, "Characteristics distribution", ));
		//m_paramListLayout->addWidget(m_diffChart[paramIdx], 1 + paramIdx, colHistogram);
	}
	for (int charactIdx = 0; charactIdx < data->m_charSelected.size(); ++charactIdx)
	{
		addColumnAction(outCharacteristic, charactIdx, charactIdx == 0);
	}
	addColumnAction(outFiberCount, -1, false);
	for (size_t dissimMeasIdx = 0; dissimMeasIdx < data->m_resultDissimMeasures.size(); ++dissimMeasIdx)
	{
		addColumnAction(outDissimilarity, static_cast<int>(dissimMeasIdx), false);
	}

	addStackedBar(outCharacteristic, 0);  // reflected in iAAlgorithmInfo construction in iASensitivity -> put in common place?

	sortListByBar(0);
}

void iAParameterInfluenceView::addTableWidgets()
{
	for (int paramRow = 0; paramRow < m_table.size(); ++paramRow)
	{
		int paramIdx = m_sort[paramRow];
		int rowIdx = 1 + RowsPerParam * paramRow;
		for (int i=0; i<LabelCount; ++i)
		{
			m_paramListLayout->addWidget(m_table[paramIdx]->labels[i], rowIdx, i, RowsPerParam, 1);
		}
		m_table[paramIdx]->head->setPos(rowIdx+RowStackedHeader, colStackedBar);
		m_table[paramIdx]->bars->setPos(rowIdx+RowStackedBar, colStackedBar);
		for (int c=0; c<m_table[paramIdx]->out.size(); ++c)
		{
			bool outVisible = m_highlightedParams.contains(paramIdx);
			m_paramListLayout->addWidget(m_table[paramIdx]->out[c], rowIdx + RowOutputChart, colStackedBar + c);
			m_paramListLayout->setRowStretch(rowIdx + RowOutputChart, outVisible ? 4 : 0);
			m_table[paramIdx]->out[c]->setMinimumHeight(outVisible ? 80 : 0);
			m_table[paramIdx]->out[c]->setMaximumHeight(outVisible ? 500 : 0);
			m_table[paramIdx]->out[c]->setVisible(outVisible);

			bool parVisible = paramRow < 2 || paramIdx == m_selectedParam;
			m_paramListLayout->addWidget(m_table[paramIdx]->par[c], rowIdx + RowParamChart, colStackedBar + c);
			m_paramListLayout->setRowStretch(rowIdx + RowParamChart, parVisible ? 4 : 0);
			m_table[paramIdx]->par[c]->setMinimumHeight(parVisible ? 80 : 0);
			m_table[paramIdx]->par[c]->setMaximumHeight(parVisible ? 500 : 0);
			m_table[paramIdx]->par[c]->setVisible(parVisible);
		}
	}
}

void iAParameterInfluenceView::addColumnAction(int objectType, int charactIdx, bool checked)
{
	auto charactShowAction = new QAction(columnName(objectType, charactIdx), nullptr);
	charactShowAction->setProperty("objectType", objectType);
	charactShowAction->setProperty("charactIdx", charactIdx);
	charactShowAction->setCheckable(true);
	charactShowAction->setChecked(checked);
	QObject::connect(charactShowAction, &QAction::triggered, this, &iAParameterInfluenceView::showStackedBar);
	for (auto row : m_table)
	{
		row->head->contextMenu()->addAction(charactShowAction);
	}
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
int iAParameterInfluenceView::selectedRow() const { return m_selectedParam; }
int iAParameterInfluenceView::selectedCol() const { return m_selectedCol; }

void iAParameterInfluenceView::showStackedBar()
{
	auto source = qobject_cast<QAction*>(QObject::sender());
	int objectType = source->property("objectType").toInt();
	int charactIdx = source->property("charactIdx").toInt();
	toggleBar(source->isChecked(), objectType, charactIdx);
}

void iAParameterInfluenceView::selectStackedBar(int outputType, int outTypeIdx)
{
	int barIdx = m_table[0]->head->barIndex(columnName(outputType, outTypeIdx));
	m_selectedCol = barIdx;
	for (auto row : m_table)
	{
		row->head->setSelectedBar(m_selectedCol);
		row->bars->setSelectedBar(m_selectedCol);
	}
}

void iAParameterInfluenceView::toggleCharacteristic(int charactIdx)
{
	bool shown = m_visibleCharacts.indexOf(qMakePair(static_cast<int>(outCharacteristic), charactIdx)) != -1;
	toggleBar(!shown, outCharacteristic, charactIdx);
}

void iAParameterInfluenceView::toggleBar(bool show, int outType, int outIdx)
{
	if (show)
	{
		addStackedBar(outType, outIdx);
	}
	else
	{
		removeStackedBar(outType, outIdx);
	}
}

void iAParameterInfluenceView::sortListByBar(int barIdx)
{
	if (m_sortLastOut == barIdx)
	{
		m_sortLastDesc = !m_sortLastDesc;
	}
	m_sortLastOut = barIdx;
	std::sort(m_sort.begin(), m_sort.end(), [this, barIdx](int r1, int r2)
	{
		double v1 = m_table[r1]->bars->barValue(barIdx),
			v2 = m_table[r2]->bars->barValue(barIdx);
		return m_sortLastDesc ? v1 > v2 : v1 < v2;
	});
	//LOG(lvlDebug, joinNumbersAsString(m_sort, ","));
	addTableWidgets();
	emit orderChanged(m_sort);
}

void iAParameterInfluenceView::paramChangedSlot()
{
	auto source = qobject_cast<QWidget*>(QObject::sender());
	setSelectedParam(source->property("paramIdx").toInt());
}

void iAParameterInfluenceView::setBarWeights(std::vector<double> const& weights)
{
	for (auto row: m_table)
	{
		row->head->setWeights(weights);
		row->bars->setWeights(weights);
	}
}

void iAParameterInfluenceView::setBarNormalizeMode(bool normalizePerBar)
{
	for (auto row : m_table)
	{
		row->head->setNormalizeMode(normalizePerBar);
		row->bars->setNormalizeMode(normalizePerBar);
	}
}

void iAParameterInfluenceView::setBarDoStack(bool doStack)
{
	for (auto row : m_table)
	{
		row->head->setDoStack(doStack);
		row->bars->setDoStack(doStack);
	}
}

void iAParameterInfluenceView::addResultHistoPlot(size_t resultIdx, int paramIdx, int barIdx, QColor c)
{
	if (m_visibleCharacts[barIdx].first != outCharacteristic)
	{
		return;
	}
	int charIdx = m_visibleCharacts[barIdx].second;
	auto const rng = m_data->m_data->spmData->paramRange(m_data->m_charSelected[charIdx]);
	auto histData = iAHistogramData::create(QString("Result %1").arg(resultIdx), iAValueType::Continuous, rng[0],
		rng[1], m_data->m_charHistograms[resultIdx][charIdx]);
	auto plotKey = std::make_tuple(resultIdx, paramIdx, charIdx);
	m_selectedResultHistoPlots.insert(plotKey, createHistoPlot(histData, c));
	m_table[paramIdx]->out[barIdx]->addPlot(m_selectedResultHistoPlots[plotKey]);
}

void iAParameterInfluenceView::setResultSelected(size_t resultIdx, bool state, QColor c)
{
	size_t inGroupIdx = resultIdx % m_data->m_starGroupSize;
	int branchIdx = (inGroupIdx != 0) ? (inGroupIdx - 1) % m_data->m_numOfSTARSteps : 0;
	size_t startResult = resultIdx - branchIdx;
	size_t starCenter = (resultIdx / m_data->m_starGroupSize) * m_data->m_starGroupSize;
	for (int paramIdx = 0; paramIdx < m_data->m_variedParams.size(); ++paramIdx)
	{
		//double paramValue = m_data->m_paramValues[m_data->m_variedParams[paramIdx]][resultIdx];
		for (int barIdx = 0; barIdx < m_table[paramIdx]->out.size(); ++barIdx)
		{
			if (m_visibleCharacts[barIdx].first == outCharacteristic)
			{
				int charIdx = m_visibleCharacts[barIdx].second;
				auto plotKey = std::make_tuple(resultIdx, paramIdx, charIdx);
				if (state)
				{
					if (inGroupIdx != 0)
					{
						for (size_t r = 0; r < m_data->m_numOfSTARSteps; ++r)
						{
							auto rIdx = startResult + r;
							double pv = m_data->m_paramValues[m_data->m_variedParams[paramIdx]][rIdx];
							m_table[paramIdx]->par[barIdx]->setXMarker(
								pv, (rIdx == resultIdx) ? c : QColor(192, 192, 192), Qt::DashLine);
						}
					}
					double pv = m_data->m_paramValues[m_data->m_variedParams[paramIdx]][starCenter];
					m_table[paramIdx]->par[barIdx]->setXMarker(pv, inGroupIdx != 0 ? QColor(192, 192, 192) : c, Qt::DashLine);
					if (m_selectedResultHistoPlots.contains(plotKey))
					{
						LOG(lvlWarn, QString("Plot to be added already exists!"));
					}
					else
					{
						addResultHistoPlot(resultIdx, paramIdx, barIdx, c);
					}
				}
				else
				{
					if (inGroupIdx != 0)
					{
						for (size_t r = 0; r < m_data->m_numOfSTARSteps; ++r)
						{
							auto rIdx = startResult + r;
							double pv = m_data->m_paramValues[m_data->m_variedParams[paramIdx]][rIdx];
							m_table[paramIdx]->par[barIdx]->removeXMarker(pv);
						}
					}
					double pv = m_data->m_paramValues[m_data->m_variedParams[paramIdx]][starCenter];
					m_table[paramIdx]->par[barIdx]->removeXMarker(pv);
					if (!m_selectedResultHistoPlots.contains(plotKey))
					{
						LOG(lvlWarn, QString("Plot to be removed does not exist!"));
					}
					else
					{
						m_table[paramIdx]->out[barIdx]->removePlot(m_selectedResultHistoPlots[plotKey]);
						m_selectedResultHistoPlots.remove(plotKey);
					}
				}
			}
		}
	}
}

void iAParameterInfluenceView::updateHighlightColors(std::vector<size_t> highlighted, iAColorTheme const * theme)
{
	for (size_t i=0; i<highlighted.size(); ++i)
	{
		size_t resultIdx = highlighted[i];
		for (int paramIdx = 0; paramIdx < m_data->m_variedParams.size(); ++paramIdx)
		{
			double paramValue = m_data->m_paramValues[m_data->m_variedParams[paramIdx]][resultIdx];
			for (int barIdx = 0; barIdx < m_table[paramIdx]->out.size(); ++barIdx)
			{
				if (m_visibleCharacts[barIdx].first == outCharacteristic)
				{
					int charIdx = m_visibleCharacts[barIdx].second;
					m_table[paramIdx]->par[barIdx]->setXMarker(paramValue, theme->color(i), Qt::DashLine);
					auto plotKey = std::make_tuple(resultIdx, paramIdx, charIdx);
					m_selectedResultHistoPlots[plotKey]->setColor(theme->color(i));
				}

			}
		}
	}
}

void iAParameterInfluenceView::setHighlightedParams(QSet<int> hiParam)
{
	m_highlightedParams = hiParam;
	addTableWidgets();
}

void iAParameterInfluenceView::paramChartClicked(double x, Qt::KeyboardModifiers modifiers)
{
	// search for parameter value "closest" to clicked x;
	auto chart = qobject_cast<iAChartWidget*>(QObject::sender());
	int variedParamIdx = chart->property("paramIdx").toInt();
	auto& paramValues = m_data->m_paramValues[m_data->m_variedParams[variedParamIdx]];
	auto minDistElem = std::min_element(paramValues.begin(), paramValues.end(), [x](double a, double b) {
		return std::abs(a - x) < std::abs(b - x);
	});
	// select the result that is closest to the currently selected one in the other parameters!
	int resultIdx = minDistElem - paramValues.begin();
	emit resultSelected(resultIdx, modifiers);
}

void iAParameterInfluenceView::setSelectedParam(int param)
{
	m_selectedParam = param;
	for (int paramIdx = 0; paramIdx < m_data->m_variedParams.size(); ++paramIdx)
	{
		//QColor color = (paramIdx == m_selectedParam) ? ParamRowSelectedBGColor : ParamRowUnselectedBGColor;
		QPalette::ColorRole bgRole = (paramIdx == m_selectedParam) ? QPalette::AlternateBase : QPalette::Window;
		for (int col = colMin; col <= colStep; ++col)
		{
			m_table[paramIdx]->labels[col]->setAutoFillBackground(true);
			m_table[paramIdx]->labels[col]->setPalette(qApp->palette());
			m_table[paramIdx]->labels[col]->setBackgroundRole(bgRole);
		}
		m_table[paramIdx]->head->setBackgroundRole(bgRole);
		m_table[paramIdx]->bars->setBackgroundRole(bgRole);
		for (int barIdx = 0; barIdx < m_table[paramIdx]->out.size(); ++barIdx)
		{
			m_table[paramIdx]->out[barIdx]->setBackgroundRole(bgRole);
			m_table[paramIdx]->par[barIdx]->setBackgroundRole(bgRole);
		}
	}
	emit parameterChanged();
}

void getParamMaxMinDiffVal(QVector<double> const & d, double & maxVal, double & minValDiff)
{
	maxVal = std::numeric_limits<double>::lowest();
	minValDiff = std::numeric_limits<double>::max();
	for (int paramIdx = 0; paramIdx < d.size(); ++paramIdx)
	{
		if (d[paramIdx] > maxVal)
		{
			maxVal = d[paramIdx];
		}
		for (int p2 = paramIdx + 1; p2 < d.size(); ++p2)
		{
			auto curDiff = std::abs(d[paramIdx] - d[p2]);
			if (curDiff < minValDiff)
			{
				minValDiff = curDiff;
			}
		}
	}
}

void iAParameterInfluenceView::updateStackedBars()
{
	for (auto col : m_visibleCharacts)
	{
		auto const& d =
		 ((col.first == outCharacteristic)  ? m_data->aggregatedSensitivities[col.second][m_measureIdx] :
		     ((col.first == outFiberCount)  ? m_data->aggregatedSensitivitiesFiberCount
		 /*(col.first == outDissimilarity)*/: m_data->aggregatedSensDissim[col.second]))[m_aggrType];
		// TODO: unify with addStackedBar
		auto title(columnName(col.first, col.second));
		double maxVal, minValDiff;
		getParamMaxMinDiffVal(d, maxVal, minValDiff);
		for (int paramIdx = 0; paramIdx < m_data->m_variedParams.size(); ++paramIdx)
		{
			m_table[paramIdx]->bars->updateBar(title, d[paramIdx], maxVal, minValDiff);
			updateStackedBarHistogram(title, paramIdx, col.first, col.second);
		}
	}
	for (int paramIdx = 0; paramIdx < m_data->m_variedParams.size(); ++paramIdx)
	{
		m_table[paramIdx]->bars->update();
	}
	updateChartY();
}

QString iAParameterInfluenceView::columnName(int outType, int outIdx) const
{
	return  +
		(outType == outCharacteristic) ? QString("Variation ") + m_data->charactName(outIdx) :
		((outType == outFiberCount) ? "Variation Fiber Count"
		/*(outType == outDissimilarity)*/	:
			getAvailableDissimilarityMeasureNames()[m_data->m_resultDissimMeasures[outIdx].first]);
}

QSharedPointer<iAPlot> iAParameterInfluenceView::createHistoPlot(QSharedPointer<iAHistogramData> histoData, QColor color)
{	// m_histogramChartType values need to match values from SensitivitySettings.ui file
	if (m_histogramChartType == "Bars")
	{
		QColor c(color);
		c.setAlpha(64);		// we need a transparent color for bars
		return QSharedPointer<iABarGraphPlot>::create(histoData, c);
	}
	else if (m_histogramChartType == "Lines")
	{
		return QSharedPointer<iALinePlot>::create(histoData, color);
	}
	else
	{
		LOG(lvlWarn, QString("Unknown chart type '%1'!").arg(m_histogramChartType));
		return QSharedPointer<iAPlot>();
	}
}

void iAParameterInfluenceView::updateStackedBarHistogram(QString const & barName, int paramIdx, int outType, int outIdx)
{
	int barIdx = m_table[paramIdx]->bars->barIndex(barName);
	auto outChart = m_table[paramIdx]->out[barIdx];
	outChart->clearPlots();
	outChart->resetYBounds();
	double rng[2];
	if (outType == outCharacteristic)
	{
		auto r = m_data->m_data->spmData->paramRange(m_data->m_charSelected[outIdx]);
		rng[0] = r[0];
		rng[1] = r[1];
	}
	else if (outType == outFiberCount)
	{
		rng[0] = m_data->m_fiberCountRange[0];
		rng[1] = m_data->m_fiberCountRange[1];
	}
	else /* outType == outDissimilarity */
	{
		rng[0] = m_data->m_dissimRanges[outIdx].first;
		rng[1] = m_data->m_dissimRanges[outIdx].second;
	}
	if (outType == outCharacteristic)
	{
		auto varHistData = iAHistogramData::create(barName, iAValueType::Continuous, rng[0], rng[1],
			m_data->charHistVarAgg[outIdx][m_aggrType][paramIdx]);
		outChart->addPlot(createHistoPlot(varHistData, VariationHistogramColor));
	}
	auto avgHistData = iAHistogramData::create("Average", iAValueType::Continuous, rng[0], rng[1],
		(outType == outCharacteristic)
			? m_data->charHistAvg[outIdx]
			: (outType == outFiberCount)
				? m_data->fiberCountHistogram
				: /* outType == outDissimilarity */ m_data->m_dissimHistograms[outIdx]);
	outChart->addPlot(createHistoPlot(avgHistData, AverageHistogramColor));
	auto selectedResults = m_viewState->selectedResults();
	for (size_t i=0; i<selectedResults.size(); ++i)
	{
		auto resultIdx = selectedResults[i];
		addResultHistoPlot(resultIdx, paramIdx, barIdx, m_viewState->selectedResultColorTheme()->color(i));
	}
	outChart->update();

	auto parChart = m_table[paramIdx]->par[barIdx];
	parChart->clearPlots();
	auto const& d = ((outType == outCharacteristic)
		? m_data->sensitivityField[outIdx][m_measureIdx][m_aggrType]
		: (outType == outFiberCount)
			? m_data->sensitivityFiberCount[m_aggrType]
			: /* (outType == outDissimilarity)*/ m_data->sensDissimField[outIdx][m_aggrType])[paramIdx];
	auto plotData = iAXYPlotData::create("Sensitivity " + columnName(outType, outIdx), iAValueType::Continuous, d.size());
	for (int i = 0; i < d.size(); ++i)
	{
		plotData->addValue(m_data->paramSetValues[i][m_data->m_variedParams[paramIdx]], d[i]);
	}
	parChart->resetYBounds();
	parChart->addPlot(QSharedPointer<iALinePlot>::create(plotData, ParamSensitivityPlotColor));
	parChart->update();
}

void iAParameterInfluenceView::setHistogramChartType(QString const & chartType)
{
	m_histogramChartType = chartType;
	updateStackedBars();
}

QVector<int> const& iAParameterInfluenceView::paramIndicesSorted() const
{
	return m_sort;
}

void iAParameterInfluenceView::updateChartY()
{
	auto getChartMax = [](QVector<iAChartWidget*> const& c, double yMax) {
		for (auto chart : c)
		{
			yMax = std::max(yMax, chart->yBounds()[1]);
		}
		return yMax;
	};
	auto setChartYBounds = [](QVector<iAChartWidget*> const& c, double yMax) {
		for (auto chart : c)
		{
			chart->setYBounds(0, yMax);
			chart->update();
		}
	};
	double yMaxOut = std::numeric_limits<double>::lowest(),
	       yMaxPar = std::numeric_limits<double>::lowest();
	for (auto chartRow : m_table)
	{
		yMaxOut = getChartMax(chartRow->out, yMaxOut);
		yMaxPar = getChartMax(chartRow->par, yMaxPar);
	}
	for (auto chartRow : m_table)
	{
		setChartYBounds(chartRow->out, yMaxOut);
		setChartYBounds(chartRow->par, yMaxPar);
	}
}

void iAParameterInfluenceView::setActionChecked(int outType, int outIdx, bool checked)
{
	for (auto action : m_table[0]->head->contextMenu()->actions())
	{
		if (action->property("objectType").toInt() == outType && action->property("charactIdx").toInt() == outIdx)
		{
			QSignalBlocker b(action);
			action->setChecked(checked);
		}
	}
}

void iAParameterInfluenceView::addStackedBar(int outType, int outIdx)
{
	setActionChecked(outType, outIdx, true);
	m_visibleCharacts.push_back(qMakePair(outType, outIdx));
	auto title(columnName(outType, outIdx));
	LOG(lvlDebug, QString("Showing stacked bar for characteristic %1").arg(title));
	auto const& d = (
		   (outType == outCharacteristic)  ? m_data->aggregatedSensitivities[outIdx][m_measureIdx]:
		      ((outType == outFiberCount)  ? m_data->aggregatedSensitivitiesFiberCount
		/*(col.first == outDissimilarity)*/: m_data->aggregatedSensDissim[outIdx]))[m_aggrType];

	int curBarIdx = static_cast<int>(m_table[0]->head->numberOfBars());  // not yet added to bars here so no -1
	auto params = m_data->m_variedParams;

	// add the two charts that can be shown inside the matrix cells:
	auto selectedResults = m_viewState->selectedResults();
	for (int paramIdx = 0; paramIdx < params.size(); ++paramIdx)
	{
		int varParIdx = m_data->m_variedParams[paramIdx];
		auto paramName = m_data->m_paramNames[varParIdx];
		//QColor color = (paramIdx == m_selectedParam) ? ParamRowSelectedBGColor : ParamRowUnselectedBGColor;
		QPalette::ColorRole bgRole = (paramIdx == m_selectedParam) ? QPalette::AlternateBase : QPalette::Window;

		auto outChart = new iAChartWidget(this, "", (curBarIdx == 0) ? "Var. from " + paramName : "");
		outChart->showLegend(true);
		outChart->showXAxisLabel(false);
		outChart->setEmptyText("");
		outChart->setBackgroundRole(bgRole);
		//outChart->setMinimumHeight(80);
		m_table[paramIdx]->out.push_back(outChart);
		//connect(outChart, &iAChartWidget::clicked, this, &iAParameterInfluenceView::);
		connect(outChart, &iAChartWidget::axisChanged, this, &iAParameterInfluenceView::charactChartAxisChanged);

		auto parChart = new iAChartWidget(this, paramName, (curBarIdx == 0) ? "Sens. " + title : "");
		parChart->setEmptyText("");
		parChart->setBackgroundRole(bgRole);
		parChart->setProperty("paramIdx", paramIdx);
		double parMin = m_data->m_paramMin[varParIdx],
			parMax = m_data->m_paramMax[varParIdx],
			parPad = (parMax - parMin) / 100.0; // add 1% of range on both sides to make sure all markers will be visible
		parChart->setXBounds(parMin - parPad, parMax + parPad);
		m_table[paramIdx]->par.push_back(parChart);
		connect(parChart, &iAChartWidget::clicked, this, &iAParameterInfluenceView::paramChartClicked);
		connect(parChart, &iAChartWidget::axisChanged, this, &iAParameterInfluenceView::paramChartAxisChanged);
		//parChart->setMinimumHeight(80);
		for (size_t i=0; i<selectedResults.size(); ++i)
		{
			auto resultIdx = selectedResults[i];
			double paramValue = m_data->m_paramValues[m_data->m_variedParams[paramIdx]][resultIdx];
			parChart->setXMarker(paramValue, m_viewState->selectedResultColorTheme()->color(i), Qt::DashLine);
		}
	}
	addTableWidgets();
	double maxVal, minValDiff;
	getParamMaxMinDiffVal(d, maxVal, minValDiff);
	for (int paramIdx = 0; paramIdx < m_data->m_variedParams.size(); ++paramIdx)
	{
		m_table[paramIdx]->head->addBar(title, 1, 1, 1);
		m_table[paramIdx]->bars->addBar(title, d[paramIdx], maxVal, minValDiff);
		updateStackedBarHistogram(title, paramIdx, outType, outIdx);
		m_table[paramIdx]->head->setLeftMargin(m_table[paramIdx]->out[0]->leftMargin());
		m_table[paramIdx]->bars->setLeftMargin(m_table[paramIdx]->out[0]->leftMargin());
	}
	updateChartY();
	emit barAdded(outType, outIdx);
}

// TODO: avoid duplication:

void iAParameterInfluenceView::paramChartAxisChanged()
{
	auto inChart = qobject_cast<iAChartWidget*>(QObject::sender());
	for (int rowIdx = 0; rowIdx < m_table.size(); ++rowIdx)
	{
		for (int i = 0; i < m_table[rowIdx]->par.size(); ++i)
		{
			auto chart = m_table[rowIdx]->par[i];
			if (chart == inChart)
			{
				continue;
			}
			chart->setXZoom(inChart->xZoom());
			chart->setXShift(inChart->xShift());
			chart->setYZoom(inChart->yZoom());
			chart->update();
		}
	}
}

void iAParameterInfluenceView::charactChartAxisChanged()
{
	auto inChart = qobject_cast<iAChartWidget*>(QObject::sender());
	for (int rowIdx = 0; rowIdx < m_table.size(); ++rowIdx)
	{
		for (int i = 0; i < m_table[rowIdx]->out.size(); ++i)
		{
			auto chart = m_table[rowIdx]->out[i];
			if (chart == inChart)
			{
				continue;
			}
			chart->setXZoom(inChart->xZoom());
			chart->setXShift(inChart->xShift());
			chart->setYZoom(inChart->yZoom());
			chart->update();
		}
	}
}


void iAParameterInfluenceView::removeStackedBar(int outType, int outIdx)
{
	setActionChecked(outType, outIdx, false);
	int visibleIdx = m_visibleCharacts.indexOf(qMakePair(outType, outIdx));
	if (visibleIdx == -1)
	{
		LOG(lvlError, QString("Invalid state - called remove on non-added bar (outType=%1, outIdx=%2").arg(outType).arg(outIdx));
	}
	m_visibleCharacts.remove(visibleIdx);
	auto title(columnName(outType, outIdx));
	LOG(lvlDebug, QString("Removing stacked bar for characteristic %1").arg(title));
	for (int rowIdx = 0; rowIdx < m_table.size(); ++rowIdx)
	{
		m_table[rowIdx]->head->removeBar(title);
		int barIdx = m_table[rowIdx]->bars->removeBar(title);
		auto ow = m_table[rowIdx]->out[barIdx];
		m_table[rowIdx]->out.remove(barIdx);
		delete ow;
		auto pw = m_table[rowIdx]->par[barIdx];
		m_table[rowIdx]->par.remove(barIdx);
		delete pw;
		auto paramName = m_data->m_paramNames[m_data->m_variedParams[m_sort[rowIdx]]];
		int newNumBars = m_table[rowIdx]->bars->numberOfBars();
		if (newNumBars > 0)
		{
			m_table[rowIdx]->out[0]->setYCaption(
				"Var. from " + paramName);  // to make sure if first chart is removed that new first gets caption
			m_table[rowIdx]->out[0]->setYCaption("Sens. " + m_table[rowIdx]->bars->barName(0));
			for (int i = barIdx; i < newNumBars; ++i)
			{  // addWidget automatically removes it from position where it was before
				m_paramListLayout->addWidget(
					m_table[rowIdx]->out[i], 1 + RowsPerParam * rowIdx + RowOutputChart, colStackedBar + i);
				m_paramListLayout->addWidget(
					m_table[rowIdx]->par[i], 1 + RowsPerParam * rowIdx + RowParamChart, colStackedBar + i);
			}
			for (int b = 0; b < newNumBars; ++b)
			{
				m_table[rowIdx]->out[b]->resetYBounds();
			}
		}
	}
	updateChartY();
	emit barRemoved(outType, outIdx);
}
