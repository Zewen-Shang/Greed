#include "Heap.h"

using namespace std;

Heap::Heap(int size)
{
	arr = new HeapNode*[size];
}

Heap::~Heap()
{
	delete[] arr;
}

void Heap::insert(HeapNode *n)
{
	cnt++;
	arr[cnt] = n;
	n->pos = cnt;
	n->heap = this;
	up(cnt);
}

void Heap::change(int i, int j)
{
	assert(i >= 1 && i <= cnt && j >= 1 && j <= cnt);
	swap(arr[i], arr[j]);
	arr[i]->pos = i;
	arr[j]->pos = j;
}

void Heap::up(int pos)
{
	int i = pos / 2;
	while (i && arr[i]->value > arr[pos]->value) {
		change(i, pos);
		pos = i;
		i = pos / 2;
	}
}

void Heap::down(int pos)
{
	int son = pos * 2;
	if (son + 1 <= cnt && arr[son+1]->value < arr[son]->value)son++;
	while (son <= cnt && arr[son]->value < arr[pos]->value) {
		change(son, pos);
		pos = son;
		son = pos * 2;
		if (son + 1 <= cnt && arr[son+1]->value < arr[son]->value)son++;
	}
}

HeapNode* Heap::top()
{
	assert(cnt >= 1);
	return arr[1];
}

void Heap::pop()
{
	change(1, cnt);
	cnt--;
	down(1);
}

void HeapNode::update(double newValue)
{
	if (newValue > value) {
		value = newValue;
		heap->down(pos);
	}
	else {
		value = newValue;
		heap->up(pos);
	}
}

bool HeapNode::inHeap()
{
	return pos > 0 && pos <= heap->cnt;
}
