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
#include "QVTKWidgetMouseReleaseWorkaround.h"

#if (VTK_MAJOR_VERSION >= 8 && defined(VTK_OPENGL2_BACKEND) )
QVTKWidgetMouseReleaseWorkaround::QVTKWidgetMouseReleaseWorkaround(QWidget* parent, Qt::WindowFlags f)
	: QVTKOpenGLWidget(parent, f)
{}
#else
QVTKWidgetMouseReleaseWorkaround::QVTKWidgetMouseReleaseWorkaround(QWidget* parent, Qt::WindowFlags f)
	: QVTKWidget(parent, f)
{}
#endif

void QVTKWidgetMouseReleaseWorkaround::mouseReleaseEvent( QMouseEvent * event )
{
	if ( Qt::RightButton == event->button() )
		emit rightButtonReleasedSignal();
	else if ( Qt::LeftButton == event->button() )
		emit leftButtonReleasedSignal();
#if (VTK_MAJOR_VERSION >= 8 && defined(VTK_OPENGL2_BACKEND) )
	QVTKOpenGLWidget::mouseReleaseEvent(event);
#else
	QVTKWidget::mouseReleaseEvent(event);
#endif
}

void QVTKWidgetMouseReleaseWorkaround::resizeEvent( QResizeEvent * event )
{
	repaint();//less flickering, but resize is less responsive
#if (VTK_MAJOR_VERSION >= 8 && defined(VTK_OPENGL2_BACKEND) )
	QVTKOpenGLWidget::resizeEvent(event);
#else
	QVTKWidget::resizeEvent(event);
#endif
}
