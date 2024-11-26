#include "TSPMatrix.h"
#include <string>
#include <chrono>
#include <iomanip>
#include <Windows.h>

// https://oeclass.aua.gr/eclass/modules/document/file.php/310/3.%20%CE%91%CF%81%CF%87%CE%B5%CE%AF%CE%B1%20VRP%20-%20format.pdf
// http://comopt.ifi.uni-heidelberg.de/software/TSPLIB95/TSPFAQ.html#:~:text=Q%3A%20What%20does%20the%20function,int%20value%20larger%20in%20magnitude.

// Flags and command line argument state.
bool mFlag = false;
bool sFlag = false;
const char* input = 0;

struct {
	double matrixInitializationTime;
	double solveTime;
} stats = { 0 };

void printUsage()
{
	std::cout << "usage: tsp [-b] [-n] [-m] [-c] [-t] [-s] <input>" << std::endl << std::endl;
	std::cout << "<input> may be a filepath or a number in which case a random problem with input used as the width is generated" << std::endl << std::endl;
	std::cout << "Flags: " << std::endl;
	std::cout << "    -s Generate and print statistics." << std::endl;
	std::cout << "    -m Print the matrix, not recommended for large datasets" << std::endl;
	std::cout << "    -b Use the brute force algorithm." << std::endl;
	std::cout << "    -n Use the nearest neighbour algorithm" << std::endl;
	std::cout << "    -t Use the minimum spanning tree algorithm" << std::endl;
	std::cout << "    -c Use the Christofide's algorithm" << std::endl;
}

std::vector<double> coords1 =
{
	1,2,
	3,2,
	3,5,
	1,5
};

std::vector<double> coords2 =
{
	1,2,
	3,2,
	3,5,
	1,5,
	12,14,
	20,20,
	-1.5,-120,
	0,8,
	100,220,
	45.73, 12.9
};

std::vector<double> coords3 =
{
	1,2,
	3,2,
	3,5,
	1,5,
	12,14,
	20,20,
	-1.5,-120,
	0,8,
	100,220,
	45.73, 12.9,
	18,99.99
};

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
				std::cout << std::fixed << std::setprecision(0) << seconds << " nano seconds" << std::endl;
			}
			else
			{
				std::cout << std::fixed << std::setprecision(2) << seconds << " micro seconds" << std::endl;
			}
		}
		else
		{
			std::cout << std::fixed << std::setprecision(2) << seconds << " milli seconds" << std::endl;
		}
	}
	else if (seconds > 60)
	{
		std::cout << std::fixed << (uint32)floor(seconds / 60.0) << " minutes and ";
		std::cout << std::fixed << std::setprecision(2) << seconds - (floor(seconds / 60.0) * 60.0) << " seconds" << std::endl;
	}
	else
	{
		std::cout << std::fixed << std::setprecision(2) << seconds << " seconds" << std::endl;
	}
}


int main(int argc, const char ** argv)
{

#ifdef _DEBUG
	argc = 4;
	const char* args[] = { "", "-n", "-s", "../../data/travelling salesman/usa13509.tsp"};
	//argc = 5;
	//const char* args[] = { "", "-s", "-b", "-m", "11" };
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
		//mat = &createTSPMatrix("../../data/travelling salesman/att48.tsp");
		//mat = &TSPMatrix2D(coords1, euclideanDistance2D);
		start = startTimer();
		TSPPath path = mat.solve(solveFunction);
		stats.solveTime = stopTimer(start);
		
		if (mFlag)
		{
			mat.print();
			std::cout << std::endl;
		}

		path.print();

		if (sFlag)
		{
			std::cout << std::endl << "Statistics:" << std::endl;
			std::cout << "----    Time    ----" << std::endl;
			std::cout << "Matrix Initialization : "; printTimeInApproprateUnits(stats.matrixInitializationTime);
			std::cout << "Solving Algorithm     : "; printTimeInApproprateUnits(stats.solveTime);

		}
	}
	catch (std::exception e)
	{
		// Literally just wanting to add a newline at the moment
		//    may do something else later although I see little need..
		std::cerr << e.what() << std::endl;
	}
	return 0;
}