/*********************************  open_iA 2016 06  ******************************** *
* **********  A tool for scientific visualisation and 3D image processing  ********** *
* *********************************************************************************** *
* Copyright (C) 2016  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan, J. Weissenb�ck, *
*                     Artem & Alexander Amirkhanov, B. Fr�hler                        *
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
* Contact: FH O� Forschungs & Entwicklungs GmbH, Campus Wels, CT-Gruppe,              *
*          Stelzhamerstra�e 23, 4600 Wels / Austria, Email:                           *
* ************************************************************************************/
#pragma once

#include "open_iA_Core_export.h"

#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkSmartPointer.h>

#include <QObject>

#include <set>
#include <vector>

class iAChannelVisualizationData;
class iAChannelRenderData;
class iAObserverProgress;
class RenderObserver;

class vtkActor;
class vtkAnnotatedCubeActor;
class vtkAxesActor;
class vtkCamera;
class vtkCellLocator;
class vtkColorTransferFunction;
class vtkCornerAnnotation;
class vtkCubeSource;
class vtkImageData;
class vtkInteractorStyleSwitch;
class vtkLogoRepresentation;
class vtkLogoWidget;
class vtkOpenGLRenderer;
class vtkOrientationMarkerWidget;
class vtkOutlineFilter;
class vtkPicker;
class vtkPiecewiseFunction;
class vtkPlane;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkQImageToImageSource;
class vtkRenderer;
class vtkRenderWindowInteractor;
class vtkSmartVolumeMapper;
class vtkTransform;
class vtkVolume;
class vtkVolumeProperty;


class open_iA_Core_API iARenderer: public QObject
{
	Q_OBJECT
public:
	iARenderer( QObject *parent = 0 );
	virtual ~iARenderer( );

protected:
	void InitObserver();

public:
	void initialize( vtkImageData* ds, vtkPolyData* pd, int e = 10 );
	void reInitialize( vtkImageData* ds, vtkPolyData* pd, int e = 10 );
	void setPolyData( vtkPolyData* pd );
	vtkPolyData* getPolyData();

/*
	// TODO: VOLUME: rewrite to use different volumes?
	void updateChannelImages();
	void addChannel(iAChannelVisualizationData * chData );
	void removeChannel(iAChannelVisualizationData * chData);
	void showMainVolumeWithChannels(bool show);
*/

	void disableInteractor();
	void enableInteractor();
	void setAxesTransform(vtkTransform *transform) { axesTransform = transform; }
	vtkTransform * getAxesTransform(void) { return axesTransform; }

	void setPlaneNormals( vtkTransform *tr ) ;
	void setCubeCenter( int x, int y, int z );
	void setCamPosition ( int uvx, int uvy, int uvz, int px, int py, int pz );

	/**
	* \brief	Set viewup, position and focal point of a renderer from the information in a double array.
	*
	* This function is used to assign camera settings from one mdichild to others.
	*
	* \param	camOptions	All informations of the camera stored in a double array
	* \param	rsParallelProjection	boolean variable to determine if parallel projection option is on.
	*/
	void setCamPosition( double * camOptions, bool rsParallelProjection  );
	void setCamera(vtkCamera* c);
	vtkCamera* getCamera();

	/**
	* \brief	Returns viewup, position and focal point information of a renderer in a double array.
	*
	* \param	camOptions	double array where all informations about the camera will be stored
	*/
	void getCamPosition ( double * camOptions );
	void setStatExt( int s ) { ext = s; };

	void setupCutter();
	void setupCube();
	void setupAxes(double spacing[3]);
	void setupOrientationMarker();
	void setupRenderer(vtkImageData* ds);
	void update();
	void showHelpers(bool show);
	void showRPosition(bool show);

	vtkPlane* getPlane1() { return plane1; };
	vtkPlane* getPlane2() { return plane2; };
	vtkPlane* getPlane3() { return plane3; };
	vtkRenderWindowInteractor* GetInteractor() { return interactor; };
	vtkRenderWindow* GetRenderWindow() { return renWin; };

	vtkOpenGLRenderer * GetRenderer() { return ren; };
	vtkActor* GetOutlineActor() { return outlineActor; };
	vtkActor* GetPolyActor() { return polyActor; };
	vtkTransform* getCoordinateSystemTransform();
	void GetImageDataBounds(double bounds[6]);
	vtkOpenGLRenderer * GetLabelRenderer (void) { return labelRen; }
	vtkPolyDataMapper* GetPolyMapper() const { return polyMapper; }
	
