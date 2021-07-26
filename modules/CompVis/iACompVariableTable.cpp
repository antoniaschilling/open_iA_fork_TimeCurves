#include "iACompVariableTable.h"

//CompVis
#include "iACompHistogramVis.h"


//vtk
#include "vtkCamera.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkScalarBarActor.h"
#include "vtkRenderer.h"
#include "vtkTextActor.h"
#include "vtkTextProperty.h"
#include "vtkLookupTable.h"
#include "vtkActor.h"
#include "vtkColorTransferFunction.h"
#include "vtkProgrammableGlyphFilter.h"
#include "vtkPlaneSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkCellData.h"
#include "vtkProperty.h"

iACompVariableTable::iACompVariableTable(iACompHistogramVis* vis, iACompBayesianBlocksData* bayesianBlocksData, iACompNaturalBreaksData* naturalBreaksData):
	iACompTable(vis),
	m_interactionStyle(vtkSmartPointer<iACompVariableTableInteractorStyle>::New()),
	m_bbData(bayesianBlocksData), 
	m_nbData(naturalBreaksData),
	m_originalPlanes(new std::vector<vtkSmartPointer<vtkPlaneSource>>())
{
	m_activeData = m_nbData;
	
	//initialize interaction
	initializeInteraction();

	//initialize visualization
	initializeTable();
}

void iACompVariableTable::setActive()
{
	//add rendererColorLegend to widget
	addLegendRendererToWidget();


	if (m_lastState == iACompVisOptions::lastState::Undefined)
	{
		drawHistogramTable();

		//init camera
		m_mainRenderer->SetActiveCamera(m_vis->getCamera());
		renderWidget();

		m_lastState = iACompVisOptions::lastState::Defined;
	}
	else if (m_lastState == iACompVisOptions::lastState::Defined)
	{
		reinitalizeState();

		drawHistogramTable();
		renderWidget();
	}
}

void iACompVariableTable::setInactive()
{
	m_mainRenderer->RemoveAllViewProps();
}

void iACompVariableTable::setActiveBinning(iACompVisOptions::binningType binningType)
{
	if (binningType == iACompVisOptions::binningType::BayesianBlocks)
	{
		m_activeData = m_bbData;
	}
	else if (binningType == iACompVisOptions::binningType::JenksNaturalBreaks)
	{
		m_activeData = m_nbData;
	}
	else
	{
		m_activeData = m_bbData;
	}
}

void iACompVariableTable::initializeCamera()
{
	m_mainRenderer->SetActiveCamera(m_vis->getCamera());
}

/****************************************** Initialization **********************************************/
void iACompVariableTable::initializeTable()
{
	//setup color table
	makeLUTFromCTF();
	makeLUTDarker();

	//initialize legend
	initializeLegend();

	//init camera
	initializeCamera();	
}

void iACompVariableTable::initializeInteraction()
{
	m_interactionStyle->setVariableTableVisualization(this);
	m_interactionStyle->SetDefaultRenderer(m_mainRenderer);
	m_interactionStyle->setIACompHistogramVis(m_vis);
}

