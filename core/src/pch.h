/*************************************  open_iA  ************************************ *
* **********   A tool for visual analysis and processing of 3D CT images   ********** *
* *********************************************************************************** *
* Copyright (C) 2016-2018  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan,            *
*                          J. Weissenböck, Artem & Alexander Amirkhanov, B. Fröhler   *
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

#ifdef USE_PRECOMPILED_HEADERS
// #pragma message("Using pre-compiled headers\n")

#ifdef _MSC_VER
// need to do that very early, otherwise some itk header will include it,
// but in some other way so that "GetConsoleWindow" is not defined
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

// ITK
#include <itkAdaptiveHistogramEqualizationImageFilter.h>
#include <itkAdaptiveOtsuThresholdImageFilter.h>
#include <itkAddImageFilter.h>
#include <itkAdditiveGaussianNoiseImageFilter.h>
#include <itkAffineTransform.h>
#include <itkAndImageFilter.h>
#include <itkBarrier.h>
#include <itkBilateralImageFilter.h>
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryContourImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkBSplineInterpolateImageFunction.h>
#include <itkCannyEdgeDetectionImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkChangeInformationImageFilter.h>
#include <itkCommand.h>
#include <itkConfidenceConnectedImageFilter.h>
#include <itkConnectedComponentImageFilter.h>
#include <itkConnectedThresholdImageFilter.h>
#include <itkConstantPadImageFilter.h>
#include <itkConstNeighborhoodIterator.h>
#include <itkConvolutionImageFilter.h>
#include <itkCurvatureAnisotropicDiffusionImageFilter.h>
#include <itkCurvatureFlowImageFilter.h>
#include <itkDanielssonDistanceMapImageFilter.h>
#include <itkDecisionRule.h>
#include <itkDerivativeImageFilter.h>
#include <itkDiscreteGaussianImageFilter.h>
#include <itkDistanceToCentroidMembershipFunction.h>
#include <itkDivideImageFilter.h>
#include <itkEllipseSpatialObject.h>
#include <itkEuclideanDistanceMetric.h>
#include <itkExceptionObject.h>
#include <itkExtractImageFilter.h>
#include <itkFCMClassifierInitializationImageFilter.h>
#include <itkFFTConvolutionImageFilter.h>
#include <itkFFTNormalizedCorrelationImageFilter.h>
#include <itkFHWRescaleIntensityImageFilter.h>
#include <itkFlipImageFilter.h>
#include <itkFuzzyClassifierImageFilter.h>
#include <itkGaussianBlurImageFunction.h>
#include <itkGDCMImageIO.h>
#include <itkGDCMSeriesFileNames.h>
#include <itkGPUContextManager.h>
#include <itkGPUGradientAnisotropicDiffusionImageFilter.h>
#include <itkGPUImage.h>
#include <itkGPUImageToImageFilter.h>
#include <itkGPUKernelManager.h>
#include <itkGradientAnisotropicDiffusionImageFilter.h>
#include <itkGradientMagnitudeImageFilter.h>
#include <itkGrayscaleDilateImageFilter.h>
#include <itkGrayscaleErodeImageFilter.h>
#include <itkHessian3DToVesselnessMeasureImageFilter.h>
#include <itkHessianRecursiveGaussianImageFilter.h>
#include <itkHigerOrderAccurateGradient/itkHigherOrderAccurateDerivativeImageFilter.h>
#include <itkHistogramMatchingImageFilter.h>
#include <itkHuangThresholdImageFilter.h>
#include <itkIdentityTransform.h>
#include <itkImage.h>
#include <itkImageAdaptor.h>
#include <itkImageBase.h>
#include <itkImageDuplicator.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkImageIOBase.h>
#include <itkImageIOFactory.h>
#include <itkImageIORegion.h>
#include <itkImageKernelOperator.h>
#include <itkImageLinearIteratorWithIndex.h>
#include <itkImageMaskSpatialObject.h>
#include <itkImagePCAShapeModelEstimator.h>
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>
#include <itkImageSeriesReader.h>
#include <itkImageSeriesWriter.h>
#include <itkImageSliceConstIteratorWithIndex.h>
#include <itkImageSliceIteratorWithIndex.h>
#include <itkImageToHistogramFilter.h>
#include <itkImageToImageFilter.h>
#include <itkImageToVTKImageFilter.h>
#include <itkIntensityWindowingImageFilter.h>
#include <itkIntermodesThresholdImageFilter.h>
#include <itkInvertIntensityImageFilter.h>
#include <itkIsoDataThresholdImageFilter.h>
#include <itkJoinImageFilter.h>
#include <itkKdTree.h>
#include <itkKdTreeBasedKmeansEstimator.h>
#include <itkKFCMSClassifierInitializationImageFilter.h>
#include <itkKittlerIllingworthThresholdImageFilter.h>
#include <itkLabelGeometryImageFilter.h>
#include <itkLabelImageToLabelMapFilter.h>
#include <itkLabelImageToShapeLabelMapFilter.h>
#include <itkLabelMapMaskImageFilter.h>
#include <itkLabelOverlapMeasuresImageFilter.h>
#include <itkLabelStatisticsImageFilter.h>
#include <itkLabelToRGBImageFilter.h>
#include <itkLaplacianImageFilter.h>
#include <itkLaplacianRecursiveGaussianImageFilter.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkListSample.h>
#include <itkLiThresholdImageFilter.h>
#include <itkMacro.h>
#include <itkMaskImageFilter.h>
#include <itkMath.h>
#include <itkMaximumDistance.h>
#include <itkMaximumEntropyThresholdImageFilter.h>
#include <itkMeanSquaresImageToImageMetric.h>
#include <itkMedianImageFilter.h>
#include <itkMesh.h>
#include <itkMinimumDecisionRule.h>
#include <itkMinimumMaximumImageCalculator.h>
#include <itkMomentsThresholdImageFilter.h>
#include <itkMorphologicalWatershedImageFilter.h>
#include <itkMSKFCMClassifierInitializationImageFilter.h>
#include <itkMultiLabelSTAPLEImageFilter.h>
#include <itkMultiplyImageFilter.h>
#include <itkMutualInformationImageToImageMetric.h>
#include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkNeighborhoodConnectedImageFilter.h>
#include <itkNormalizedCorrelationImageFilter.h>
#include <itkNormalizedCorrelationImageToImageMetric.h>
#include <itkNormalizeImageFilter.h>
#include <itkNormalVariateGenerator.h>
#include <itkNrrdImageIO.h>
#include <itkNumericSeriesFileNames.h>
#include <itkNumericTraits.h>
#include <itkOtsuMultipleThresholdsImageFilter.h>
#include <itkOtsuThresholdImageFilter.h>
#include <itkOutputWindow.h>
#include <itkPasteImageFilter.h>
#include <itkPatchBasedDenoisingBaseImageFilter.h>
#include <itkPatchBasedDenoisingImageFilter.h>
#include <itkPermuteAxesImageFilter.h>
#include <itkPipelineMonitorImageFilter.h>
#include <itkProcessObject.h>
#include <itkProgressReporter.h>
#include <itkRawImageIO.h>
#include <itkRecursiveGaussianImageFilter.h>
#include <itkRelabelComponentImageFilter.h>
#include <itkRemovePeaksOtsuThresholdImageFilter.h>
#include <itkRenyiEntropyThresholdImageFilter.h>
#include <itkResampleImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkRGBAPixel.h>
#include <itkRGBPixel.h>
#include <itkRobustAutomaticThresholdImageFilter.h>
#include <itkSaltAndPepperNoiseImageFilter.h>
#include <itkSampleClassifierFilter.h>
#include <itkScalarConnectedComponentImageFilter.h>
#include <itkScalarImageKmeansImageFilter.h>
#include <itkScalarToRGBPixelFunctor.h>
#include <itkShanbhagThresholdImageFilter.h>
#include <itkShiftScaleImageFilter.h>
#include <itkShotNoiseImageFilter.h>
#include <itkSignedMaurerDistanceMapImageFilter.h>
#include <itkSmartPointer.h>
#include <itkSpatialObjectToImageFilter.h>
#include <itkSpeckleNoiseImageFilter.h>
#include <itkSTAPLEImageFilter.h>
#include <itkStatisticsImageFilter.h>
#include <itkStreamingImageFilter.h>
#include <itkSubtractImageFilter.h>
#include <itkSymmetricEigenAnalysisImageFilter.h>
#include <itkTestingComparisonImageFilter.h>
#include <itkThresholdImageFilter.h>
#include <itkTranslationTransform.h>
#include <itkTriangleCell.h>
#include <itkTriangleThresholdImageFilter.h>
#include <itkUnaryFunctorImageFilter.h>
#include <itkVector.h>
#include <itkVectorImage.h>
#include <itkVectorIndexSelectionCastImageFilter.h>
#include <itkVTKImageExport.h>
#include <itkVTKImageImport.h>
#include <itkVTKImageToImageFilter.h>
#include <itkWatershedImageFilter.h>
#include <itkWeightedCentroidKdTreeGenerator.h>
#include <itkWindowedSincInterpolateImageFunction.h>
#include <itkYenThresholdImageFilter.h>

// VTK
#include <QVTKInteractor.h>
#include <QVTKInteractorAdapter.h>
#include <QVTKWidget.h>
#include <QVTKWidget2.h>
#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkActor2DCollection.h>
#include <vtkActorCollection.h>
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkAnnotatedCubeActor.h>
#include <vtkAnnotationLink.h>
#include <vtkAppendPolyData.h>
#include <vtkAVIWriter.h>
#include <vtkAxes.h>
#include <vtkAxesActor.h>
#include <vtkAxis.h>
#include <vtkAxisActor2D.h>
#include <vtkBMPReader.h>
#include <vtkBMPWriter.h>
#include <vtkBorderRepresentation.h>
#include <vtkBorderWidget.h>
#include <vtkBrush.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCameraPass.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCellLocator.h>
#include <vtkCellPicker.h>
#include <vtkChart.h>
#include <vtkChartLegend.h>
#include <vtkChartParallelCoordinates.h>
#include <vtkChartXY.h>
#include <vtkColor.h>
#include <vtkColorTransferFunction.h>
#include <vtkCommand.h>
#include <vtkConeSource.h>
#include <vtkContext2D.h>
#include <vtkContextActor.h>
#include <vtkContextInteractorStyle.h>
#include <vtkContextMouseEvent.h>
#include <vtkContextScene.h>
#include <vtkContextTransform.h>
#include <vtkContextView.h>
#include <vtkContourFilter.h>
#include <vtkCoordinate.h>
#include <vtkCornerAnnotation.h>
#include <vtkCubeAxesActor.h>
#include <vtkCubeAxesActor2D.h>
#include <vtkCubeSource.h>
#include <vtkCutter.h>
#include <vtkCylinderSource.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkDataSetAttributes.h>
#include <vtkDataSetMapper.h>
#include <vtkDecimatePro.h>
#include <vtkDelaunay2D.h>
#include <vtkDepthPeelingPass.h>
#include <vtkDepthSortPolyData.h>
#include <vtkDiscretizableColorTransferFunction.h>
#include <vtkDiskSource.h>
#include <vtkDistancePolyDataFilter.h>
#include <vtkDoubleArray.h>
#include <vtkDynamic2DLabelMapper.h>
#include <vtkEdgeListIterator.h>
#include <vtkEvent.h>
#include <vtkEventQtSlotConnect.h>
#include <vtkFeatureEdges.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkFloatArray.h>
#include <vtkFlyingEdges3D.h>
#include <vtkFollower.h>
#include <vtkGenericMovieWriter.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkGenericRenderWindowInteractor.h>
#include <vtkGraph.h>
#include <vtkGraphItem.h>
#include <vtkIdTypeArray.h>
#include <vtkImageAccumulate.h>
#include <vtkImageActor.h>
#include <vtkImageBlend.h>
#include <vtkImageCast.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageData.h>
#include <vtkImageExport.h>
#include <vtkImageExtractComponents.h>
#include <vtkImageImport.h>
#include <vtkImageMapper3D.h>
#include <vtkImageMapToColors.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkImageProperty.h>
#include <vtkImageReader2.h>
#include <vtkImageResample.h>
#include <vtkImageReslice.h>
#include <vtkImageResliceMapper.h>
#include <vtkImageShiftScale.h>
#include <vtkImageSlice.h>
#include <vtkImageViewer2.h>
#include <vtkImageWriter.h>
#include <vtkImplicitFunction.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkIntArray.h>
#include <vtkInteractorObserver.h>
#include <vtkInteractorStyle.h>
#include <vtkInteractorStyleImage.h>
#include <vtkInteractorStyleSwitch.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkIOStream.h>
#include <vtkJPEGReader.h>
#include <vtkJPEGWriter.h>
#include <vtkKochanekSpline.h>
#include <vtkLegendScaleActor.h>
#include <vtkLight.h>
#include <vtkLightsPass.h>
#include <vtkLineSource.h>
#include <vtkLogoRepresentation.h>
#include <vtkLogoWidget.h>
#include <vtkLookupTable.h>
#include <vtkMapper2D.h>
#include <vtkMarchingContourFilter.h>
#include <vtkMarchingCubes.h>
#include <vtkMath.h>
#include <vtkMathUtilities.h>
#include <vtkMatrix4x4.h>
#include <vtkMatrixToLinearTransform.h>
#include <vtkMetaImageReader.h>
#include <vtkMetaImageWriter.h>
#include <vtkMutableDirectedGraph.h>
#include <vtkNew.h>
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkOBJReader.h>
#include <vtkOggTheoraWriter.h>
#include <vtkOpaquePass.h>
#include <vtkOpenGLRenderer.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkOutlineFilter.h>
#include <vtkOutputWindow.h>
#include <vtkParametricFunctionSource.h>
#include <vtkParametricSpline.h>
#include <vtkPCAStatistics.h>
#include <vtkPen.h>
#include <vtkPicker.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPlane.h>
#include <vtkPlaneCollection.h>
#include <vtkPlaneSource.h>
#include <vtkPlot.h>
#include <vtkPlotLine.h>
#include <vtkPlotParallelCoordinates.h>
#include <vtkPlotPoints.h>
#include <vtkPNGReader.h>
#include <vtkPNGWriter.h>
#include <vtkPointData.h>
#include <vtkPointLocator.h>
#include <vtkPointPicker.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkPolyDataNormals.h>
#include <vtkPolyDataSilhouette.h>
#include <vtkPolyLine.h>
#include <vtkProbeFilter.h>
#include <vtkProp.h>
#include <vtkPropCollection.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkPropPicker.h>
#include <vtkQImageToImageSource.h>
#include <vtkQuad.h>
#include <vtkQuadricClustering.h>
#include <vtkQuadricDecimation.h>
#include <vtkRegularPolygonSource.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderPassCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRotationalExtrusionFilter.h>
#include <vtkSampleFunction.h>
#include <vtkScalarBarActor.h>
#include <vtkScalarBarRepresentation.h>
#include <vtkScalarBarWidget.h>
#include <vtkScalarsToColors.h>
#include <vtkScatterPlotMatrix.h>
#include <vtkSelection.h>
#include <vtkSelectionNode.h>
#include <vtkSequencePass.h>
#include <vtkSmartPointer.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkSphereSource.h>
#include <vtkSplineFilter.h>
#include <vtkStdString.h>
#include <vtkSTLReader.h>
#include <vtkSTLWriter.h>
#include <vtkStringArray.h>
#include <vtkStripper.h>
#include <vtkStructuredData.h>
#include <vtkStructuredGrid.h>
#include <vtkStructuredGridGeometryFilter.h>
#include <vtkStructuredPoints.h>
#include <vtksys/SystemTools.hxx>
#include <vtkTable.h>
#include <vtkTextActor.h>
#include <vtkTextActor3D.h>
#include <vtkTextMapper.h>
#include <vtkTextProperty.h>
#include <vtkTexture.h>
#include <vtkThinPlateSplineTransform.h>
#include <vtkTIFFReader.h>
#include <vtkTIFFWriter.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTranslucentPass.h>
#include <vtkTriangle.h>
#include <vtkTubeFilter.h>
#include <vtkType.h>
#include <vtkTypeUInt32Array.h>
#include <vtkUnsignedCharArray.h>
#include <vtkVariantArray.h>
#include <vtkVector.h>
#include <vtkVersion.h>
#include <vtkVertexListIterator.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkVoxel.h>
#include <vtkWidgetCallbackMapper.h>
#include <vtkWidgetEvent.h>
#include <vtkWindow.h>
#include <vtkWindowedSincPolyDataFilter.h>
#include <vtkWindowToImageFilter.h>
#include <vtkWorldPointPicker.h>
#include <vtkXMLImageDataReader.h>

// QT
#include <QAbstractItemView>
#include <QAbstractTextDocumentLayout>
#include <QAction>
#include <QApplication>
#include <QBitmap>
#include <QBrush>
#include <QByteArray>
#include <QCheckBox>
#include <QCloseEvent>
#include <QCollator>
#include <QColor>
#include <QColorDialog>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QContiguousCache>
#include <QCoreApplication>
#include <QCursor>
#include <QDate>
#include <QDateTime>
#include <QDebug>
#include <QDesktopWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDirIterator>
#include <QDockWidget>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNodeList>
#include <QDoubleSpinBox>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QErrorMessage>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileInfoList>
#include <QFont>
#include <QFontMetrics>
#include <QFrame>
#include <QGLBuffer>
#include <QGLWidget>
#include <QGridLayout>
#include <QGroupBox>
#include <QHash>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QImage>
#include <QInputDialog>
#include <QIODevice>
#include <QItemDelegate>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QLayout>
#include <QLinearGradient>
#include <QLineEdit>
#include <QList>
#include <QListView>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLocale>
#include <QMainWindow>
#include <QMap>
#include <QMapIterator>
#include <QMdiSubWindow>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QModelIndex>
#include <QMouseEvent>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QOpenGLFramebufferObject>
#include <QPainter>
#include <QPaintEvent>
#include <QPair>
#include <QPen>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QPoint>
#include <QPolygon>
#include <QProcess>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QRect>
#include <QRegExp>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QRubberBand>
#include <QRunnable>
#include <QScopedPointer>
#include <QScrollArea>
#include <QScrollBar>
#include <QSet>
#include <QSettings>
#include <QSharedPointer>
#include <QShortcut>
#include <QShowEvent>
#include <QSignalMapper>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QSpinBox>
#include <QSplashScreen>
#include <QSplitter>
#include <QStack>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStatusBar>
#include <QString>
#include <QStringList>
#include <QStringListModel>
#include <QStyle>
#include <QStyleOption>
#include <QTableView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QtCore/QCache>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QFlags>
#include <QtCore/qglobal.h>
#include <QtCore/QMargins>
#include <QtCore/QMultiMap>
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QSharedPointer>
#include <QtCore/QStack>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtCore/QVector>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QXmlStreamWriter>
#include <QTextBrowser>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextStream>
#include <QtGui/QApplication>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QPalette>
#include <QtGui/QPixmap>
#include <QtGui/QWheelEvent>
#include <QtGui>
#include <QThread>
#include <QThreadPool>
#include <QTime>
#include <QTimer>
#include <QTimerEvent>
#include <QtMath>
#include <QToolBar>
#include <QToolButton>
#include <QToolTip>
#include <QtOpenGL/QGLWidget>
#include <QTreeView>
#include <QTreeWidget>
#include <QtWidgets>
#include <QtXml/QDomDocument>
#include <QtXml/QDomNode>
#include <QVariant>
#include <QVBoxLayout>
#include <QVector>
#include <QWaitCondition>
#include <QWheelEvent>
#include <QWidget>
#include <QXmlSimpleReader>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

 // Standard library includes
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#endif	// USE_PRECOMPILED_HEADERS
