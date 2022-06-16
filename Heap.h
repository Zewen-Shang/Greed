#pragma once

#include <algorithm>
#include <assert.h>

class Heap;
class HeapNode;

class HeapNode {
public:
	int pos = 0;
	double value;
	Heap* heap;
	void update(double newValue);
	bool inHeap();
};

class Heap
{
public:
	HeapNode** arr;
	int cnt = 0;
	Heap(int size);
	~Heap();
	void insert(HeapNode *n);
	void change(int i, int j);
	void up(int pos);
	void down(int pos);
	HeapNode* top();
	void pop();
};

