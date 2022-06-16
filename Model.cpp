#include "Model.h"

using namespace std;


Model::Model(string path) {
	for (int i = 0; i < 4; i++) {
		upBound[i] = DBL_MIN;
		downBound[i] = DBL_MAX;
	}

	int loadVCnt, loadTCnt, temp;

	vtkNew<vtkUnstructuredGridReader> reader;
	vtkUnstructuredGrid* unGrid;
	string fileName = "./input/" + path + ".vtk";
	reader->SetFileName(fileName.c_str());
	reader->Update();
	unGrid = reader->GetOutput();

	loadVCnt = unGrid->GetPoints()->GetNumberOfPoints();
	vertexBuffer = new Vertex[loadVCnt + 10];

	string attrName = "attr";

	for (int i = 0; i < loadVCnt; i++) {
		double *pos = unGrid->GetPoint(i);
		vtkPointData* pointData = unGrid->GetPointData();
		double* attr;
		attr = pointData->GetArray(attrName.c_str())->GetTuple(i);
		
		Vector4d Pos(pos[0], pos[1], pos[2], attr[0]);
		insertVertex(Pos);
	}

	loadTCnt = unGrid->GetCells()->GetNumberOfCells();
	edgeBuffer = new Edge[loadTCnt*2];
	tetraBuffer = new Tetra[loadTCnt+10];

	vtkIdList* ids;
	for (int i = 0; i < loadTCnt; i++) {
		int id[4];
		ids = unGrid->GetCell(i)->GetPointIds();
		for (int j = 0; j < 4; j++) {
			id[j] = ids->GetId(j);
		}
		insertTetra(id);
	}

	setScale();
	AddQ();
	cout << "Load success. V = " << vCnt << ", T = " << tCnt << endl;
}


Model::~Model() {
	delete[] vertexBuffer;
	delete[] edgeBuffer;
	delete[] tetraBuffer;
}

Vertex* Model::insertVertex(Vector4d Pos)
{
	for (int i = 0; i < 4; i++) {
		downBound[i] = min(downBound[i], Pos[i]);
		upBound[i] = max(upBound[i], Pos[i]);
	}
	vertexBuffer[vCnt] = Vertex(vCnt, Pos);
	return &vertexBuffer[vCnt++];
}


Edge* Model::insertEdge(int id0, int id1) {
	if (id0 > id1)
		swap(id0, id1);
	Vertex* v0 = &vertexBuffer[id0], * v1 = &vertexBuffer[id1];
	Edge* target = eMap[make_pair(id0, id1)];
	if (target)return target;
	edgeBuffer[eCnt] = Edge(v0, v1);
	target = (&edgeBuffer[eCnt]);
	eCnt++;
	v0->edge.push_back(target);
	v1->edge.push_back(target);
	eMap[make_pair(id0, id1)] = target;
	return target;
}

Tetra* Model::insertTetra(int id[])
{
	sort(id, id+4);
	tuple<int, int, int> tups[4] = {
		tuple<int,int,int>(id[0],id[1],id[2]),
		tuple<int,int,int>(id[0],id[1],id[3]),
		tuple<int,int,int>(id[0],id[2],id[3]),
		tuple<int,int,int>(id[1],id[2],id[3]),
	};

	for (int i = 0; i < 4; i++) {
		if (borderMap.count(tups[i]) == 1) {
			borderMap.erase(tups[i]);
		}
		else {
			borderMap[tups[i]] = 1;
		}
	}

	Edge* newEdges[6];
	int cnt = 0;
	for (int i = 0; i < 4; i++) {
		for (int j = i + 1; j < 4; j++) {
			newEdges[cnt++] = insertEdge(id[i], id[j]);
		}
	}
	assert(cnt == 6);
	tetraBuffer[tCnt] = Tetra(newEdges);
	Tetra* target = &tetraBuffer[tCnt];
	tCnt++;
	for (auto e : newEdges) {
		e->tetra.push_back(target);
	}

	return target;
}

