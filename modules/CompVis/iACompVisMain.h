#pragma once

#include "mainwindow.h"

#include "iACsvDataStorage.h"
#include "dlg_VisMainWindow.h"
#include "iACompHistogramTableData.h"
//#include "iACorrelationCoefficient.h"
#include "vtkSmartPointer.h"

//QT
#include"qlist.h"

class iAMultidimensionalScaling;
class iACoefficientOfVariation;
class iACorrelationCoefficient;

class iACompBarChart;
class iACompHistogramTable;
class iACompBoxPlot;
class iACompCorrelationMap;
class iAHistogramData;


class iACompVisMain
{
   public:
	iACompVisMain(MainWindow* mainWin);

	//load the CSV datasets
	void loadData();

	void orderHistogramTableAscending();
	void orderHistogramTableDescending();
	void orderHistogramTableAsLoaded();

	//update all charts according to the Histogram Table selection
	//zoomedRowData stores bin data of selected rows that will be zoomed.
	//Each entry in the list represents a row, where any cell(or several) were selected.
	//The first entry is the most upper row that was selected, the ordering is then descending.
	//each entry has as many bins as cells were selected for this row
	void updateOtherCharts(csvDataType::ArrayType* selectedData, std::map<int, std::vector<double>>* pickStatistic);
	//update BarChart  according to the Histogram Table selection
	void updateBarChart(csvDataType::ArrayType* selectedData);
	void updateBoxPlot(csvDataType::ArrayType* selectedData);
	void updateCorrelationMap(csvDataType::ArrayType* selectedData, std::map<int, std::vector<double>>* pickStatistic);

	void resetOtherCharts();
	void resetBarChart();
	void resetBoxPlot();
	void resetCorrelationMap();
	
   private:
	void initializeMDS();
	void initializeVariationCoefficient();
	void initializeCorrelationCoefficient();


	dlg_VisMainWindow* m_mainW;
	iACsvDataStorage* m_dataStorage;

	iAMultidimensionalScaling* m_mds;
	iACoefficientOfVariation* m_cofVar;
	iACorrelationCoefficient* m_corCoeff;
	
	iAHistogramData* m_histData;

	iACompBarChart* m_BarChartDockWidget;
	iACompHistogramTable* m_HistogramTableDockWidget;
	iACompBoxPlot* m_BoxPlotDockWidget;
	iACompCorrelationMap* m_CorrelationMapDockWidget;
};