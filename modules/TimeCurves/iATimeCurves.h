#pragma once

#include "iAMainWindow.h"
#include "tapkee/tapkee.hpp"
#include "tapkee/defines/types.hpp"


class iATimeCurves
{
public:
	static void start(iAMainWindow* mainWindow);

	//load data
	bool loadData();

	//save matrix to computer
	bool downloadDistanceMatrix;

private:
	iAMainWindow* m_mainWindow;
	QStringList* csvFiles;
	int* headerLine;
	//compute distance matrix

	//parse csv
	void parseCsvAllToOne(std::vector<std::vector<double>>* data);

	std::vector<std::vector<double>>* parseCsv(QString fileName);

	bool mds();

	bool simpleMds();

	//for debugging
	bool filePath;
	bool precomputedFVs;
	bool precomputedMDS;
	void printTapkeeOutput(tapkee::TapkeeOutput output, QString fileName);
	void printTapkeeDistanceMatrix(tapkee::DenseMatrix output);
	void readDistanceMatrixFromFile(tapkee::DenseSymmetricMatrix* distanceMatrix, QString fileName);
	void mdsExample();
};