void iACompVariableTable::makeLUTFromCTF()
{
	vtkSmartPointer<vtkColorTransferFunction> ctf = vtkSmartPointer<vtkColorTransferFunction>::New();
	ctf->SetColorSpaceToRGB();

	QColor c1 = QColor(103, 21, 45);
	QColor c2 = QColor(128, 0, 38);
	QColor c3 = QColor(189, 0, 38);
	QColor c4 = QColor(227, 26, 28);
	QColor c5 = QColor(252, 78, 42);
	QColor c6 = QColor(253, 141, 60);
	QColor c7 = QColor(254, 178, 76);
	QColor c8 = QColor(254, 217, 118);
	QColor c9 = QColor(255, 237, 160);
	QColor c10 = QColor(255, 255, 204);

	ctf->AddRGBPoint(1.0, c1.redF(), c1.greenF(), c1.blueF());
	ctf->AddRGBPoint(0.9, c1.redF(), c1.greenF(), c1.blueF());
	ctf->AddRGBPoint(0.8, c2.redF(), c2.greenF(), c2.blueF());
	ctf->AddRGBPoint(0.7, c3.redF(), c3.greenF(), c3.blueF());
	ctf->AddRGBPoint(0.6, c4.redF(), c4.greenF(), c4.blueF());
	ctf->AddRGBPoint(0.5, c5.redF(), c5.greenF(), c5.blueF());
	ctf->AddRGBPoint(0.4, c6.redF(), c6.greenF(), c6.blueF());
	ctf->AddRGBPoint(0.3, c7.redF(), c7.greenF(), c7.blueF());
	ctf->AddRGBPoint(0.2, c8.redF(), c8.greenF(), c8.blueF());
	ctf->AddRGBPoint(0.1, c9.redF(), c9.greenF(), c9.blueF());
	ctf->AddRGBPoint(0.0, c10.redF(), c10.greenF(), c10.blueF());

	m_lut->SetNumberOfTableValues(m_tableSize);
	m_lut->Build();

	double min = 0;
	double max = 0;
	int startVal = 1;

	double binRange = calculateUniformBinRange();

	for (size_t i = 0; i < m_tableSize; i++)
	{
		double* rgb;
		rgb = ctf->GetColor(static_cast<double>(i) / (double)m_tableSize);
		m_lut->SetTableValue(i, rgb);

		//make format of annotations
		double low = round_up(startVal + (i * binRange), 2);
		double high = round_up(startVal + ((i + 1) * binRange), 2);

		std::string sLow = std::to_string(low);
		std::string sHigh = std::to_string(high);

		std::string lowerString = initializeLegendLabels(sLow);
		std::string upperString = initializeLegendLabels(sHigh);

		//position description in the middle of each color bar in the scalarBar legend
		//m_lut->SetAnnotation(low + ((high - low) * 0.5), lowerString + " - " + upperString);
		m_lut->SetAnnotation(low + ((high - low) * 0.5), lowerString);

		//store min and max value of the dataset
		if (i == 0)
		{
			min = low;
		}
		else if (i == m_tableSize - 1)
		{
			max = high;
		}
	}

	m_lut->SetTableRange(min, max);

	/*double rgbMin[3];
	m_lut->GetColor(min, rgbMin);
	double rgbMax[3];
	m_lut->GetColor(max, rgbMax);
	LOG(lvlDebug,
		" LUT min = " + QString::number(min) + " with rgb = " + QString::number(rgbMin[0] * 255) + ", " +
			QString::number(rgbMin[1] * 255) + ", " + QString::number(rgbMin[2] * 255));
	LOG(lvlDebug,
		" LUT max = " + QString::number(max) + " with rgb = " + QString::number(rgbMax[0] * 255) + ", " +
			QString::number(rgbMax[1] * 255) + ", " + QString::number(rgbMax[2] * 255));
	*/

	double col[3];
	col[0] = iACompVisOptions::getDoubleArray(iACompVisOptions::BACKGROUNDCOLOR_GREY)[0];
	col[1] = iACompVisOptions::getDoubleArray(iACompVisOptions::BACKGROUNDCOLOR_GREY)[1];
	col[2] = iACompVisOptions::getDoubleArray(iACompVisOptions::BACKGROUNDCOLOR_GREY)[2];
	m_lut->SetBelowRangeColor(col[0], col[1], col[2], 1);
	m_lut->UseBelowRangeColorOn();

	double* colAbove = ctf->GetColor(1);
	m_lut->SetAboveRangeColor(colAbove[0], colAbove[1], colAbove[2], 1);
	m_lut->UseAboveRangeColorOn();
}

