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
#pragma once

#include "iArenderer_export.h"

#include <QMap>

class vtkCamera;
class vtkObject;
class vtkRenderer;

//! Class synchronizing the viewing parameters between multiple renderers.
//! Two "modes":
//! - Shared camera (default, i.e. sharedCamera = true in constructor),
//!   each added renderer gets assigned the same camera (the one from the
//!   renderer added first).
//!   Pro: Less overhead on updates (only triggering an update on other renderers)
//!   Con: The views share the visible area; so if the renderers have different
//!        pixel dimensions, viewed objects might be distorted
//! - Synchronized camera settings: An observer is registered to each
//!   added renderer, and whenever one of them is updated(=rendered),
//!   its main camera parameters (camera position, focal point, view up
//!   direction, clipping planes, and parallel scale if source camera has
//!   parallel projection turned on) are copied to all other renderers.
//!   Pro: Avoids sharing "too much", i.e. avoids distortions and renderers can have
//!        separate visible areas
//!   Con: Slightly more (coding) effort to synchronize relevant viewing parameters
//!        (and potentially error prone - relevant parameters for specific viewing
//!        modes might be missed)
class iArenderer_API iARendererViewSync
{
public:
	//! Creates a new renderer view synchronization service.
	//! @param sharedCamera whether to use shared camera (true)
	//!        or just synchronize required view parameters (false).
	//!        see class description for details.
	iARendererViewSync(bool sharedCamera = true);
	//! Adds given renderer to the bundle of renderers with synced views.
	//! Any change in that renderer's viewing parameters will be synced to all other
	//! renderers in the bundle.
	//! @param renderer the renderer to add to the bundle
	void addToBundle(vtkRenderer* renderer);
	//! Remove given renderer from bundle. Stops synchronizing this renderers'
	//! viewing parameters with the other renderers in the bundle.
	bool removeFromBundle(vtkRenderer* renderer);
	//! Remove all renderers from the bundle synchronized by this class.
	void removeAll();

private:
	//! @{ No copy construction or assignment
	iARendererViewSync(const iARendererViewSync&) = delete;
	iARendererViewSync& operator=(const iARendererViewSync&) = delete;
	//! @}

	//! callback for when one renderer is updated. updates viewing parameters in all
	//! other renderers in the bundle
	void redrawOtherRenderers(vtkObject* caller, long unsigned int eventId, void* callData);

	bool m_updateInProgress;    //!< avoids recursion in redrawOtherRenderers.
	vtkCamera* m_commonCamera;  //!< the common camera (if m_sharedCamera is true).
	bool m_sharedCamera;        //!< the viewing parameter share mode
	QMap<vtkRenderer*, unsigned long> m_rendererObserverTags;
};
