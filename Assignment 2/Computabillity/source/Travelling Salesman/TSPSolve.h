#pragma once
#include "common.h"
struct TSPPath
{
	uint64 count;
	double fullLength;
	std::vector<uint64> path;

	void print(bool fullPath = true);
};


typedef TSPPath(*TSPSolveFunction)(std::vector<double> matrix, uint64 width);

// Can be used by solving algorithms to provide extra options
extern uint8 solveAlgorithmVariant;

// Solving functions
TSPPath solveTSPBruteForce(std::vector<double> matrix, uint64 width);
TSPPath solveTSPNearestNeighbour(std::vector<double> matrix, uint64 width);
TSPPath solveTSPChristofides(std::vector<double> matrix, uint64 width);
TSPPath solveTSPMinSpanTree(std::vector<double> matrix, uint64 width);

// Christofides Minimum Matching algorithms
typedef std::vector<std::pair<uint64, uint64>> PairList;
typedef PairList(*ChristofidesMinMatchingFunctionPointer) (std::vector<uint64> oddNodes, std::vector<double> matrix, uint64 width);

std::vector<uint64> findOddDegreeNodes(PairList minSpanTree);

PairList nearestNeighbourMinMatching (std::vector<uint64> oddNodes, std::vector<double> matrix, uint64 width);
PairList bruteForceMinMatching (std::vector<uint64> oddNodes, std::vector<double> matrix, uint64 width);
PairList hungarianMinMatching (std::vector<uint64> oddNodes, std::vector<double> matrix, uint64 width);