void iACompVariableTable::makeLUTDarker()
{
	vtkSmartPointer<vtkColorTransferFunction> ctf = vtkSmartPointer<vtkColorTransferFunction>::New();
	ctf->SetColorSpaceToRGB();

	QColor c1 = QColor(51, 10, 23);
	QColor c2 = QColor(64, 0, 19);
	QColor c3 = QColor(64, 0, 19);
	QColor c4 = QColor(113, 13, 14);
	QColor c5 = QColor(126, 39, 21);
	QColor c6 = QColor(126, 70, 30);
	QColor c7 = QColor(127, 89, 38);
	QColor c8 = QColor(127, 108, 59);
	QColor c9 = QColor(127, 118, 80);
	QColor c10 = QColor(127, 127, 102);

	ctf->AddRGBPoint(1.0, c1.redF(), c1.greenF(), c1.blueF());
	ctf->AddRGBPoint(0.9, c1.redF(), c1.greenF(), c1.blueF());
	ctf->AddRGBPoint(0.8, c2.redF(), c2.greenF(), c2.blueF());
	ctf->AddRGBPoint(0.7, c3.redF(), c3.greenF(), c3.blueF());
	ctf->AddRGBPoint(0.6, c4.redF(), c4.greenF(), c4.blueF());
	ctf->AddRGBPoint(0.5, c5.redF(), c5.greenF(), c5.blueF());
	ctf->AddRGBPoint(0.4, c6.redF(), c6.greenF(), c6.blueF());
	ctf->AddRGBPoint(0.3, c7.redF(), c7.greenF(), c7.blueF());
	ctf->AddRGBPoint(0.2, c8.redF(), c8.greenF(), c8.blueF());
	ctf->AddRGBPoint(0.1, c9.redF(), c9.greenF(), c9.blueF());
	ctf->AddRGBPoint(0.0, c10.redF(), c10.greenF(), c10.blueF());

	m_lutDarker->SetNumberOfTableValues(m_tableSize);
	m_lutDarker->Build();

	double min = 0;
	double max = 0;
	int startVal = 1;

	double binRange = calculateUniformBinRange();

	for (size_t i = 0; i < m_tableSize; i++)
	{
		double* rgb;
		rgb = ctf->GetColor(static_cast<double>(i) / (double)m_tableSize);
		m_lutDarker->SetTableValue(i, rgb);

		//make format of annotations
		double low = round_up(startVal + (i * binRange), 2);
		double high = round_up(startVal + ((i + 1) * binRange), 2);

		std::string sLow = std::to_string(low);
		std::string sHigh = std::to_string(high);

		std::string lowerString = initializeLegendLabels(sLow);
		std::string upperString = initializeLegendLabels(sHigh);

		//position description in the middle of each color bar in the scalarBar legend
		//m_lut->SetAnnotation(low + ((high - low) * 0.5), lowerString + " - " + upperString);
		m_lutDarker->SetAnnotation(low + ((high - low) * 0.5), lowerString);

		//store min and max value of the dataset
		if (i == 0)
		{
			min = low;
		}
		else if (i == m_tableSize - 1)
		{
			max = high;
		}
	}

	m_lutDarker->SetTableRange(min, max);

	/*double rgbMin[3];
	m_lut->GetColor(min, rgbMin);
	double rgbMax[3];
	m_lut->GetColor(max, rgbMax);
	LOG(lvlDebug,
		" LUT min = " + QString::number(min) + " with rgb = " + QString::number(rgbMin[0] * 255) + ", " +
			QString::number(rgbMin[1] * 255) + ", " + QString::number(rgbMin[2] * 255));
	LOG(lvlDebug,
		" LUT max = " + QString::number(max) + " with rgb = " + QString::number(rgbMax[0] * 255) + ", " +
			QString::number(rgbMax[1] * 255) + ", " + QString::number(rgbMax[2] * 255));
	*/

	double col[3];
	col[0] = iACompVisOptions::getDoubleArray(iACompVisOptions::BACKGROUNDCOLOR_GREY)[0];
	col[1] = iACompVisOptions::getDoubleArray(iACompVisOptions::BACKGROUNDCOLOR_GREY)[1];
	col[2] = iACompVisOptions::getDoubleArray(iACompVisOptions::BACKGROUNDCOLOR_GREY)[2];
	m_lutDarker->SetBelowRangeColor(col[0], col[1], col[2], 1);
	m_lutDarker->UseBelowRangeColorOn();

	double* colAbove = ctf->GetColor(1);
	m_lutDarker->SetAboveRangeColor(colAbove[0], colAbove[1], colAbove[2], 1);
	m_lutDarker->UseAboveRangeColorOn();
}

