#include "Model.h"

using namespace std;


Model::Model(string path) {

	xBound[0] = yBound[0] = zBound[0] = DBL_MAX;
	xBound[1] = yBound[1] = zBound[1] = DBL_MIN;
	memset(unionVertex, 0, sizeof(unionVertex));
	vertexBuffer = new Vertex[N];
	edgeBuffer = new Edge[N * 2];
	tetraBuffer = new Tetra[N];

	int loadVCnt, loadTCnt, temp;

	ifstream fs("./input/" + path + ".node");
	fs >> loadVCnt >> temp >> temp >> temp;

	for (int i = 0; i < loadVCnt; i++) {
		double x, y, z;
		fs >> temp >> x >> y >> z >> temp;
		Vector3d Pos(x, y, z);
		insertVertex(Pos);
	}

	fs.close();
	fs.open("./input/" + path + ".ele");
	fs >> loadTCnt >> temp >> temp;
	for (int i = 0; i < loadTCnt; i++) {
		int ids[4];
		fs >> temp >> ids[0] >> ids[1] >> ids[2] >> ids[3];
		insertTetra(ids);
	}

	cout << "Load success. V = " << vCnt << ", F = " << tCnt << endl;
}

Model::~Model() {
	delete[] vertexBuffer;
	delete[] edgeBuffer;
	delete[] tetraBuffer;
}

Vertex* Model::insertVertex(Vector3d Pos)
{
	xBound[0] = min(xBound[0], Pos.x());
	xBound[1] = max(xBound[1], Pos.x());
	yBound[0] = min(yBound[0], Pos.y());
	yBound[1] = max(yBound[1], Pos.y());
	zBound[0] = min(zBound[0], Pos.z());
	zBound[1] = max(zBound[1], Pos.z());
	vCnt++;
	vertexBuffer[vCnt] = Vertex(vCnt, Pos);
	return &vertexBuffer[vCnt];
}


Edge* Model::insertEdge(int id0, int id1) {
	if (id0 > id1)
		swap(id0, id1);
	Vertex* v0 = &vertexBuffer[id0], * v1 = &vertexBuffer[id1];
	Edge* target = eMap[make_pair(id0, id1)];
	if (target)return target;
	eCnt++;
	edgeBuffer[eCnt] = Edge(v0, v1);
	target = (&edgeBuffer[eCnt]);
	v0->edge.push_back(target);
	v1->edge.push_back(target);
	eMap[make_pair(id0, id1)] = target;
	return target;
}

Tetra* Model::insertTetra(int id[])
{
	sort(id, id+4);

	for (int i = 0; i < 4; i++) {
		triple tri;
		for (int j = 0; j < 4; j++) {
			if (j == i)continue;
			tri.insert(id[j]);
		}
		assert(tri.cnt == 3);
		fMap[tri]++;
	}

	Edge* newEdges[6];
	int cnt = 0;
	for (int i = 0; i < 4; i++) {
		for (int j = i + 1; j < 4; j++) {
			newEdges[cnt++] = insertEdge(id[i], id[j]);
		}
	}
	assert(cnt == 6);
	tCnt++;
	tetraBuffer[tCnt] = Tetra(newEdges);
	Tetra* target = &tetraBuffer[tCnt];
	for (auto e : newEdges) {
		e->tetra.push_back(target);
	}
	//Cofficient c = faceBuffer[fCnt].getCofficient();
	//cout <<"abc:"<< a * a + b * b + c * c << endl;
	//vertexBuffer[id0].C = vertexBuffer[id0].C + c;
	//vertexBuffer[id1].C = vertexBuffer[id1].C + c;
	//vertexBuffer[id2].C = vertexBuffer[id2].C + c;
	return target;
}

vtkSmartPointer<vtkUnstructuredGrid> Model::outputVtk(string path)
{
	vtkSmartPointer<vtkUnstructuredGrid> unGrid= vtkSmartPointer<vtkUnstructuredGrid>::New();
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkCellArray> cellArray = vtkSmartPointer<vtkCellArray>::New();
	points->InsertNextPoint(0, 0, 0);

	for (int i = 1; i <= vCnt; i++) {
		Vertex* v = &vertexBuffer[i];
		points->InsertNextPoint(v->pos.x(), v->pos.y(), v->pos.z());
	}

	for (int i = 1; i <= tCnt; i++) {
		Tetra* t = &tetraBuffer[i];
		if (t->valid()) {
			vector<int>vs = t->getVertexIds();
			vtkSmartPointer<vtkTetra> tetra = vtkSmartPointer<vtkTetra>::New();
			
			for (int i = 0; i < vs.size(); i++) {
				tetra->GetPointIds()->SetId(i, vs[i]);
			}
			cellArray->InsertNextCell(tetra);
		}
	}
	
	unGrid->SetPoints(points);
	unGrid->SetCells(10,cellArray);

	vtkSmartPointer<vtkUnstructuredGridWriter> writer = vtkSmartPointer<vtkUnstructuredGridWriter>::New();
	writer->SetFileName(("./outputVtk/" + path + ".vtk").c_str());
	writer->SetInputData(unGrid);
	writer->Write();

	return unGrid;
};



