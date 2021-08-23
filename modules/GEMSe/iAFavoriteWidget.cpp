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
#include "iAFavoriteWidget.h"

#include "iAImagePreviewWidget.h"
#include "iAImageTree.h"
#include "iAPreviewWidgetPool.h"
#include "iAQtCaptionWidget.h"
#include "iAGEMSeConstants.h"

#include <iALog.h>

#include <QVBoxLayout>

typedef QVBoxLayout LikeLayoutType;


iAFavoriteWidget::iAFavoriteWidget(iAPreviewWidgetPool* previewPool) :
	m_previewPool(previewPool)
{
	QWidget* favListWdgt = this;
	QHBoxLayout* favListLayout = new QHBoxLayout();
	favListLayout->setSpacing(0);
	favListLayout->setContentsMargins(0, 0, 0, 0);
	favListLayout->setAlignment(Qt::AlignTop | Qt::AlignCenter);
	favListWdgt->setLayout(favListLayout);
	favListWdgt->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QWidget* likes = new QWidget();
	m_likeLayout =  new LikeLayoutType();
	m_likeLayout->setSpacing(ExampleViewSpacing);
	m_likeLayout->setContentsMargins(ExampleViewSpacing, ExampleViewSpacing, ExampleViewSpacing, ExampleViewSpacing);
	m_likeLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	likes->setLayout(m_likeLayout);
	likes->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	likes->setStyleSheet("background-color: #DFD;");

	favListLayout->addWidget(likes);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}


bool iAFavoriteWidget::HasAnyFavorite() const
{
	return m_favorites.size() > 0;
}


bool iAFavoriteWidget::ToggleLike(iAImageTreeNode * node)
{
	if (!node)
	{
		LOG(lvlError, "Favorites: ToggleLike called for nullptr node.\n");
		return false;
	}
	if (node->GetAttitude() == iAImageTreeNode::Liked)
	{
		node->SetAttitude(iAImageTreeNode::NoPreference);
		Remove(node);
		return false;
	}
	else
	{
		node->SetAttitude(iAImageTreeNode::Liked);
		Add(node);
		return true;
	}
}

bool iAFavoriteWidget::ToggleHate(iAImageTreeNode * node)
{
	if (!node)
	{
		LOG(lvlError, "Favorites: ToggleHate called for nullptr node.\n");
		return false;
	}
	if (node->GetAttitude() == iAImageTreeNode::Hated)
	{
		node->SetAttitude(iAImageTreeNode::NoPreference);
		return false;
	}
	else
	{
		if (node->GetAttitude() == iAImageTreeNode::Liked)
		{
			int idx = GetIndexForNode(node);
			if (idx == -1)
			{
				LOG(lvlError, "Favorites: node not found in favorite list.\n");
				return false;
			}
			iAImagePreviewWidget * widget = m_favorites[idx].widget;
			if (!widget)
			{
				LOG(lvlError, "Favorites: remove called for unset widget.\n");
				return false;
			}
			m_likeLayout->removeWidget(widget);
		}
		node->SetAttitude(iAImageTreeNode::Hated);
		return true;
	}
}


void iAFavoriteWidget::Add(iAImageTreeNode * node)
{
	if (!node || node->GetAttitude() != iAImageTreeNode::Liked)
	{
		return;
	}
	iAImagePreviewWidget * widget = m_previewPool->getWidget(this);
	if (!widget)
	{
		LOG(lvlError, "FavoriteView: No more slicer widgets available.\n");
		return;
	}
	widget->setFixedSize(FavoriteWidth, FavoriteWidth);
	widget->setImage(node->GetRepresentativeImage(iARepresentativeType::Difference,
		LabelImagePointer()), false, true);
	connect(widget, &iAImagePreviewWidget::clicked, this, &iAFavoriteWidget::FavoriteClicked);
	connect(widget, &iAImagePreviewWidget::rightClicked, this, &iAFavoriteWidget::FavoriteRightClicked);
	connect(widget, &iAImagePreviewWidget::updated, this, &iAFavoriteWidget::ViewUpdated);
	m_favorites.push_back(FavoriteData(node, widget));
	dynamic_cast<LikeLayoutType*>(m_likeLayout)->insertWidget(0, widget);
}


void iAFavoriteWidget::Remove(iAImageTreeNode const * node)
{
	if (!node)
	{
		LOG(lvlError, "Favorites: remove called for nullptr node\n");
		return;
	}
	int idx = GetIndexForNode(node);
	if (idx == -1)
	{
		LOG(lvlError, "Favorites: node not found in favorite list\n");
		return;
	}
	iAImagePreviewWidget * widget = m_favorites[idx].widget;
	if (!widget)
	{
		LOG(lvlError, "Favorites: remove called for unset widget\n");
		return;
	}
	m_favorites[idx].node = 0;
	m_favorites[idx].widget = 0;
	m_favorites.remove(idx);
	disconnect(widget, &iAImagePreviewWidget::clicked, this, &iAFavoriteWidget::FavoriteClicked);
	disconnect(widget, &iAImagePreviewWidget::rightClicked, this, &iAFavoriteWidget::FavoriteRightClicked);
	disconnect(widget, &iAImagePreviewWidget::updated, this, &iAFavoriteWidget::ViewUpdated);
	m_previewPool->returnWidget(widget);
}


void iAFavoriteWidget::FavoriteClicked()
{
	iAImagePreviewWidget * widget = dynamic_cast<iAImagePreviewWidget*>(sender());
	if (!widget)
	{
		LOG(lvlError, "FavoriteClicked: Invalid sender!\n");
	}
	iAImageTreeNode * node = GetNodeForWidget(widget);
	if (!node)
	{
		LOG(lvlError, "FavoriteClicked: Node not found!\n");
	}
	emit clicked(node);
}


void iAFavoriteWidget::FavoriteRightClicked()
{
	iAImagePreviewWidget * widget = dynamic_cast<iAImagePreviewWidget*>(sender());
	if (!widget)
	{
		LOG(lvlError, "FavoriteClicked: Invalid sender!\n");
	}
	iAImageTreeNode * node = GetNodeForWidget(widget);
	if (!node)
	{
		LOG(lvlError, "FavoriteClicked: Node not found!\n");
	}
	emit rightClicked(node);
}


int iAFavoriteWidget::GetIndexForNode(iAImageTreeNode const* node)
{
	for (int i=0; i<m_favorites.size(); ++i)
	{
		if (m_favorites[i].node == node)
		{
			return i;
		}
	}
	return -1;
}


iAImageTreeNode * iAFavoriteWidget::GetNodeForWidget(iAImagePreviewWidget* widget)
{
	for (FavoriteData const & fav: m_favorites)
	{
		if (fav.widget == widget)
		{
			return fav.node;
		}
	}
	return 0;
}


QVector<iAImageTreeNode const *> iAFavoriteWidget::GetFavorites(iAImageTreeNode::Attitude att) const
{
	QVector<iAImageTreeNode const *> result;
	for (FavoriteData const & fav : m_favorites)
	{
		if (fav.node->GetAttitude() == att)
		{
			result.push_back(fav.node);
		}
	}
	return result;
}
