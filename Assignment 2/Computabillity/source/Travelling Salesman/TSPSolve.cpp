#include "TSPSolve.h"
#include <iostream>
#include <algorithm>
#include <functional>

ChristofidesMinMatchingFunctionPointer christofidesMinMatchFunctions[] =
{
	nearestNeighbourMinMatching,
	bruteForceMinMatching,
	hungarianMinMatching
};

uint8 solveAlgorithmVariant = 0;

void TSPPath::print(bool fullPath)
{
	if (fullPath)
	{
		std::cout << " Path: " << std::endl << " ( " << path[0];
		for (uint64 i = 1; i < path.size(); ++i)
			std::cout << ", " << path[i];
		std::cout << ')' << std::endl << std::endl;
	}
	std::cout << " Path Length: " << fullLength << std::endl;
	std::cout << " Node Count: " << count << std::endl;
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

PairList _getMinSpanTree(std::vector<double> matrix, uint64 width)
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

std::vector<uint64> findOddDegreeNodes(PairList minSpanTree)
{
	//Find nodes with an odd number of connections.
	std::vector<uint64> connectionCount(minSpanTree.size() + 1);
	for (uint64 i = 0; i < minSpanTree.size(); ++i)
	{
		++connectionCount[minSpanTree[i].first];
		++connectionCount[minSpanTree[i].second];
	}
	std::vector<uint64> oddNodes;
	for (uint64 i = 0; i < connectionCount.size(); ++i)
	{
		if (connectionCount[i] % 2 == 1)
			oddNodes.push_back(i);
	}
	return oddNodes;
}

PairList nearestNeighbourMinMatching(std::vector<uint64> oddNodes, std::vector<double> matrix, uint64 width)
{
	PairList oddPairs(oddNodes.size() / 2);
	std::vector<bool> matched(oddNodes.size());
	uint64 count = 0;
	for (uint64 i = 0; count != oddNodes.size() / 2; ++i)
	{
		if (matched[i]) continue;
		double min = DBL_MAX;
		uint64 minJ;
		for (uint64 j = 0; j < oddNodes.size(); ++j)
		{
			if (matched[j]) continue;
			if (j == i) continue;

			double distance = matrix[oddNodes[i] * width + oddNodes[j]];
			if (distance < min)
			{
				min = distance;
				minJ = j;
			}
		}
		if (min == DBL_MAX) continue;
		

		matched[i] = true;
		matched[minJ] = true;
		oddPairs[count].first = oddNodes[i];
		oddPairs[count++].second = oddNodes[minJ];
	}
	return oddPairs;
}

PairList bruteForceMinMatching(std::vector<uint64> oddNodes, std::vector<double> matrix, uint64 width)
{
	// TODO: This is a far from optimal brute force method, it doesn't take
	//     take into account that the pair (a,b) == (b,a) nor that the set
	//     ((a,b), (c,d), ...) == ((c,d), (a,b), ...) and so does a lot more
	//     work than required.
	// Find the set of pairs of oddNodes with the minimum total distance
	std::vector<uint64> bestOddPairs(oddNodes.size());
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
			// Keep track of the smallest overall distance,
			//   and it's pair list.
			min = distance;
			std::copy(oddNodes.begin(), oddNodes.end(), bestOddPairs.begin());
		}
	} while (std::next_permutation(oddNodes.begin(), oddNodes.end()));

	PairList oddPairs(oddNodes.size() / 2);
	for (uint64 i = 0; i < oddNodes.size() / 2; ++i)
	{
		oddPairs[i].first = oddNodes[i * 2];
		oddPairs[i].second = oddNodes[i * 2 + 1];
	}
	return oddPairs;
}

