// Copyright 2016-2023, the open_iA contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "iADataSetListWidget.h"

#include "iADataSet.h"
#include "iALog.h"
#include "iAParameterDlg.h"

#include <QHeaderView>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QToolButton>

namespace
{
	QStringList columnNames = QStringList() << "Name" << "Views" << "Actions";
	enum ColumIdx {
		NameColumn = 0,
		ViewColumn = 1,
		EditColumn = 2,
		ColumnCount
	};

	void addActionButton(QWidget* w, QAction* a)
	{
		auto tb = new QToolButton(w);
		tb->setDefaultAction(a);
		w->layout()->addWidget(tb);
	}

	QWidget* actionWidget()
	{
		auto w = new QWidget();
		w->setLayout(new QHBoxLayout);
		w->layout()->setContentsMargins(0, 0, 0, 0);
		w->layout()->setSpacing(1);
		return w;
	}
}

iADataSetListWidget::iADataSetListWidget()
{
	m_dataList = new QTableWidget;
	m_dataList->setColumnCount(columnNames.size());
	m_dataList->setHorizontalHeaderLabels(columnNames);
	m_dataList->verticalHeader()->hide();
	m_dataList->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_dataList->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	m_dataList->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_dataList->resizeColumnsToContents();
	connect(m_dataList, &QTableWidget::itemSelectionChanged, this,
		[this]()
		{
			size_t dataSetIdx = std::numeric_limits<size_t>::max();    // TODO: same as iAMdiChild::NoDataSet - common constant somewhere!
			if (m_dataList->selectedItems().size() > 0)
			{
				dataSetIdx = m_dataList->item(m_dataList->selectedItems()[0]->row(), 0)->data(Qt::UserRole).toULongLong();
			}
			emit dataSetSelected(dataSetIdx);
		});
	setLayout(new QHBoxLayout);
	layout()->addWidget(m_dataList);
	layout()->setContentsMargins(1, 0, 0, 0);
	layout()->setSpacing(4);
}

void iADataSetListWidget::addDataSet(iADataSet const* dataset, size_t dataSetIdx)
{
	QSignalBlocker blockList(m_dataList);
	auto nameItem = new QTableWidgetItem(dataset->name());
	nameItem->setToolTip(dataset->info());
	nameItem->setData(Qt::UserRole, static_cast<qulonglong>(dataSetIdx));
	int row = m_dataList->rowCount();
	m_dataList->insertRow(row);
	m_dataList->setItem(row, NameColumn, nameItem);
	auto viewWidget = actionWidget();
	viewWidget->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	m_dataList->setCellWidget(row, ViewColumn, viewWidget);
	m_dataList->setCellWidget(row, EditColumn, actionWidget());
	m_dataList->resizeColumnsToContents();
}

void iADataSetListWidget::setName(size_t dataSetIdx, QString newName)
{
	int row = findDataSetIdx(dataSetIdx);
	assert(row != -1);
	m_dataList->item(row, 0)->setText(newName);
}

void iADataSetListWidget::removeDataSet(size_t dataSetIdx)
{
	int row = findDataSetIdx(dataSetIdx);
	assert(row != -1);
	m_dataList->removeRow(row);
}

void iADataSetListWidget::addAction(size_t dataSetIdx, QAction* viewAction, ActionColumn col)
{
	int row = findDataSetIdx(dataSetIdx);
	assert(row != -1);
	addActionButton(m_dataList->cellWidget(row, col == ActionColumn::View ? ViewColumn: EditColumn), viewAction);
}

int iADataSetListWidget::findDataSetIdx(size_t dataSetIdx)
{
	for (int row=0; row < m_dataList->rowCount(); ++row)
	{
		auto listDSIdx = m_dataList->item(row, 0)->data(Qt::UserRole).toULongLong();
		if (listDSIdx == dataSetIdx)
		{
			return row;
		}
	}
	return -1;
}
