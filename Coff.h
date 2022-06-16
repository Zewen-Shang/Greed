#pragma once

#include <Eigen/dense>
#include <vector>

using namespace std;

typedef Eigen::Matrix<double, 4, 4> Matrix4d;
typedef Eigen::Matrix<double, 4, 1> Vector4d;

class Coff
{
public:
	Coff();
	Coff(Matrix4d A, Vector4d b, double c);
	Matrix4d A;
	Vector4d b;
	double c;
	Coff operator+(const Coff& other) {
		return Coff(this->A + other.A, this->b + other.b, this->c + other.c);
	}

	Coff operator*(const double x) {
		return Coff(this->A * x, this->b * x, this->c * x);
	}
	double getError(Vector4d x);
};

vector<Vector4d> smtOrth(vector<Vector4d>A);