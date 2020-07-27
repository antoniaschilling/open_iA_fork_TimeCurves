#include "dlg_VisMainWindow.h"

//iA
#include "mainwindow.h"
#include "iACompVisMain.h"

dlg_VisMainWindow::dlg_VisMainWindow(QList<csvFileData>* data, iAMultidimensionalScaling* mds, MainWindow* parent, iACompVisMain* main)
	:
	QMainWindow(parent),
	m_data(data),
	m_mds(mds),
	m_main(main)
{
	//setup mainwindow
	parent->mdiArea->addSubWindow(this);
	setupUi(this);

	//start mds dialog
	startMDSDialog();

	//finish mainwindow setup
	createMenu();
	this->showMaximized();
}

void dlg_VisMainWindow::startMDSDialog()
{
	m_MDSD = new dlg_MultidimensionalScalingDialog(m_data, m_mds);
	if (m_MDSD->exec() != QDialog::Accepted)
	{
		return;
	}
}

QList<csvFileData>* dlg_VisMainWindow::getData()
{
	return m_data;
}

void dlg_VisMainWindow::createMenu()
{
	connect(actionRecalculateMDS, &QAction::triggered, this, &dlg_VisMainWindow::startMDSDialog);
	
	connect(actionAscending_to_Number_of_Objects, &QAction::triggered, this, &dlg_VisMainWindow::reorderHistogramTableAscending);
	connect(actionDescending_to_Number_of_Objects, &QAction::triggered, this, &dlg_VisMainWindow::reorderHistogramTableDescending);
	connect(actionAs_Loaded, &QAction::triggered, this, &dlg_VisMainWindow::reorderHistogramTableAsLoaded);
}

void dlg_VisMainWindow::reorderHistogramTableAscending()
{
	m_main->orderHistogramTableAscending();
}

void dlg_VisMainWindow::reorderHistogramTableDescending()
{
	m_main->orderHistogramTableDescending();
}

void dlg_VisMainWindow::reorderHistogramTableAsLoaded()
{
	m_main->orderHistogramTableAsLoaded();
}