double iACompVariableTable::calculateUniformBinRange()
{
	int maxAmountInAllBins = m_activeData->getMaxAmountInAllBins();
	return ((double)maxAmountInAllBins) / ((double)m_tableSize);
}

void iACompVariableTable::calculateBinRange()
{
}

/****************************************** Getter & Setter **********************************************/
vtkSmartPointer<iACompVariableTableInteractorStyle> iACompVariableTable::getInteractorStyle()
{
	return m_interactionStyle;
}

/****************************************** Rendering **********************************************/
void iACompVariableTable::drawHistogramTable()
{
	m_originalPlanes->clear();
	if (m_mainRenderer->GetViewProps()->GetNumberOfItems() > 0)
	{
		m_mainRenderer->RemoveAllViewProps();
	}

	m_vis->calculateRowWidthAndHeight(m_vis->getWindowWidth(), m_vis->getWindowHeight(), m_vis->getAmountDatasets());

	//draw cells from bottom to top --> so start with last dataset and go to first
	for (int currCol = 0; currCol < m_vis->getAmountDatasets(); currCol++)
	{
		int dataInd = m_vis->getOrderOfIndicesDatasets()->at(currCol);
		drawRow(dataInd, currCol, 0);
	}

	renderWidget();
}

void iACompVariableTable::drawRow(int currDataInd, int currentColumn, double offset)
{
	bin::BinType* currDataset = m_activeData->getBinData()->at(currDataInd);
	double numberOfBins = currDataset->size();

	//each row consists of a certain number of bins and each bin will be drawn as glyph
	vtkSmartPointer<vtkPoints> glyphPoints = vtkSmartPointer<vtkPoints>::New();
	glyphPoints->SetDataTypeToDouble();
	glyphPoints->SetNumberOfPoints(numberOfBins);

	vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
	polydata->SetPoints(glyphPoints);

	vtkSmartPointer<vtkDoubleArray> originArray = vtkSmartPointer<vtkDoubleArray>::New();
	originArray->SetName("originArray");
	originArray->SetNumberOfComponents(3);
	originArray->SetNumberOfTuples(numberOfBins);
	
	vtkSmartPointer<vtkDoubleArray> point1Array = vtkSmartPointer<vtkDoubleArray>::New();
	point1Array->SetName("point1Array");
	point1Array->SetNumberOfComponents(3);
	point1Array->SetNumberOfTuples(numberOfBins);

	vtkSmartPointer<vtkDoubleArray> point2Array = vtkSmartPointer<vtkDoubleArray>::New();
	point2Array->SetName("point2Array");
	point2Array->SetNumberOfComponents(3);
	point2Array->SetNumberOfTuples(numberOfBins);

	vtkSmartPointer<vtkUnsignedCharArray> colorArray = vtkSmartPointer<vtkUnsignedCharArray>::New();
	colorArray->SetName("colorArray");
	colorArray->SetNumberOfComponents(3);
	colorArray->SetNumberOfTuples(numberOfBins);

	constructBins(currDataset, originArray, point1Array, point2Array, colorArray, currentColumn, offset);

	polydata->GetPointData()->AddArray(originArray);
	polydata->GetPointData()->AddArray(point1Array);
	polydata->GetPointData()->AddArray(point2Array);
	polydata->GetCellData()->AddArray(colorArray); 
	polydata->GetCellData()->SetActiveScalars("colorArray");

	 vtkSmartPointer<vtkPlaneSource> planeSource = vtkSmartPointer<vtkPlaneSource>::New();
	planeSource->SetOutputPointsPrecision(vtkAlgorithm::DOUBLE_PRECISION);
	planeSource->SetCenter(0, 0, 0);
	planeSource->Update();

	vtkSmartPointer<vtkProgrammableGlyphFilter> glypher = vtkSmartPointer<vtkProgrammableGlyphFilter>::New();
	glypher->SetInputData(polydata);
	glypher->SetSourceData(planeSource->GetOutput());
	glypher->SetGlyphMethod(buildGlyphRepresentation, glypher);
	glypher->Update();

	vtkSmartPointer<vtkPolyDataMapper> glyphMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	glyphMapper->SetInputConnection(glypher->GetOutputPort());
	glyphMapper->SetColorModeToDefault();
	glyphMapper->SetScalarModeToUseCellData();
	glyphMapper->GetInput()->GetCellData()->SetScalars(colorArray);
	glyphMapper->ScalarVisibilityOn();

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(glyphMapper);
	if (!m_useDarkerLut)
	{  //the edges of the cells are drawn
		actor->GetProperty()->EdgeVisibilityOn();
		double col[3];
		col[0] = iACompVisOptions::getDoubleArray(iACompVisOptions::BACKGROUNDCOLOR_BLACK)[0];
		col[1] = iACompVisOptions::getDoubleArray(iACompVisOptions::BACKGROUNDCOLOR_BLACK)[1];
		col[2] = iACompVisOptions::getDoubleArray(iACompVisOptions::BACKGROUNDCOLOR_BLACK)[2];
		actor->GetProperty()->SetEdgeColor(col[0], col[1], col[2]);
		actor->GetProperty()->SetLineWidth(actor->GetProperty()->GetLineWidth() * 1.5);
	}
	else
	{  //not showing the edges of the cells
		actor->GetProperty()->EdgeVisibilityOff();
	}

	m_mainRenderer->AddActor(actor);

	//add name of dataset/row
	double y = (m_vis->getColSize() * currentColumn) + offset;
	double pos[3] = {-(m_vis->getRowSize()) * 0.05, y + (m_vis->getColSize() * 0.5), 0.0};
	addDatasetName(currDataInd, pos);
}

