#pragma once
#include "common.h"
#include "TSPSolve.h"
#include <iostream>
#include <functional>

// x1, y1, x2, y2
typedef double (*DistanceFunction2D)(double, double, double, double);
//x1, y1, z1, x2, y2, z2
typedef double (*DistanceFunction3D)(double, double, double, double, double, double);

typedef void (*TSPMatrixInsertionFunction)(uint64, double*, double*);

// Nearest Integer
double nint(double value);

// Distance Functions
double euclideanDistance2D(double x1, double y1, double x2, double y2);
double euclideanDistance3D(double x1, double y1, double z1, double x2, double y2, double z2);
double manhattenDistance2D(double x1, double y1, double x2, double y2);
double manhattenDistance3D(double x1, double y1, double z1, double x2, double y2, double z2);
double maximumDistance2D(double x1, double y1, double x2, double y2);
double maximumDistance3D(double x1, double y1, double z1, double x2, double y2, double z2);
double geographicalDistance(double latitude1, double longitude1, double latitude2, double longitude2);
double pseudoEuclidianDistance2D(double x1, double y1, double x2, double y2);


class TSPMatrix
{
protected:
	uint64 matrixWidth;
	std::vector<double> matrix;

	// Should be called by child class constructors
	void _initializeMatrix(std::vector<double> coords, uint8 step, std::function<void(uint64,double*,double*)> fun);
	
public:
	TSPMatrix(std::vector<double> distances, uint64 width);
	TSPMatrix();
	~TSPMatrix();

	TSPPath solve(TSPSolveFunction solver);
	void print(uint8 precision = 2);
	uint64 getWidth() { return matrixWidth; };
};

TSPMatrix createTSPMatrix(const char* filepath);

class TSPMatrix2D : public TSPMatrix
{
public:
	TSPMatrix2D(std::vector<double> positions, DistanceFunction2D distanceFunction);
};

class TSPMatrix3D : public TSPMatrix
{
public:
	TSPMatrix3D(std::vector<double> positions, DistanceFunction3D distanceFunction);
};