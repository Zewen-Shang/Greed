#pragma once

#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridWriter.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkTetra.h>


#include "Primes.h"
#include <iostream>
#include <fstream>


struct triple {
	int ids[3], cnt = 0;
	triple(){
	}
	void insert(int id){
		ids[cnt++] = id;
	}
	bool operator<(const triple& other) const {
		if (ids[0] != other.ids[0])return ids[0] < other.ids[0];
		if (ids[1] != other.ids[1])return ids[1] < other.ids[1];
		if (ids[2] != other.ids[2])return ids[2] < other.ids[2];
		return 0;
	}
};

class Model {
public:
	Model() {};
	Model(string path);
	vtkSmartPointer<vtkUnstructuredGrid> outputVtk(string path);
	~Model();
	Vertex* vertexBuffer;
	int vCnt = 0;
	Edge* edgeBuffer;
	int eCnt = 0;
	map<pair<int, int>, Edge*>eMap;
	map<triple, int>fMap;
	Tetra* tetraBuffer;
	int tCnt = 0;
	int vElimate = 0, eElimate = 0, fElimate = 0;
	Heap  H;
	bool unionVertex[N];
	double xBound[2], yBound[2], zBound[2];
	Vertex* insertVertex(Vector3d Pos);
	Edge* insertEdge(int id0, int id1);
	Tetra* insertTetra(int id[]);
	void setEdgeCost();
	bool collaspeMin();
	//void outputStand(int n,bool ran);
	void getUnionVertex();
	//void segment();
	//void merge();
};