PairList hungarianMinMatching(std::vector<uint64> oddNodes, std::vector<double> matrix, uint64 width)
{
	std::vector<bool> matched(oddNodes.size());
	uint64 oddWidth = oddNodes.size();
	std::vector<double> oddMatrix(oddWidth * oddWidth);

	// Construct the sub-matrix of matrix only containing the oddNodes where we
	//     replace the distance between a node and itself with DBL_MAX to prevent
	//     these being the shortest pairs with themselves.
	for (uint64 i = 0; i < oddNodes.size(); ++i)
	{
		for (uint64 j = 0; j < oddNodes.size(); ++j)
		{
			if (i == j)
				oddMatrix[i * oddWidth + j] = DBL_MAX;
			else
				oddMatrix[i * oddWidth + j] = matrix[oddNodes[i] * width + oddNodes[j]];
		}
	}

	// Subtract minimum value in each row from each value in the row.
	for (uint64 row = 0; row < oddWidth; ++row)
	{
		// Find minimum
		double min = DBL_MAX;
		for (uint64 i = 0; i < oddWidth; ++i)
		{
			if (oddMatrix[row * oddWidth + i] < min)
				min = oddMatrix[row * oddWidth + i];
		}
		// Subtract
		for (uint64 i = 0; i < oddWidth; ++i)
			oddMatrix[row * oddWidth + i] -= min;
	}

	// Repeat for columns.
	for (uint64 col = 0; col < oddWidth; ++col)
	{
		// Find minimum
		double min = DBL_MAX;
		for (uint64 i = 0; i < oddWidth; ++i)
		{
			if (oddMatrix[i * oddWidth + col] < min)
				min = oddMatrix[i * oddWidth + col];
		}
		// Subtract
		for (uint64 i = 0; i < oddWidth; ++i)
			oddMatrix[i * oddWidth + col] -= min;
	}

	// This counts the minimum number of lines you would
	//    have to draw to cover all 0's in oddMatrix.
	uint64 minimum0CoveringLines = 0;
	// I don't think wer can ever actually hit this condition
	//    but I'm not sure and we'd have to calculate the covered
	//    lines either way before we can do the next step..
	while (minimum0CoveringLines < oddWidth)
	{
		minimum0CoveringLines = 0;
		// First we find the minimum set of rows and
		//    columns to cover all 0s.
		std::vector<uint64> row0s(oddWidth);
		std::vector<uint64> col0s(oddWidth);

		//This finds the number of 0's in each row and column.
		uint64 max = 0;
		bool rowLarger = true;
		for (uint64 row = 0; row < oddWidth; ++row)
		{
			for (uint64 col = 0; col < oddWidth; ++col)
			{
				if (oddMatrix[row * oddWidth + col] == 0)
				{
					++row0s[row];
					++col0s[col];
					if (row0s[row] >= col0s[col])
					{
						if (row0s[row] > max)
						{
							rowLarger = true;
							max = row0s[row];
						}
					}
					else
					{
						if (col0s[col] > max)
						{
							rowLarger = false;
							max = col0s[col];
						}
					}
				}
			}
		}

		// We iterate through whichever had the highest single row / column of 0s
		//    and where a column crosses a row whichever has the lesser count is 
		//    decremented.
		{
			std::vector<uint64>* bigVec = rowLarger ? &row0s : &col0s;
			std::vector<uint64>* smallVec = rowLarger ? &col0s : &row0s;

			for (uint64 big = 0; big < oddWidth; ++big)
			{
				for (uint64 small = 0; small < oddWidth; ++small)
				{
					uint64 row = rowLarger ? big : small;
					uint64 col = rowLarger ? small : big;
					if (oddMatrix[row * oddWidth + col] == 0)
					{
						if ((*bigVec)[big] >= (*smallVec)[small])
							--((*smallVec)[small]);
						else
							--((*bigVec)[big]);
					}
				}
			}
		}
		// Now we have 2 lists where the total of them combined
		//    adds up to the number of 0s, and we know that if a
		//    row0Count or col0Count > 0 that row or column is covered
		//    it's easy to try this out on paper, it's how I worked it out!
		std::vector<bool> coveredRows(oddWidth);
		std::vector<bool> coveredCols(oddWidth);
		for (uint64 i = 0; i < oddWidth; ++i)
		{
			if (row0s[i] > 0)
			{
				coveredRows[i] = true;
				++minimum0CoveringLines;
			}
			if (col0s[i] > 0)
			{
				coveredCols[i] = true;
				++minimum0CoveringLines;
			}
		}

		// If the minimum number of covering lines is
		//    equal to the width of our matrix then
		//    the combination of positions of 0's where
		//    no two 0's share a row or column is our
		//    perfect matching.
		if (minimum0CoveringLines >= oddWidth) 
			break;


		// This is an iteration of the Hungarian algorithm,
		//    I don't think I am really able to explain
		//    why it works in words honestly.. But I have
		//    outlined the steps

		// Find the minimum value that is not covered
		double min = DBL_MAX;
		for (uint64 row = 0; row < oddWidth; ++row)
		{
			if (coveredRows[row]) continue;
			for (uint64 col = 0; col < oddWidth; ++col)
			{
				if (coveredCols[col]) continue;

				if (oddMatrix[row * oddWidth + col] < min)
					min = oddMatrix[row * oddWidth + col];
			}
		}

		// Subtract the minimum from the uncovered rows.
		for (uint64 row = 0; row < oddWidth; ++row)
		{
			if (coveredRows[row]) continue;
			for (uint64 i = 0; i < oddWidth; ++i)
				oddMatrix[row * oddWidth + i] -= min;
		}

		// Add the minimum to the covered columns.
		for (uint64 col = 0; col < oddWidth; ++col)
		{
			if (!coveredCols[col]) continue;
			for (uint64 i = 0; i < oddWidth; ++i)
				oddMatrix[i * oddWidth + col] += min;
		}
	}

	// Convert each 0 in our matrix to a pair removing
	//    any duplicates f.e. (a,b), (b,a)
	PairList oddPairs;
	for (uint64 row = 0; row < oddWidth; ++row)
	{
		for (uint64 col = 0; col < oddWidth; ++col)
		{
			if (oddMatrix[row * oddWidth + col] == 0)
			{
				bool found = false;
				for (uint64 i = 0; i < oddPairs.size(); ++i)
				{
					if (oddPairs[i].first == col && oddPairs[i].second == row)
						found = true;
				}

				if (!found)
				{
					oddPairs.push_back(std::make_pair(row, col));
				}
			}
		}
	}

	// We, will use an elimination method by recursively removing
	//    any odd pair that contains the only occurance of a node
	//    and adding the single pair they exist within to the final 
	//    list. Where we do not have a node that occurs only once
	//    we fall back on brute force by using this procees iteratively on
	//    the lowest occuring node and solving the sub problem this creates,
	//    if we can not solve it then we move onto the next node. It's quite
	//    difficult to explain, especially given it's recursive nature however
	//    it appears to work well. Recursion finishes when we either find the 
	//    solution or determine one doesn't exist meaning we need to try a 
	//    different iteration on a previous recursion.
	PairList finalList;
	// This function certainly risks stack overflow... We are storing most of
	//    our data on the heap so hopefully it's not a problem!
	std::function<bool(PairList, std::vector<bool>, uint64)> recursionFunction2 = [&](PairList candidate, std::vector<bool> ruledOut, uint64 depth) -> bool
		{
			// If candidate is the right size we're done and can copy
			//    the PairList then return true to unwind the stack.
			if (candidate.size() == oddWidth / 2)
			{
				for (uint64 i = 0; i < candidate.size(); ++i)
				{
					finalList.push_back(std::make_pair(oddNodes[candidate[i].first], oddNodes[candidate[i].second]));
				}
				return true;
			}

			// Calculate occurances
			uint64 maxOccurances = 0;
			std::vector<uint64> occurances(oddNodes.size());
			for (uint64 i = 0; i < oddPairs.size(); ++i)
			{
				if (ruledOut[i]) continue;
				occurances[oddPairs[i].first]++;
				occurances[oddPairs[i].second]++;
				if (occurances[oddPairs[i].first] > occurances[oddPairs[i].second])
				{
					if (occurances[oddPairs[i].first] > maxOccurances)
					{
						maxOccurances = occurances[oddPairs[i].first];
					}
				}
				else
				{
					if (occurances[oddPairs[i].second] > maxOccurances)
					{
						maxOccurances = occurances[oddPairs[i].second];
					}
				}
			}

			// Simplest possibillity is that there are one or more nodes that 
			//    only occur once, this means they have to be in the solution
			//    as every node will appear once in the solution.

			// Check if there are any nodes with a single occurance.
			bool singleOccuranceNodes = false;
			for (uint64 i = 0; i < occurances.size(); ++i)
			{
				if (occurances[i] != 1) continue;

				singleOccuranceNodes = true;
				break;
			}

			if (singleOccuranceNodes)
			{
				std::vector<bool> subRuledOut(ruledOut.size());
				std::copy(ruledOut.begin(), ruledOut.end(), subRuledOut.begin());
				PairList subCandidate(candidate.size());
				std::copy(candidate.begin(), candidate.end(), subCandidate.begin());

				for (uint64 i = 0; i < occurances.size(); ++i)
				{
					if (occurances[i] != 1) continue;
					
					// Find the single pair containing our node and get
					//    the node it is paired with.
					uint64 nodeToRuleOut;
					for (uint64 j = 0; j < oddPairs.size(); ++j)
					{
						if (subRuledOut[j]) continue;
						if (!(oddPairs[j].first == i || oddPairs[j].second == i)) continue;
						nodeToRuleOut = (oddPairs[j].first == i) ? oddPairs[j].second : oddPairs[j].first;
						
						// Add the pair to the candidate list for the sub problem.
						subCandidate.push_back(oddPairs[j]);
						break;
					}

					//Rule out all pairs which contain nodeToRuleOut.
					for (uint64 j = 0; j < oddPairs.size(); ++j)
					{
						if (subRuledOut[j]) continue;
						if (!(oddPairs[j].first == nodeToRuleOut || oddPairs[j].second == nodeToRuleOut)) continue;

						subRuledOut[j] = true;
					}
					occurances[nodeToRuleOut] = 0;
				}
				// Finally attempt the subProblem if it fails then the
				//    current problem is unsolvable and we revert back
				//    up the recursion stack.
				return recursionFunction2(subCandidate, subRuledOut, depth + 1);
			}


			// Iterate through each sub problem for each node
			//    that only occurs once in oddPairs.
			for (uint64 o = 2; o <= maxOccurances; ++o)
			{
				for (uint64 i = 0; i < occurances.size(); ++i)
				{
					if (occurances[i] == o)
					{

						// For each pair containing our node get the other node in 
						//    that pair (nodeToRuleOut), initialize the subproblem
						//    where the pair is included in the solution and attempt
						//    to solve the sub problem
						uint64 co = 0;
						for (uint64 j = 0; co < o; ++j)
						{
							if (ruledOut[j]) continue;
							if (!(oddPairs[j].first == i || oddPairs[j].second == i)) continue;

							std::vector<bool> subRuledOut(ruledOut.size());
							std::copy(ruledOut.begin(), ruledOut.end(), subRuledOut.begin());
							PairList subCandidate(candidate.size());
							std::copy(candidate.begin(), candidate.end(), subCandidate.begin());

							// We don't know which node is which in the pair!
							uint64 nodeToRuleOut = (oddPairs[j].second == i) ? oddPairs[j].first : oddPairs[j].second;
							subCandidate.push_back(oddPairs[j]);

							// Rule out all pairs which contain nodeToRuleOut
							for (uint64 j = 0; j < oddPairs.size(); ++j)
							{
								if (ruledOut[j]) continue;
								if (!(oddPairs[j].first == nodeToRuleOut || oddPairs[j].second == nodeToRuleOut)) continue;

								subRuledOut[j] = true;
							}
							occurances[nodeToRuleOut] = 0;

							// Finally attempt the sub-problem
							if (recursionFunction2(subCandidate, subRuledOut, depth + 1))
								return true;
							++co;
						}

					}
				}
			}

			// If no node occurs only once or none of the sub 
			//    problems produce a solution return false.
			return false;
		};

	// Start the recursionFunction
	recursionFunction2(PairList(), std::vector<bool>(oddPairs.size()), 0);

	return finalList;
}

