#include "Primes.h"

Vertex::Vertex()
{
}

Vertex::Vertex(int id, Vector6d pos):id(id),pos(pos)
{
}

//仔细检查
void Vertex::mapTo(Vertex* target)
{
	assert(state == Invalid);
	for (auto i = edge.begin(); i != edge.end(); i++) {
		Edge* e = (*i);
		//这条边被折叠了
		if (e->checkVertex(target) || !e->valid())continue;
		Vertex* another = e->anotherVertex(this);
		Edge* e2 = target->isNeighbour(another);
		if (e2) {
			//两条边需要压缩在一起
			e->state = Invalid;
			e->mapTo(e2);
		}
		else {
			//改变边的指向，将边加入另一个顶点
			e->changeVertex(this, target);
			target->edge.push_back(e);
		}
	}
	for (auto e : target->edge) {
		if (e->valid() && e->inHeap()) {
			e->updateCost();
		}
	}
}

Edge* Vertex::isNeighbour(Vertex* other) const
{
	for (auto i = edge.begin(); i != edge.end(); i++) {
		if ((*i)->checkVertex(other))
			return (*i);
	}
	return nullptr;
}

vector<Tetra*> Vertex::getTetras() const
{
	set<Tetra*>S;
	for (auto e : edge) {
		if (!e->valid())continue;
		for (auto t : e->tetra) {
			if (t->valid())S.insert(t);
		}
	}
	vector<Tetra*> ans;
	for (auto t : S) {
		ans.push_back(t);
	}
	return ans;
}

Edge::Edge(Vertex* v0, Vertex* v1):v0(v0),v1(v1) {};


void Edge::collapse()
{
	//cout << "v0:" << v0->id << ", v1:" << v1->id << ",  to  " << v0->id << endl;
	//删除自己
	kill();
	//删除所有相关面
	for (auto t = tetra.begin(); t != tetra.end(); t++) {
		if ((*t)->valid()) {
			(*t)->kill();
		}
	}
	v0->pos = midPos;
	v1->state = Invalid;
	v1->mapTo(v0);
}

bool Edge::checkVertex(Vertex* v)
{
	return v0 == v || v1 == v;
}

Vertex* Edge::anotherVertex(Vertex* v)
{
	assert(checkVertex(v));
	if (v == v0)return v1;
	else return v0;
}

bool Edge::checkTetra(Tetra* t)
{
	for (auto i = tetra.begin(); i != tetra.end(); i++) {
		if ((*i)->state == Valid && (*i) == t)return true;
	}
	return false;
}

void Edge::adjust()
{
	if (v0->id > v1->id)
		swap(v0, v1);
}

void Edge::changeVertex(Vertex* from, Vertex* to)
{
	assert(checkVertex(from));
	if (v0 == from)v0 = to;
	else v1 = to;
}

void Edge::mapTo(Edge* target)
{
	assert(this->state == Invalid);
	for (auto i = tetra.begin(); i != tetra.end(); i++) {
		if (!(*i)->valid())continue;
		Tetra* t = (*i);
		t->changeEdge(this, target);
		target->tetra.push_back(t);
	}
}

double Edge::len()
{
	return (v1->pos - v0->pos).norm();
	
}

void Edge::updateCost()
{
	assert(valid());
	double newValue = calcCost();
	update(newValue);
}

void Edge::InitCost(Heap& H)
{
	assert(valid());
	heap = &H;
	value = calcCost();
	H.insert(this);
}

double Edge::calcCost() {
	//Vector3d alterPos[3];
	//Cofficient C = v0->C + v1->C;
	//double newValue = 100000;
	//if ( abs(C.A.determinant()) >= 0.000002) {
	//	Matrix3d AI = C.A.inverse();
	//	midPos = -AI * C.b;
	//	newValue = -C.b.transpose() * AI * C.b + C.c;
	//}
	//else {
	//	alterPos[0] = v0->pos;
	//	alterPos[1] = (v0->pos + v1->pos) / 2;
	//	alterPos[2] = v1->pos;

	//	for (auto pos : alterPos) {
	//		double error = C.getError(pos);
	//		if (newValue > error) {
	//			newValue = error;
	//			midPos = pos;
	//		}
	//	}
	//}
	midPos = (v0->pos + v1->pos) / 2;
	return len();
}

bool Edge::checkInversion() const
{
	vector<Tetra*>teralsOfV0 = v0->getTetras(), facesOfV1 = v1->getTetras();
	for (auto t : teralsOfV0) {
		if (t->checkInversion(v0, midPos)) {
			return true;
		}
	}
	for (auto t : facesOfV1) {
		if (t->checkInversion(v1, midPos)) {
			return true;
		}
	}
	return false;
}

Tetra::Tetra(Edge* es[]){
	for (int i = 0; i < 6; i++) {
		this->es[i] = es[i];
	}
};

bool Tetra::checkEdge(Edge* e)
{
	for (int i = 0; i < 6; i++) {
		if (es[i] == e)return 1;
	}
	return 0;
}

vector<int> Tetra::getVertexIds() const
{
	set<int> S;
	vector<int>ans;
	for (int i = 0; i < 6; i++) {
		S.insert(es[i]->v0->id);
		S.insert(es[i]->v1->id);
	}
	for (auto i = S.begin(); i != S.end(); i++) {
		ans.push_back((*i));
	}
	return ans;
}

vector<Vertex*> Tetra::getVertexs() const
{
	set<Vertex*>S;
	for (int i = 0; i < 6; i++) {
		S.insert(es[i]->v0);
		S.insert(es[i]->v1);
	}

	vector<Vertex*>ans;
	for (auto v : S) {
		ans.push_back(v);
	}
	if (ans.size() != 4) {
		cout << "fuck" << endl;
	}
	return ans;;
}

bool Tetra::checkInversion(Vertex* v, Vector6d midPos)
{
	assert(valid());
	vector<Vertex*> vs = getVertexs();
	vector<Vector3d>stablePos;
	for (auto tv : vs) {
		if (tv != v) {
			stablePos.push_back(tv->pos.block<3,1>(1,1));
		}
	}

	Vector3d normal = (stablePos[0] - stablePos[1]).cross(stablePos[0] - stablePos[2]).normalized();
	Vector3d p1 = midPos.block<3,1>(1,1) - stablePos[0], p2 = v->pos.block<3,1>(1,1) - stablePos[0];

	return p1.dot(normal) * p2.dot(normal) < 0;

	//Vector3d oldPos[3], newPos[3];
	//vector<Vertex*> vs = getVertexs();
	//for (int i = 0; i < 3; i++) {
	//	newPos[i] = oldPos[i] = vs[i]->pos;
	//	if (vs[i] == v)newPos[i] = midPos;
	//}

	//Vector3d newNormal = (newPos[0] - newPos[1]).cross(newPos[0] - newPos[2]);
	//Vector3d oldNormal = (oldPos[0] - oldPos[1]).cross(oldPos[0] - oldPos[2]);
	//return newNormal.dot(oldNormal)<0;
}

Vector6d Tetra::getCenter() const
{
	vector<Vertex*>vs = getVertexs();
	Vector6d ans = Vector6d::Zero();
	for (auto v : vs) {
		ans += v->pos;
	}
	ans /= 4;
	return ans;
}


void Tetra::changeEdge(Edge* from,Edge*to)
{
	assert(checkEdge(from));
	for (int i = 0; i < 6; i++) {
		if (es[i] == from)es[i] = to;
	}
}