void Model::setEdgeCost()
{
	for (int i = 1; i <= eCnt; i++) {
		assert(edgeBuffer[i].valid());
		Edge* e = &edgeBuffer[i];
		if (!unionVertex[e->v0->id] && !unionVertex[e->v1->id])
			edgeBuffer[i].InitCost(H);
	}
}

bool Model::collaspeMin()
{
	Edge* e = (Edge*)H.top();
	H.pop();
	if (!e->valid()) return 0;
	if (e->checkInversion())return 0;
	e->collapse();
	return 1;
}

void Model::getUnionVertex()
{
	for (auto f : fMap) {
		if (f.second == 1)
			for (int i = 0; i < 3; i++) {
				unionVertex[f.first.ids[i]] = 1;
			}
	}
}

//void Model::outputStand(int n,bool ran)
//{	
//	ofstream fs("./mini.obj");
//	for (int i = 0; i < n; i++) {
//		for (int j = 0; j < n; j++) {
//			fs << "v " << j <<" " << i <<" " << (ran?rand()%5:0) << "\n";
//		}
//	}
//
//	int* low = new int[n], *high = new int[n];
//
//	for (int i = 0; i < n-1; i++) {
//		for (int j = 1; j <= n; j++) {
//			low[j] = i * n + j; high[j] = (i + 1) * n + j;
//		}
//		for (int j = 1; j <= n-1; j++) {
//			fs << "f " << low[j] << " " << low[j + 1] << " " << high[j + 1] << endl;
//			fs << "f " << low[j] << " " << high[j + 1] << " " << high[j] << endl;
//		}
//	}
//	fs.close();
//}



//void Model::segment()
//{
//	ofstream ss[4];
//	ss[0].open("./segment/0.obj");
//	ss[1].open("./segment/1.obj");
//	ss[2].open("./segment/2.obj");
//	ss[3].open("./segment/3.obj");
//	ofstream* s;
//	for (int i = 1; i <= vCnt; i++) {
//		Vertex* v = &vertexBuffer[i];
//		for (int i = 0; i < 4;i++) {
//			ss[i] << "v " << v->pos.x() << " " << v->pos.y() << " " << v->pos.z() << endl;
//		}
//	}
//
//	for (int i = 1; i <= fCnt; i++) {
//		Face* f = &faceBuffer[i];
//		Vector3d center = f->getCenter();
//		if (center.x() > (xBound[0] + xBound[1]) / 2) {
//			if (center.y() > yBound[0] + yBound[1] / 2) {
//				s = &ss[0];
//			}
//			else {
//				s = &ss[1];
//			}
//		}
//		else {
//			if (center.y() > yBound[0] + yBound[1] / 2) {
//				s = &ss[2];
//			}
//			else {
//				s = &ss[3];
//			}
//		}
//		vector<int>vs = f->getVertexIds();
//		(*s) << "f ";
//		for (int i = 0; i < vs.size(); i++) {
//			(*s) << vs[i] << " ";
//		}
//		(*s) << endl;
//	}
//	for (int i = 0; i < 4; i++) {
//		ss[i].close();
//	}
//}

//void Model::merge()
//{
//	Model* ms[4];
//	ms[0] = new Model("./output/0.obj");
//	ms[1] = new Model("./output/1.obj");
//	ms[2] = new Model("./output/2.obj");
//	ms[3] = new Model("./output/3.obj");
//	ofstream result("./output.obj");
//	for (int i = 1; i <= ms[0]->vCnt; i++) {
//		Vertex* v = &ms[0]->vertexBuffer[i];
//		result << "v " << v->pos.x() << " " << v->pos.y() << " " << v->pos.z() << endl;
//	}
//
//	for (int i = 0; i < 4; i++) {
//		for (int j = 1; j <= ms[i]->fCnt; j++) {
//			Face* f = &ms[i]->faceBuffer[j];
//			if (f->valid()) {
//				vector<int>vs = f->getVertexIds();
//				result << "f ";
//				for (int k = 0; k < vs.size(); k++) {
//					result << vs[k] << " ";
//				}
//				result << endl;
//			}
//		}
//	}
//
//	result.close();
//}