void iACompVariableTable::constructBins(
	bin::BinType* currRowData, 
	vtkSmartPointer<vtkDoubleArray> originArray,
	vtkSmartPointer<vtkDoubleArray> point1Array,
	vtkSmartPointer<vtkDoubleArray> point2Array, 
	vtkSmartPointer<vtkUnsignedCharArray> colorArray, 
	int currentColumn,
	double offset
	)
{
	int numberOfBins = currRowData->size();

	//drawing positions
	double min_x = 0.0;
	double max_x = m_vis->getRowSize();
	double min_y = (m_vis->getColSize() * currentColumn) + offset;
	double max_y = min_y + m_vis->getColSize();

	//store plane representing a dataset for later use
	vtkSmartPointer<vtkPlaneSource> currentPlane = vtkSmartPointer<vtkPlaneSource>::New();
	currentPlane->SetXResolution(1);
	currentPlane->SetYResolution(1);
	currentPlane->SetOrigin(min_x, min_y, 0);
	currentPlane->SetPoint1(max_x, min_y, 0);
	currentPlane->SetPoint2(min_x, max_y, 0);
	currentPlane->Update();
	m_originalPlanes->push_back(currentPlane);

	//xOffset is added to prevent bins from overlapping
	double xOffset = 0.0;  //m_vis->getRowSize() * 0.5; //0.05
	
	double intervalStart = 0.0;
	for (int i = 0; i < numberOfBins; i++)
	{ 
		double minVal = m_activeData->getMinVal();
		double maxVal = m_activeData->getMaxVal();
		double currVal;
		if ( i < numberOfBins-1)
		{
			if (currRowData->at(1 + i).size() >= 1)
			{
				currVal = currRowData->at(1 + i).at(0);

				if (currVal == m_activeData->getMaxVal())
					currVal = currVal * 0.99;
			}
			else
			{
				currVal = m_activeData->getMaxVal() * 0.99;
			}
		}
		else
		{
			currVal = m_activeData->getMaxVal();
		}
		
		double percent = iACompVisOptions::calculatePercentofRange(currVal, minVal, maxVal);

		//calculate min & max position for each bin
		double posXMin = intervalStart;
		double posXMax = iACompVisOptions::calculateValueAccordingToPercent(min_x, max_x, percent);

		originArray->InsertTuple3(i, posXMin, min_y, 0.0);
		point1Array->InsertTuple3(i, posXMax + xOffset, min_y, 0.0);  //width
		point2Array->InsertTuple3(i, posXMin, max_y, 0.0); // height
		
		/////////////////
		//// DEBUG INFO
		//// Check input
		//double v1[3], v2[3], normal[3];
		//double origin[3] = {posXMin, min_y, 0.0};
		//double point1[3] = {posXMax , min_y, 0.0};
		//double point2[3] = {posXMin, max_y, 0.0};

		//for (int k = 0; k < 3; k++)
		//{
		//	v1[k] = point1[k] - origin[k];
		//	v2[k] = point2[k] - origin[k];
		//}
		//
		//vtkMath::Cross(v1, v2, normal);
		//if (vtkMath::Normalize(normal) == 0.0)
		//{
		//	LOG(lvlDebug,
		//		"NOT Working : " + QString::number(normal[0]) + ", " + QString::number(normal[1]) + ", " +
		//			QString::number(normal[2]) + ") ");
		//}
		/////////////////
		
		intervalStart = posXMax + xOffset;

		//calculate color for each bin
		double numberOfObjects = currRowData->at(i).size();
		double* rgbBorder;
		double rgb[3] = {0.0, 0.0 ,0.0};
		double maxNumber = m_lut->GetRange()[1];
		double minNumber = m_lut->GetRange()[0];
		
		if (numberOfObjects > maxNumber)
		{
			if (m_useDarkerLut)
			{
				rgbBorder = m_lutDarker->GetAboveRangeColor();
			}
			else
			{
				rgbBorder = m_lut->GetAboveRangeColor();
			}
			
			unsigned char* ucrgb = iACompVisOptions::getColorArray(rgbBorder);
			colorArray->InsertTuple3(i, ucrgb[0], ucrgb[1], ucrgb[2]);
		}
		else if (numberOfObjects < minNumber)
		{
			if (m_useDarkerLut)
			{
				rgbBorder = m_lutDarker->GetBelowRangeColor();
			}
			else
			{
				rgbBorder = m_lut->GetBelowRangeColor();
			}
			
			unsigned char* ucrgb = iACompVisOptions::getColorArray(rgbBorder);
			colorArray->InsertTuple3(i, ucrgb[0], ucrgb[1], ucrgb[2]);
		}
		else
		{
			if (m_useDarkerLut)
			{
				m_lutDarker->GetColor(numberOfObjects, rgb);
			}
			else
			{
				m_lut->GetColor(numberOfObjects, rgb);
			}
			
			unsigned char* ucrgb = iACompVisOptions::getColorArray(rgb);
			colorArray->InsertTuple3(i, ucrgb[0], ucrgb[1], ucrgb[2]);
		}
	}
}

