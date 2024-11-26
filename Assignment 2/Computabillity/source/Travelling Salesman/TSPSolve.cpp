#include "TSPSolve.h"
#include <iostream>
#include <algorithm>

void TSPPath::print()
{
	std::cout << "Path: " << std::endl << " ( " << path[0];
	for (uint64 i = 1; i < path.size(); ++i)
		std::cout << ", " << path[i];
	std::cout << ')' << std::endl;
	std::cout << "Path Length: " << fullLength << std::endl;
	std::cout << "Node Count: " << count << std::endl;
}


TSPPath solveTSPBruteForce(std::vector<double> matrix, uint64 width)
{
	if (matrix.size() == 0 || width == 0)
		throw std::runtime_error("Attempted to solve an uninitialized TSPMatrix");


	std::vector<uint64> indices;
	for (uint64 i = 1; i < width; ++i)
		indices.push_back(i);

	TSPPath path;
	path.count = width;
	path.fullLength = DBL_MAX;
	path.path = std::vector<uint64>(width);
	std::vector<uint64> currentPath(width);
	do
	{
		double pathLength = 0;
		uint64 matX = 0;
		uint64 matY = indices[0];
		for (uint64 i = 0; i < indices.size(); ++i)
		{
			matY = indices[i];
			if (matY * width + matX >= width * width) throw std::runtime_error("Hello");
			pathLength += matrix[matY * width + matX];
			currentPath[i] = matX;
			matX = indices[i];
		}
		if (matY * width + matX >= width * width) throw std::runtime_error("Hello2");
		pathLength += matrix[matY * width];
		currentPath[width - 1] = matY;

		if (pathLength < path.fullLength)
		{
			path.fullLength = pathLength;
			std::copy(currentPath.begin(), currentPath.end(), path.path.begin());
		}
	} while (std::next_permutation(indices.begin(), indices.end()));

	return path;
}

TSPPath solveTSPNearestNeighbour(std::vector<double> matrix, uint64 width)
{
	if (matrix.size() == 0 || width == 0)
		throw std::runtime_error("Attempted to solve an uninitialized TSPMatrix");

	std::vector<bool> visited(width);
	visited[0] = true;

	uint64 row = 0;
	TSPPath path;
	path.fullLength = 0;
	path.path = std::vector<uint64>(width);
	path.path[0] = 0;
	for (path.count = 1; path.count < width;)
	{
		double min = DBL_MAX;
		uint64 minI = 0;
		for (uint64 i = 1; i < width; ++i)
		{
			if (!visited[i])
			{
				if (matrix[row * width + i] < min)
				{
					minI = i;
					min = matrix[row * width + i];
				}
			}
		}
		row = minI;
		path.path[path.count++] = minI;
		path.fullLength += min;
		visited[row] = true;
	}
	path.fullLength += matrix[path.path[path.count - 1] * width];

	return path;
}

std::vector<std::pair<uint64, uint64>> _getMinSpanTree(std::vector<double> matrix, uint64 width)
{
	std::vector<std::pair<uint64, uint64>> minSpanTree;
	std::vector<bool> visited(width);
	visited[0] = true;
	uint64 minI = 1;

	// First pair
	for (uint64 i = 2; i < width; ++i)
	{
		if (matrix[i] < matrix[minI])
			minI = i;
	}
	minSpanTree.push_back(std::make_pair((uint64)0, minI));
	visited[minI] = true;


	// Recursively find the shortest distance between a visited node and
	//    a non-visited node until all nodes have been visited
	while (minSpanTree.size() != width - 1)
	{
		std::pair<uint64, uint64> minF;
		double min = DBL_MAX;
		for (uint64 i = 0; i < width; ++i)
		{
			if (visited[i])
			{
				for (uint64 j = 1; j < width; ++j)
				{
					if (!visited[j])
					{
						if (matrix[i * width + j] < min)
						{
							min = matrix[i * width + j];
							minF = std::make_pair(i, j);
						}
					}
				}
			}
		}
		minSpanTree.push_back(minF);
		visited[minF.second] = true;
	}

	return minSpanTree;
}

TSPPath solveTSPMinSpanTree(std::vector<double> matrix, uint64 width)
{
	std::vector<bool> visited(width);
	visited[0] = true;

	std::vector<std::pair<uint64, uint64>> minSpanTree = _getMinSpanTree(matrix, width);


	// Remove any duplicate visits to nodes 
	TSPPath path;
	path.path.push_back(0);
	path.fullLength = 0;
	path.count = width;

	for (uint64 i = 0; i < minSpanTree.size(); ++i)
	{
		for (uint64* pair = &minSpanTree[i].first; pair < &minSpanTree[i].first + 2; ++pair)
		{
			if (!visited[*pair])
			{
				visited[*pair] = true;
				path.path.push_back(*pair);
				path.fullLength += matrix[path.path[path.path.size() - 2] * width + (*pair)];
			}
		}
	}
	path.fullLength += matrix[path.path[path.path.size() - 1] * width];
	return path;
}

TSPPath solveTSPChristofides(std::vector<double> matrix, uint64 width)
{
	std::vector<std::pair<uint64, uint64>> minSpanTree = _getMinSpanTree(matrix, width);

	//Find nodes with an odd number of connections.
	std::vector<uint64> connectionCount(width);
	for (uint64 i = 0; i < minSpanTree.size(); ++i)
	{
		++connectionCount[minSpanTree[i].first];
		++connectionCount[minSpanTree[i].second];
	}
	std::vector<uint64> oddNodes;
	for (uint64 i = 0; i < width; ++i)
	{
		if (connectionCount[i] % 2 == 1)
			oddNodes.push_back(i);
	}

	// Find the set of pairs of oddNodes with the minimum total distance
	std::vector<uint64> oddNodePairs(oddNodes.size());
	double min = DBL_MAX;
	do
	{
		double distance = 0;
		for (uint64 i = 0; i < oddNodes.size() / 2; ++i)
		{
			distance += matrix[oddNodes[i * 2] * width + oddNodes[i * 2 + 1]];
		}

		if (distance < min)
		{
			min = distance;
			std::copy(oddNodes.begin(), oddNodes.end(), oddNodePairs.begin());
		}
	} while (std::next_permutation(oddNodes.begin(), oddNodes.end()));

	// Add the oddNodePairs to the minSpanTree
	for (uint64 i = 0; i < oddNodePairs.size() / 2; ++i)
	{
		auto it = minSpanTree.begin();
		while (it->first != oddNodePairs[i * 2])
		{
			if (it->second == oddNodePairs[i * 2])
			{
				++it;
				break;
			}
			++it;
		}
		minSpanTree.insert(it, std::make_pair(oddNodePairs[i * 2], oddNodePairs[i * 2 + 1]));
	}

	TSPPath path;
	std::vector<bool> visited(width);
	visited[0] = true;
	path.path.push_back(0);
	path.fullLength = 0;
	path.count = width;

	for (uint64 i = 0; i < minSpanTree.size(); ++i)
	{
		for (uint64* pair = &minSpanTree[i].first; pair < &minSpanTree[i].first + 2; ++pair)
		{
			if (!visited[*pair])
			{
				visited[*pair] = true;
				path.path.push_back(*pair);
				path.fullLength += matrix[path.path[path.path.size() - 2] * width + (*pair)];
			}
		}
	}
	path.fullLength += matrix[path.path[path.path.size() - 1] * width];
	return path;
}