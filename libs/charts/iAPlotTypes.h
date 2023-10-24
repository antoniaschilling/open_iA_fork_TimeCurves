// Copyright 2016-2023, the open_iA contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "iAPlot.h"

#include "iAcharts_export.h"

#include <QVector>
#include <QSharedPointer>
#include <QRectF>

class iALookupTable;
class iAMapper;
class iAPlotData;

class QPolygon;

//! Plot highlighting a single bin in a histogram plot.
class iAcharts_API iASelectedBinPlot : public iAPlot
{
public:
	iASelectedBinPlot(QSharedPointer<iAPlotData> proxyData, size_t idx = 0, QColor const & color = Qt::red );
	void setSelectedBin(size_t idx);
	void draw(QPainter& painter, size_t startIdx, size_t endIdx, iAMapper const & xMapper, iAMapper const & yMapper) const override;
private:
	size_t m_idx;
};

//! Plots the given data as a line.
//! Well-suited for (pseudo-)continuous data.
//! For filling the area under the curve, see the iAFilledLinePlot.
class iAcharts_API iALinePlot: public iAPlot
{
public:
	iALinePlot(QSharedPointer<iAPlotData> data, QColor const& color);
	void setLineWidth(int width);
private:
	int m_lineWidth;
	void draw(QPainter& painter, size_t startIdx, size_t endIdx, iAMapper const& xMapper, iAMapper const& yMapper) const override;
};

//! Plots each data point as a rectangular bar, all the bars are horizontally connected via a line.
//! Well suited for binned, e.g. histogram data.
class iAcharts_API iAStepFunctionPlot : public iAPlot
{
public:
	iAStepFunctionPlot(QSharedPointer<iAPlotData> data, QColor const & color);
private:
	void draw(QPainter& painter, size_t startIdx, size_t endIdx, iAMapper const & xMapper, iAMapper const & yMapper) const override;
	void drawLegendItem(QPainter& painter, QRect const& rect) override;
	QColor getFillColor() const;
};

//! Plots the given data as a line and fills the area below the line.
//! Well-suited for (pseudo-)continuous data.
//! For a plot without filling the area under the curve, see the iALinePlot.
class iAcharts_API iAFilledLinePlot : public iAPlot
{
public:
	iAFilledLinePlot(QSharedPointer<iAPlotData> data, QColor const & color);
private:
	void draw(QPainter& painter, size_t startIdx, size_t endIdx, iAMapper const & xMapper, iAMapper const & yMapper) const override;
	void drawLegendItem(QPainter& painter, QRect const& rect) override;
	QColor getFillColor() const;
};

//! Plots the given data points as single bars, horizontally separated by the given margin.
//! Well-suited for discrete or binned data.
class iAcharts_API iABarGraphPlot: public iAPlot
{
public:
	iABarGraphPlot(QSharedPointer<iAPlotData> data, QColor const& color, int margin = 0);
	void draw(QPainter& painter, size_t startIdx, size_t endIdx, iAMapper const& xMapper, iAMapper const& yMapper) const override;
	void drawLegendItem(QPainter& painter, QRect const& rect) override;
	void setLookupTable(QSharedPointer<iALookupTable> lut);
private:
	QSharedPointer<iALookupTable> m_lut;
	int m_margin;
};

//! Collects multiple plots and makes them act as a single plot.
class iAcharts_API iAPlotCollection: public iAPlot
{
public:
	iAPlotCollection();
	void draw(QPainter& painter, size_t startIdx, size_t endIdx, iAMapper const& xMapper, iAMapper const& yMapper) const override;
	void add (QSharedPointer<iAPlot> plot);
	void clear();
	void setColor(QColor const & color) override;
	QSharedPointer<iAPlotData> data() override;
private:
	QVector<QSharedPointer<iAPlot> > m_drawers;
};

class iAcharts_API iASplinePlot : public iAPlot
{
public:
	iASplinePlot(QSharedPointer<iAPlotData> data, QColor const& color);
	void setLineWidth(int width);
	void setPointSize(int size);
	QRectF getBoundingBox(iAMapper const& xMapper, iAMapper const& yMapper) const;

private:
	int m_lineWidth;
	int m_pointSize;
	QVector<QPointF>* m_points;
	//todo smoothness parameter
	void draw(QPainter& painter, size_t startIdx, size_t endIdx, iAMapper const& xMapper,
		iAMapper const& yMapper) const override;
};