void buildGlyphRepresentation(void *arg)
{
	vtkProgrammableGlyphFilter* glyphFilter = (vtkProgrammableGlyphFilter*)arg;
	double origin[3];
	double point1[3];
	double point2[3];

	int pid = glyphFilter->GetPointId();
	glyphFilter->GetPointData()->GetArray("originArray")->GetTuple(pid, origin);
	glyphFilter->GetPointData()->GetArray("point1Array")->GetTuple(pid, point1);
	glyphFilter->GetPointData()->GetArray("point2Array")->GetTuple(pid, point2);

	vtkSmartPointer<vtkPlaneSource> plane = vtkSmartPointer<vtkPlaneSource>::New();
	plane->SetXResolution(1);
	plane->SetYResolution(1);

	plane->SetOrigin(origin);
	plane->SetPoint1(point1);
	plane->SetPoint2(point2);
	plane->Update();

	glyphFilter->SetSourceData(plane->GetOutput());
}

void iACompVariableTable::reinitalizeState()
{
	m_useDarkerLut = false;
}

/****************************************** Ordering/Ranking **********************************************/

void iACompVariableTable::drawHistogramTableInAscendingOrder(int bins)
{
	LOG(lvlDebug, "Variable Table Draw Ascending");

	std::vector<int> amountObjectsEveryDataset = *(m_activeData->getAmountObjectsEveryDataset());

	//stores the new order of the datasets as their new position
	std::vector<int>* newOrder = m_vis->sortWithMemory(amountObjectsEveryDataset, 0);
	m_vis->setOrderOfIndicesDatasets(m_vis->reorderAccordingTo(newOrder));

	if (m_useDarkerLut)
	{
		removeBarCharShowingAmountOfObjects();
	}

	m_useDarkerLut = true;

	drawHistogramTable();
	drawBarChartShowingAmountOfObjects(amountObjectsEveryDataset);
	renderWidget();
}

