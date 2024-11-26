#pragma once
#include "common.h"
struct TSPPath
{
	uint64 count;
	double fullLength;
	std::vector<uint64> path;

	void print();
};


typedef TSPPath(*TSPSolveFunction)(std::vector<double> matrix, uint64 width);

// Solving functions
TSPPath solveTSPBruteForce(std::vector<double> matrix, uint64 width);
TSPPath solveTSPNearestNeighbour(std::vector<double> matrix, uint64 width);
TSPPath solveTSPChristofides(std::vector<double> matrix, uint64 width);
TSPPath solveTSPMinSpanTree(std::vector<double> matrix, uint64 width);
