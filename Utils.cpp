#include "Utils.h"

void sort3(int& id0, int& id1, int& id2) {
	if (id0 > id1)
		swap(id0, id1);
	if (id1 > id2)
		swap(id1, id2);
	if (id0 > id1)
		swap(id0, id1);
}

void sort4(int& id0, int& id1, int& id2, int& id3) {
	int arr[4] = { id0,id1,id2,id3 };
	sort(arr, arr + 4);
	id0 = arr[0];
	id1 = arr[1];
	id2 = arr[2];
	id3 = arr[3];
}