TSPPath solveTSPChristofides(std::vector<double> matrix, uint64 width)
{
	auto minSpanTree = _getMinSpanTree(matrix, width);

	auto oddNodes = findOddDegreeNodes(minSpanTree);

	auto oddPairs = christofidesMinMatchFunctions[solveAlgorithmVariant](oddNodes, matrix, width); 

	// Add the oddNodePairs to the minSpanTree
	for (uint64 i = 0; i < oddPairs.size(); ++i)
	{
		auto it = minSpanTree.begin();
		while (it->first != oddPairs[i].first)
		{
			if (it->second == oddPairs[i].first)
			{
				// Insert the oddPair after the current node,
				//    the other case is when the while loop ends
				//    in which case the oddPair is inserted before.
				++it;
				break;
			}
			++it;
		}
		minSpanTree.insert(it, oddPairs[i]);
	}
	
	TSPPath path;
	std::vector<bool> visited(width);
	std::function<bool(std::vector<uint64>, uint64)> recursionFunction =
		[&](std::vector<uint64> walk, uint64 depth) -> bool
		{
			if (path.path.size() == width)
			{
				return true;
			}
			uint64 n = *(walk.end() - 1);
			for (uint64 i = minSpanTree.size() - 1; i < minSpanTree.size(); --i)
			{
				if (!(minSpanTree[i].second == n || minSpanTree[i].first == n)) continue;
				uint64 other = (minSpanTree[i].second == n) ? minSpanTree[i].first : minSpanTree[i].second;
				if (visited[other]) continue;

				std::vector<uint64> subWalk(walk.size());
				std::copy(walk.begin(), walk.end(), subWalk.begin());
				subWalk.push_back(other);
				visited[other] = true;
				path.path.push_back(other);
				path.fullLength += matrix[n * width + other];
				if (recursionFunction(subWalk, depth + 1)) 
					return true;
			}

			// No connected nodes so we go back up the tree
			return false;
		};

	path.path.push_back(0);
	visited[0] = true;
	auto last = minSpanTree.end() - 1;
	path.path.push_back(last->second);
	path.fullLength = matrix[last->second * width];
	visited[last->second] = true;

	std::vector<uint64> walk(path.path.size());
	std::copy(path.path.begin(), path.path.end(), walk.begin());


	if (!recursionFunction(walk, 0))
	{
		for (uint64 i = 0; i < minSpanTree.size(); ++i)
		{
			uint64 n;
			if (!visited[minSpanTree[i].first])
				n = minSpanTree[i].first;
			else if (!visited[minSpanTree[i].second])
				n = minSpanTree[i].second;
			else continue;
			visited[n] = true;
			path.path.push_back(n);
			path.fullLength += matrix[path.path[path.path.size() - 1] * width + n];
		}
	}
	path.fullLength += matrix[path.path[path.path.size() - 1]];
	path.count = width;
	return path;
}