	iAObserverProgress* getObserverFPProgress() { return observerFPProgress; };
	iAObserverProgress* getObserverGPUProgress() { return observerGPUProgress; };

	void saveMovie(const QString& fileName, int mode, int qual = 2);
	RenderObserver * getRenderObserver(){ return renderObserver; }

/*
	// TODO: move out of here, to MObjects ----------
	void setTransferFunctions(vtkPiecewiseFunction* opacityTFHighlight, vtkColorTransferFunction* colorTFHighlight, vtkPiecewiseFunction* opacityTFTransparent, vtkColorTransferFunction* colorTFTransparent);
	void initializeHighlight(vtkImageData* ds, vtkPiecewiseFunction* otfHighlight, vtkColorTransferFunction* ctfHighlight, vtkPiecewiseFunction* otf, vtkColorTransferFunction* ctf);
	void reInitializeHighlight(vtkImageData* ds, vtkPiecewiseFunction* otf, vtkColorTransferFunction* ctf);
	void visualizeHighlight(bool enabled);

	void setMeanObjectSelected ( bool s ) { meanObjectSelected = s; };
	bool getMeanObjectSelected ( ) { return meanObjectSelected; };
	void setMeanObjectHighlighted ( bool h ) { meanObjectHighlighted = h; };
	bool getMeanObjectHighlighted ( ) { return meanObjectHighlighted; };
	void setMeanObjectId( int id ) { meanObjectId = id; };
	int  getMeanObjectId( ) { return meanObjectId; };
	// <---------- until here!
*/
	void AddRenderer(vtkRenderer* renderer);
protected:
	RenderObserver *renderObserver;
	iAObserverProgress* observerFPProgress;
	iAObserverProgress* observerGPUProgress;

private:
	vtkRenderWindowInteractor* interactor;
	vtkInteractorStyleSwitch* interactorStyle;
	vtkGenericOpenGLRenderWindow* renWin;
	vtkOpenGLRenderer * ren, *labelRen;
	vtkSmartPointer<vtkCamera> cam;
	vtkPolyData* polyData;
	vtkCellLocator * cellLocator;
	vtkPolyDataMapper* polyMapper;
	vtkActor* polyActor;

	//! @{ Logo
	vtkLogoRepresentation *rep;
	vtkLogoWidget *logowidget;
	vtkQImageToImageSource *image1;
	//! @}

	//! @{ Bounding Box
	vtkOutlineFilter* outlineFilter;
	vtkPolyDataMapper* outlineMapper;
	vtkActor* outlineActor;
	//! @}

	// TODO: VOLUME: check if this can be removed:
	vtkImageData* imageData;
	
	vtkAnnotatedCubeActor* annotatedCubeActor;
	vtkAxesActor* axesActor;
	vtkAxesActor* moveableAxesActor;
	vtkTransform *axesTransform;
	vtkOrientationMarkerWidget* orientationMarkerWidget;
	vtkOutlineFilter *outlineSliceFilter;
	vtkPolyDataMapper *outlineSlicePolyDataMapper;
	vtkPlane *plane1, *plane2, *plane3;
	vtkPicker* pointPicker;

	// multi channel image members
	vtkImageData*	multiChannelImageData;
	std::set<iAChannelVisualizationData*>	m_channels;
	bool	m_showMainVolumeWithChannels;

	// mobject visualization members
	// TODO: remove ----------
	vtkImageData* imageDataHighlight;
	vtkPiecewiseFunction* piecewiseFunctionHighlight;
	vtkColorTransferFunction* colorTransferFunctionHighlight;
	vtkVolume* volumeHighlight;
	vtkVolumeProperty* volumePropertyHighlight;
	bool highlightMode;
	bool meanObjectSelected;
	bool meanObjectHighlighted;
	int meanObjectId;
	vtkPiecewiseFunction* piecewiseFunctionTransparent;
	vtkColorTransferFunction* colorTransferFunctionTransparent;
	// ------------> until here!

	//! @{
	//! position marker cube
	vtkCubeSource *cSource;
	vtkPolyDataMapper *cMapper;
	vtkActor *cActor;
	//! @}

	int ext; //!< statistical extent size
public slots:
	void mouseRightButtonReleasedSlot();
	void mouseLeftButtonReleasedSlot();
Q_SIGNALS:
	void msg(QString s);
	void progress(int);
	void Clicked(int, int, int);

	void reInitialized();
	void onSetupRenderer();
	void onSetCamera();
};
