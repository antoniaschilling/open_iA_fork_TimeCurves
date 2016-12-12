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
*          Stelzhamerstra�e 23, 4600 Wels / Austria, Email: c.heinzl@fh-wels.at       *
* ************************************************************************************/

#include "iABoneThicknessModuleInterface.h"
#include "iABoneThicknessAttachment.h"
#include "mainwindow.h"

#include <mdichild.h>

iABoneThicknessModuleInterface::iABoneThicknessModuleInterface( )
{ /* not implemented */ }

iABoneThicknessModuleInterface::~iABoneThicknessModuleInterface( )
{ /* not implemented */ }

void iABoneThicknessModuleInterface::Initialize( )
{
	QMenu* toolsMenu (m_mainWnd->getToolsMenu());

	QAction* pBoneThickness (new QAction(QApplication::translate("MainWindows", "bone thickness", 0), m_mainWnd));
	connect(pBoneThickness, SIGNAL(triggered()), this, SLOT(slotBoneThickness()));
	toolsMenu->addAction(pBoneThickness);
}

void iABoneThicknessModuleInterface::slotBoneThickness()
{
	PrepareActiveChild();

	if (m_mdiChild)
	{
		AttachToMdiChild(m_mdiChild);
	}
}

iAModuleAttachmentToChild* iABoneThicknessModuleInterface::CreateAttachment(MainWindow* mainWnd, iAChildData childData)
{
	return new iABoneThicknessAttachment(mainWnd, childData);
}
