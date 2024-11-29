#include "TSPMatrix.h"
#include <string>
#include <chrono>
#include <iomanip>
#include <Windows.h>

// Flags and command line argument state.
bool mFlag = false;
bool sFlag = false;
bool qFlag = false;
const char* input = 0;

struct {
	double matrixInitializationTime;
	double solveTime;
} stats = { 0 };

void printUsage()
{
	std::cout << "usage: tsp [-b] [-n] [-m] [-c[n,h,b]] [-t] [-s] <input>" << std::endl << std::endl;
	std::cout << "<input> may be a filepath or a number in which case a random problem with input used as the width is generated" << std::endl << std::endl;
	std::cout << "Flags: " << std::endl;
	std::cout << "    -s Generate and print statistics." << std::endl;
	std::cout << "    -q Path length and Node Count only. Do not print full path." << std::endl;
	std::cout << "    -m Print the matrix, not recommended for large datasets" << std::endl;
	std::cout << "    -b Use the brute force algorithm." << std::endl;
	std::cout << "    -n Use the nearest neighbour algorithm" << std::endl;
	std::cout << "    -t Use the minimum spanning tree algorithm" << std::endl;
	std::cout << "    -c[n,h,b] Use the Christofide's algorithm. Matching is done with nearest neighbour (-cn), brute force (-cb) or Hungarian Minimum Matching(-ch)" << std::endl;
}

auto startTimer()
{
	return std::chrono::high_resolution_clock::now();
}

// C++ is great so long as you never have to use any one else's mental
//    template nonsense :) I just need a double for seconds :))
template<typename T> double stopTimer(T start)
{
	return ((std::chrono::duration<double>)(std::chrono::high_resolution_clock::now() - start)).count();
}

// TODO: refactor, little jumbled
void printTimeInApproprateUnits(double seconds)
{
	if (seconds < 1.0)
	{
		seconds *= 1000.0;
		if (seconds < 1.0)
		{
			seconds *= 1000.0;
			if (seconds < 1.0)
			{
				seconds *= 1000.0;
				std::cout << std::fixed << std::setprecision(0) << seconds << " Nano Seconds" << std::endl;
			}
			else
			{
				std::cout << std::fixed << std::setprecision(2) << seconds << " Micro Seconds" << std::endl;
			}
		}
		else
		{
			std::cout << std::fixed << std::setprecision(2) << seconds << " Milli Seconds" << std::endl;
		}
	}
	else if (seconds > 60)
	{
		std::cout << std::fixed << (uint32)floor(seconds / 60.0) << " Minutes and ";
		std::cout << std::fixed << std::setprecision(2) << seconds - (floor(seconds / 60.0) * 60.0) << " Seconds" << std::endl;
	}
	else
	{
		std::cout << std::fixed << std::setprecision(2) << seconds << " Seconds" << std::endl;
	}
}


void printSizeInApproprateUnits(uint64 bytes)
{
	if (bytes < 1000)
		std::cout << bytes << " Bytes" << std::endl;
	else
	{
		double bigger =  (double)bytes / 1000.0;
		if (bigger < 1000)
			std::cout << std::fixed << std::setprecision(2) << bigger << " Kilo-Bytes" << std::endl;
		else
		{
			bigger /= 1000.0;
			if (bigger < 1000)
				std::cout << std::fixed << std::setprecision(2) << bigger << " Mega-Bytes" << std::endl;
			else
			{
				bigger /= 1000.0;
				if (bigger < 1000)
					std::cout << std::fixed << std::setprecision(2) << bigger << " Giga-Bytes" << std::endl;
			}
		}
		
	}
}

int main(int argc, const char ** argv)
{

#ifdef _DEBUG
	argc = 4;
	const char* args[] = { "", "-ch", "-s", "burma14.tsp"};
	argv = args;
#endif
	try {
		TSPSolveFunction solveFunction = 0;
		std::srand(std::time(nullptr));
		
		for (uint8 i = 1; i < argc; ++i)
		{
			if (argv[i][0] == '-')
			{
				switch (argv[i][1])
				{
				case 't':
					solveFunction = solveTSPMinSpanTree;
					break;
				case 's':
					sFlag = true;
					break;
				case 'c':
					solveFunction = solveTSPChristofides;
					if (argv[i][2] == 'n')
						break; // Default
					else if (argv[i][2] == 'b')
						solveAlgorithmVariant = 1;
					else if (argv[i][2] == 'h')
						solveAlgorithmVariant = 2;
					else
						std::cout << std::string("Warning: No matching algorithm was chosen for Christofides! "
						"Defaulting to nearest neighbour. Argument number: ").append(std::to_string(i)) << std::endl;
					break;
				case 'b':
					solveFunction = solveTSPBruteForce;
					break;
				case 'n':
					solveFunction = solveTSPNearestNeighbour;
					break;
				case 'm':
					mFlag = true;
					break;
				case 'q':
					qFlag = true;
					break;
				default:
					throw std::runtime_error(std::string("Unrecognized flag: ").append(argv[i]));
				}
			}
			else
			{
				if (input != 0)
					throw std::runtime_error(std::string("Only one input allowed"));
				input = argv[i];
			}
		}

		if (argc < 2)
		{
			std::cout << "No input was provided!" << std::endl << std::endl;
			printUsage();
			exit(0);
		}

		if (solveFunction == 0)
		{
			std::cout << "No solving algorithm was selected!" << std::endl << std::endl;
			printUsage();
			exit(0);
		}

		int64 genWidth = 0;
		std::vector<double> generatedCoords;
		for (uint64 i = 0; input[i] != '\0'; ++i)
		{
			if (input[i] < 48 || input[i]>57)
			{
				genWidth = -1;
				break;
			}
			genWidth = genWidth * 10 + (input[i] - 48);
		}

		TSPMatrix mat;
		auto start = startTimer();
		if (genWidth > 0)
		{
			for (int64 i = 0; i < genWidth; ++i)
			{
				generatedCoords.push_back(((double)(std::rand() % 25117) / 3085553.1801) * (2690201.227 + ((double)(std::rand() % 31) * 13873.11)));
				generatedCoords.push_back(((double)(std::rand() % 25117) / 3085553.1801) * (2690201.227 + ((double)(std::rand() % 31) * 13873.719)));
			}
		}

		if (genWidth > 0)
		{
			mat = TSPMatrix2D(generatedCoords, euclideanDistance2D);
		}
		else
		{
			mat = createTSPMatrix(input);
		}
		stats.matrixInitializationTime = stopTimer(start);
		start = startTimer();
		TSPPath path = mat.solve(solveFunction);
		stats.solveTime = stopTimer(start);
		
		if (mFlag)
		{
			std::cout << std::endl << "---------------   Matrix   ---------------" << std::endl;
			mat.print();
			std::cout << std::endl;
		}

		std::cout << std::endl << "---------------   Output   ---------------" << std::endl;
		path.print(!qFlag);

		if (sFlag)
		{
			std::cout << std::endl << "--------------- Statistics ---------------" << std::endl;
			std::cout << " Matrix Size           : "; printSizeInApproprateUnits(sizeof(double)* mat.getWidth()* mat.getWidth());
			std::cout << " Matrix Initialization : "; printTimeInApproprateUnits(stats.matrixInitializationTime);
			std::cout << " Solving Algorithm     : "; printTimeInApproprateUnits(stats.solveTime);

		}
	}
	catch (std::exception e)
	{
		std::cerr << " " << e.what() << std::endl;
	}
	return 0;
}