#include "TSPMatrix.h"
#include <string>

// https://oeclass.aua.gr/eclass/modules/document/file.php/310/3.%20%CE%91%CF%81%CF%87%CE%B5%CE%AF%CE%B1%20VRP%20-%20format.pdf
// http://comopt.ifi.uni-heidelberg.de/software/TSPLIB95/TSPFAQ.html#:~:text=Q%3A%20What%20does%20the%20function,int%20value%20larger%20in%20magnitude.

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


int main()
{
	try {
		auto start = clock();
		//TSPMatrix2D mat("../../data/travelling salesman/burma14.tsp");
		TSPMatrix2D mat(coords3, euclideanDistance2D);
		TSPPath path = mat.solve();
		auto clocks = clock() - start;

		mat.print();
		std::cout << std::endl;
		path.print();
		std::cout << std::endl << "Time Taken: ";
		std::cout << ((double)clocks / (double)CLOCKS_PER_SEC) << " seconds" << std::endl << std::endl;
	}
	catch (std::exception e)
	{
		// Literally just wanting to add a newline at the moment
		//    may do something else later although I see little need..
		std::cerr << e.what() << std::endl;
	}
	return 0;
}