void iACompVariableTable::drawHistogramTableInDescendingOrder(int bins)
{
	std::vector<int> amountObjectsEveryDataset = *(m_activeData->getAmountObjectsEveryDataset());

	//stores the new order of the datasets as their new position
	std::vector<int>* newOrder = m_vis->sortWithMemory(amountObjectsEveryDataset, 1);
	m_vis->setOrderOfIndicesDatasets(m_vis->reorderAccordingTo(newOrder));

	if (m_useDarkerLut)
	{
		removeBarCharShowingAmountOfObjects();
	}

	m_useDarkerLut = true;

	drawHistogramTable();
	drawBarChartShowingAmountOfObjects(amountObjectsEveryDataset);
	renderWidget();
}

void iACompVariableTable::drawHistogramTableInOriginalOrder(int bins)
{
	std::vector<int>* originalOrderOfIndicesDatasets = m_vis->getOriginalOrderOfIndicesDatasets();
	std::vector<int>* orderOfIndicesDatasets = m_vis->getOrderOfIndicesDatasets();

	iACompVisOptions::copyVector(originalOrderOfIndicesDatasets, orderOfIndicesDatasets);
	m_useDarkerLut = true;

	drawHistogramTable();
	
	std::vector<int> amountObjectsEveryDataset = *(m_activeData->getAmountObjectsEveryDataset());
	drawBarChartShowingAmountOfObjects(amountObjectsEveryDataset);
	
	renderWidget();
}

void iACompVariableTable::drawBarChartShowingAmountOfObjects(std::vector<int> amountObjectsEveryDataset)
{
	std::vector<int>* orderOfIndicesDatasets = m_vis->getOrderOfIndicesDatasets();

	//define bars
	auto minMax = std::minmax_element(begin(amountObjectsEveryDataset), end(amountObjectsEveryDataset));
	int max = *minMax.second;

	for (int i = 0; i < m_originalPlanes->size(); i++)
	{
		vtkSmartPointer<vtkPlaneSource> currPlane = m_originalPlanes->at(i);

		createBar(currPlane, amountObjectsEveryDataset.at(orderOfIndicesDatasets->at(i)), max);
		createAmountOfObjectsText(currPlane, amountObjectsEveryDataset.at(orderOfIndicesDatasets->at(i)));
	}
}

/****************************************** Update THIS **********************************************/
void iACompVariableTable::showSelectionOfCorrelationMap(std::map<int, double>* dataIndxSelectedType)
{
}

void iACompVariableTable::removeSelectionOfCorrelationMap()
{
}