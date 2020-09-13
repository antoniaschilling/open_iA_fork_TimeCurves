/*************************************  open_iA  ************************************ *
* **********   A tool for visual analysis and processing of 3D CT images   ********** *
* *********************************************************************************** *
* Copyright (C) 2016-2019  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan, Ar. &  Al. *
*                          Amirkhanov, J. Weissenböck, B. Fröhler, M. Schiwarth       *
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

#include "iANModalSeedTracker.h"

#include "iANModalObjects.h"

#include "iASlicer.h"
#include "mdichild.h"
#include "dlg_slicer.h"

#include <QTimer>

// iANModalSeedTracker ----------------------------------------------------------------

iANModalSeedTracker::iANModalSeedTracker() {
	// Do nothing
}

iANModalSeedTracker::iANModalSeedTracker(MdiChild *mdiChild) {
	reinitialize(mdiChild);
}

void iANModalSeedTracker::reinitialize(MdiChild *mdiChild) {
	constexpr iASlicerMode modes[iASlicerMode::SlicerCount]{
		iASlicerMode::YZ,
		iASlicerMode::XZ,
		iASlicerMode::XY
	};

	for (int i = 0; i < iASlicerMode::SlicerCount; ++i) {
		auto mode = modes[i];
		m_visualizers[i] = new iANModalSeedVisualizer(mdiChild, mode);
		connect(m_visualizers[i], &iANModalSeedVisualizer::binClicked, [this, mode](int sliceNumber){
			emit binCliked(mode, sliceNumber); });
	}
}

iANModalSeedTracker::~iANModalSeedTracker() {
	teardown();
}

#define iANModal_FOR_EACH_VISUALIZER_DO(visualizer, athing) \
for (int iANModal_vis_id = 0; iANModal_vis_id < iASlicerMode::SlicerCount; ++iANModal_vis_id) { \
	auto visualizer = m_visualizers[iANModal_vis_id]; \
	athing; \
}

void iANModalSeedTracker::teardown() {
	iANModal_FOR_EACH_VISUALIZER_DO(vis, vis->teardown());
}

void iANModalSeedTracker::addSeeds(const QList<iANModalSeed> &seeds, const iANModalLabel &label) {
	iANModal_FOR_EACH_VISUALIZER_DO(vis, vis->addSeeds(seeds, label));
}

void iANModalSeedTracker::removeSeeds(const QList<iANModalSeed> &seeds) {
	iANModal_FOR_EACH_VISUALIZER_DO(vis, vis->removeSeeds(seeds));
}

void iANModalSeedTracker::removeAllSeeds() {
	iANModal_FOR_EACH_VISUALIZER_DO(vis, vis->removeAllSeeds());
}

void iANModalSeedTracker::updateLater() {
	iANModal_FOR_EACH_VISUALIZER_DO(vis, vis->updateLater());
}

// iANModalSeedVisualizer -------------------------------------------------------------

iANModalSeedVisualizer::iANModalSeedVisualizer(MdiChild *mdiChild, iASlicerMode mode) :
	m_mode(mode),
	m_timer_resizeUpdate(new QTimer())
{
	auto slicerDockWidget = mdiChild->slicerDockWidget(m_mode);
	slicerDockWidget->horizontalLayout_2->addWidget(this, 0);

	setMouseTracking(true);

	setToolTipDuration(-1);

	m_timer_resizeUpdate->setSingleShot(true);
	m_timer_resizeUpdate->setInterval(250); // In ms
	connect(m_timer_resizeUpdate, &QTimer::timeout, this, &iANModalSeedVisualizer::update);

	reinitialize(mdiChild);
}

void iANModalSeedVisualizer::reinitialize(MdiChild *mdiChild) {
	auto widget = mdiChild->slicerDockWidget(m_mode);

	int scrollbarWidth = widget->verticalScrollBar->minimumSize().width();
	int imageWidth = scrollbarWidth < 20 ? 20 : scrollbarWidth;
	setMinimumSize(QSize(imageWidth, 0));

	int min = widget->verticalScrollBar->minimum();
	int max = widget->verticalScrollBar->maximum();
	int range = max - min + 1;

	m_values.resize(range);
	std::fill(m_values.begin(), m_values.end(), 0);

	autoresize();
};

void iANModalSeedVisualizer::teardown() {
	deleteLater();
}

void iANModalSeedVisualizer::addSeeds(const QList<iANModalSeed> &seeds, const iANModalLabel &label) {
	int id = label.id;
	for (const auto &seed : seeds) {
		int i;
		switch (m_mode) {
		case iASlicerMode::XY: i = seed.z; break;
		case iASlicerMode::XZ: i = seed.y; break;
		case iASlicerMode::YZ: i = seed.x; break;
		default: break; // TODO: error 
		}
		++m_values[i];
	}
}

void iANModalSeedVisualizer::removeSeeds(const QList<iANModalSeed> &seeds) {
	for (const auto &seed : seeds) {
		int i;
		switch (m_mode) {
		case iASlicerMode::XY: i = seed.z; break;
		case iASlicerMode::XZ: i = seed.y; break;
		case iASlicerMode::YZ: i = seed.x; break;
		default: break; // TODO: error 
		}
		--m_values[i];
	}
}

void iANModalSeedVisualizer::removeAllSeeds() {
	std::fill(m_values.begin(), m_values.end(), 0);
}

void iANModalSeedVisualizer::updateLater() {
	m_timer_resizeUpdate->start();
}

void iANModalSeedVisualizer::update() {
	constexpr QRgb HISTOGRAM_COLOR_FOREGROUND = qRgb(0, 114, 189);
	constexpr QRgb HISTOGRAM_COLOR_FOREGROUND_SELECTED = qRgb(0, 0, 0);
	constexpr QRgb HISTOGRAM_COLOR_BACKGROUND = qRgb(255, 255, 255);
	constexpr QRgb HISTOGRAM_COLOR_BACKGROUND_SELECTED = qRgb(191, 191, 191);

	unsigned int maxValue = *std::max_element(m_values.begin(), m_values.end());

	if (maxValue == 0) {
		//m_image.fill(HISTOGRAM_COLOR_BACKGROUND);
		m_image.fill(QColor(255, 255, 255));

	} else {
		float maxValue_float = (float) maxValue;
		for (int y = 0; y < m_image.height(); ++y) {

			int sliceNumber = yToSliceNumber(y);
			int index = sliceNumber;
			unsigned int value = m_values[index];

			float lineLength_float = ((float) value / maxValue_float) * ((float) m_image.width());
			int lineLength = ceil(lineLength_float);
			assert((value == 0 && lineLength == 0) || (lineLength >= 1 && lineLength <= m_image.width()));

			QRgb fg, bg;
			if (index == m_indexHovered) {
				fg = HISTOGRAM_COLOR_FOREGROUND_SELECTED;
				bg = HISTOGRAM_COLOR_BACKGROUND_SELECTED;
			} else {
				fg = HISTOGRAM_COLOR_FOREGROUND;
				bg = HISTOGRAM_COLOR_BACKGROUND;
			}

			QRgb *line = (QRgb*) m_image.scanLine(y);
			int x = 0;
			for (; x < lineLength; ++x) {
				line[x] = fg;
			}
			for (; x < m_image.width(); ++x) {
				line[x] = bg;
			}
		}
	}

	QWidget::update();
}

inline int iANModalSeedVisualizer::yToSliceNumber(int y) {
	int y_inv = m_image.height() - 1 - y;
	float valueIndex_float = ((float)y_inv / (float)(m_image.height()) * (float)(m_values.size() - 1));
	int valueIndex = round(valueIndex_float);
	assert(valueIndex >= 0 && valueIndex < m_values.size());
	return valueIndex;
}

// EVENT FUNCTIONS --------------------------------------------------------------------

void iANModalSeedVisualizer::paint() {
	QPainter p(this);
	p.drawImage(0, 0, m_image);
}

void iANModalSeedVisualizer::autoresize() {
	resize(size().width(), size().height());
}

void iANModalSeedVisualizer::resize(int width, int height) {
	if (m_image.isNull()) {
		m_image = QImage(width, height, QImage::Format::Format_RGB32);
		update();
	} else {
		m_image = m_image.scaled(width, height);
		updateLater();
	}
}

void iANModalSeedVisualizer::click(int y) {
	int sliceNumber = yToSliceNumber(y);
	int index = sliceNumber;
	emit binClicked(index);
}

void iANModalSeedVisualizer::hover(int y) {
	int sliceNumber = yToSliceNumber(y);
	int index = sliceNumber;
	unsigned int value = m_values[index];
	setToolTip(QString("Slice %0\n%1 seed%2").arg(sliceNumber).arg(value).arg(value == 1 ? "" : "s"));

	if (index != m_indexHovered) {
		m_indexHovered = index;
		update();
	}
}

void iANModalSeedVisualizer::leave() {
	m_indexHovered = -1;
	update();
}

// EVENTS -----------------------------------------------------------------------------

void iANModalSeedVisualizer::paintEvent(QPaintEvent* event) {
	paint();
}

void iANModalSeedVisualizer::resizeEvent(QResizeEvent* event) {
	autoresize();
}

void iANModalSeedVisualizer::mousePressEvent(QMouseEvent* event) {
	click(event->pos().y());
}

void iANModalSeedVisualizer::mouseMoveEvent(QMouseEvent* event) {
	hover(event->pos().y());
}

void iANModalSeedVisualizer::leaveEvent(QEvent* event) {
	leave();
}