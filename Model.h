#pragma once

#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridWriter.h>
#include <vtkUnstructuredGridReader.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkTetra.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkCell.h>
#include <vtkFloatArray.h>

#include "Primes.h"
#include <iostream>
#include <fstream>
#include <tuple>

using namespace std;

class Model {
public:
	Model() {};
	Model(string path);
	void outputVtk(string fileName);
	~Model();
	vector<double>errArr;
	Vertex* vertexBuffer;
	int vCnt = 0;
	Edge* edgeBuffer;
	int eCnt = 0;
	map<pair<int, int>, Edge*>eMap;
	map<tuple<int, int, int>, bool>borderMap;
	Tetra* tetraBuffer;
	int tCnt = 0;
	int vElimate = 0, eElimate = 0, tElimate = 0;
	Heap *H;
	double upBound[4], downBound[4];
	Vertex* insertVertex(Vector4d Pos);
	Edge* insertEdge(int id0, int id1);
	Tetra* insertTetra(int id[]);
	void setEdgeCost();
	bool collaspeMin();
	void simplification(double rate);
	//void outputStand(int n,bool ran);
	void selectBorder();
	void AddQ();
	void setScale();
	double scaleRate;
	void getErr();
	//void segment();
	//void merge();
};