void Model::outputVtk(string fileName)
{
	vtkSmartPointer<vtkUnstructuredGrid> unGrid= vtkSmartPointer<vtkUnstructuredGrid>::New();
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkCellArray> cellArray = vtkSmartPointer<vtkCellArray>::New();
	vtkNew<vtkPointData> pointData;
	vtkNew<vtkFloatArray> attr;

	string attrName = "attr";

	attr->SetName(attrName.c_str());

	for (int i = 0; i < vCnt; i++) {
		Vertex* v = &vertexBuffer[i];
		points->InsertNextPoint(v->pos.x(), v->pos.y(), v->pos.z());
		attr->InsertNextTuple1(v->pos(3)/scaleRate);
	}

	for (int i = 0; i < 3; i++) {
		unGrid->GetPointData()->AddArray(attr);
	}

	for (int i = 0; i < tCnt; i++) {
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
	writer->SetFileName(("./outputVtk/" + fileName + ".vtk").c_str());
	writer->SetInputData(unGrid);
	writer->Write();

	//return unGrid;
};


void Model::setEdgeCost()
{
	H = new Heap(eCnt + 10);
	for (int i = 0; i < eCnt; i++) {
		assert(edgeBuffer[i].valid());
		Edge* e = &edgeBuffer[i];
		edgeBuffer[i].InitCost(*H);
	}
}

bool Model::collaspeMin()
{
	Edge* e = (Edge*)H->top();
	H->pop();
	if (!e->valid()) return 0;
	if (e->checkInversion())return 0;
	tElimate += e->collapse();
	errArr.push_back(e->value);
	return 1;
}

void Model::simplification(double rate)
{
	int line = 10000;
	int targetNum = (1 - rate) * tCnt;
	while (tElimate < targetNum)
	{
		if (tElimate >= line) {
			cout << tElimate << endl;
			line += 10000;
		}
		if (H->cnt == 0) {
			for (int i = 0; i < eCnt; i++) {
				if (edgeBuffer[i].state == Valid) {
					edgeBuffer[i].InitCost(*H);
				}
			}
		}
		collaspeMin();
	}
}


void Model::selectBorder()
{
	set<int> borderSet;
	for (auto i : borderMap) {
		int vPos[3];
		vPos[0] = std::get<0>(i.first);
		vPos[1] = std::get<1>(i.first);
		vPos[2] = std::get<2>(i.first);


		Vector4d e0 = vertexBuffer[vPos[1]].pos - vertexBuffer[vPos[0]].pos, e1 = vertexBuffer[vPos[2]].pos - vertexBuffer[vPos[0]].pos;
		Vector3d a0 = e0.block(0, 0, 3, 1), a1 = e1.block(0, 0, 3, 1);
		double deter = a0.cross(a1).norm();
		e0.normalize();
		e1 = e1 - e0 * (e0.dot(e1));
		e1.normalize();
		Matrix4d A = Matrix4d::Identity();
		A -= (e0 * e0.transpose() + e1 * e1.transpose());
		A *= abs(deter) / 6.0 * 20;

		for (int j = 0; j < 3; j++) {
			vertexBuffer[vPos[j]].c.A += A;
			borderSet.insert(vPos[j]);
		}
	}
}

void Model::AddQ()
{
	for (int i = 0; i < tCnt; i++) {
		Tetra t = tetraBuffer[i];
		vector<Vertex*>ids = t.getVertexs();
		vector<Vector4d> es;
		for (int j = 0; j < 3; j++) {
			es.push_back((ids[j]->pos - ids[3]->pos).normalized());
		}
		es = smtOrth(es);
		Matrix4d A = Matrix4d::Identity();
		for (int j = 0; j < 3; j++) {
			A -= es[j] * es[j].transpose();
		}


		Matrix3d M;
		for (int j = 0; j < 3; j++) {
			M.col(j) = es[j].block(0, 0, 3, 1);
		}
		double deter = abs(M.determinant());

		for (int j = 0; j < 4; j++) {
			Vector4d p = vertexBuffer[ids[j]->id].pos;
			Vector4d b = -A * vertexBuffer[ids[j]->id].pos;
			double c = p.transpose() * A * p;
			Coff C(A, b, c);
			vertexBuffer[j].c = vertexBuffer[j].c + C * deter;
		}
	}
}

void Model::setScale()
{
	scaleRate = (upBound[0] - downBound[0]) * (upBound[1] - downBound[1]) * (upBound[2] - downBound[2]);
	scaleRate = pow(scaleRate, 1.0 / 3);
	scaleRate /= (upBound[3] - downBound[3]);
	for (int i = 0; i < vCnt; i++) {
		vertexBuffer[i].pos[3] *= scaleRate;
	}
}

void Model::getErr()
{
	double maxErr = 0, avgErr = 0;
	for (double err : errArr) {
		maxErr = max(maxErr, err);
		avgErr += err;
	}
	cout << "Max Err:" << maxErr << endl;
	cout << "Avg Err:" << avgErr / errArr.size() << endl;
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


