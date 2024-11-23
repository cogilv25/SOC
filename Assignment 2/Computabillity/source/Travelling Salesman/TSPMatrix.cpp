#include "TSPMatrix.h"
#include <math.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <unordered_map>
#include <algorithm>

// The labels here and below are horribly inconsistant in the TSPLIB
//    spec with the data having different values sometimes so I have
//    done my best to find what they should be but some may be wrong..
const std::vector<std::string> TSPLIB_SPEC =
{
	"NAME",
	"EDGE_WEIGHT_TYPE",
	"COMMENT",
	"DIMENSION",
	"CAPACITY",
	"DISPLAY_DATA_TYPE",
	"NODE_COORD_TYPE",
	"TYPE",
	"EDGE_WEIGHT_FORMAT",
	"EDGEJ3ATA_FORMAT"
};

const std::vector<std::string> TSPLIB_DATA =
{
	"NODE_COORD_SECTION",
	"DEPOT_SECTION",
	"EDGEJ3ATA_SECTION",
	"FIXED_EDGES-SECTION",
	"DISPLAY_DATA_SECTION",
	"TOURJ3ECTION",
	"EDGE_WEIGHT_SECTION"
};

const std::unordered_map<std::string, DistanceFunction2D> TSPLIB_FUN_MAP2D =
{
	{"EUC_2D", euclideanDistance2D},
	{"MAN_2D", manhattenDistance2D},
	{"MAX_2D", maximumDistance2D},
	{"GEO", geographicalDistance},
	{"ATT", pseudoEuclidianDistance2D}
};

const std::unordered_map<std::string, DistanceFunction3D> TSPLIB_FUN_MAP3D =
{
	{"EUC_3D", euclideanDistance3D},
	{"MAN_3D", manhattenDistance3D},
	{"MAX_3D", maximumDistance3D}
};

// Nearest Integer
double nint(double value)
{
	return ((int64)value + 0.5);
}

// Distance functions follow the TSPLIB format specification which I
//    can't link because of Unicode characters but will include in
//    the documentation. We support the majority of functions.
// TODO: Add EXPLICIT, CEIL_2D
double euclideanDistance2D(double x1, double y1, double x2, double y2)
{
	double dx = x1 - x2;
	double dy = y1 - y2;
	return sqrt(dx * dx + dy * dy);
}

double euclideanDistance3D(double x1, double y1, double z1, double x2, double y2, double z2)
{
	double dx = x1 - x2;
	double dy = y1 - y2;
	double dz = z1 - z2;
	return sqrt(dx * dx + dy * dy + dz * dz);
}

double manhattenDistance2D(double x1, double y1, double x2, double y2)
{
	double dx = abs(x1 - x2);
	double dy = abs(y1 - y2);
	return nint(dx + dy);
}

double manhattenDistance3D(double x1, double y1, double z1, double x2, double y2, double z2)
{
	double dx = abs(x1 - x2);
	double dy = abs(y1 - y2);
	double dz = abs(z1 - z2);
	return nint(dx + dy + dz);
}

double maximumDistance2D(double x1, double y1, double x2, double y2)
{
	double dx = nint(abs(x1 - x2));
	double dy = nint(abs(y1 - y2));
	return (dx > dy) ? dx : dy;
}

double maximumDistance3D(double x1, double y1, double z1, double x2, double y2, double z2)
{
	double dx = nint(abs(x1 - x2));
	double dy = nint(abs(y1 - y2));
	double dz = nint(abs(z1 - z2));
	return (dx > dy) ?
		(dx > dz ? dx : dz) :
		(dy > dz ? dy : dz);
}


double geographicalDistance(double latitude1, double longitude1, double latitude2, double longitude2)
{
	// The TSPLIB docs use a different value for pi from c++ which we will have to use
	//    to be able to verify our solutions against theirs.
	static double PI = 3.141592;

	// Convert the latitudes and longitudes from degrees.minutes to radians 
	double coords[4] = { latitude1, longitude1, latitude2, longitude2 };
	double radianCoords[4];
	for (int i = 0; i < 4; ++i)
	{
		double deg = nint(coords[i]);
		double min = coords[i] - deg;
		radianCoords[i] = PI * (deg + 5.0 * min / 3.0) / 180.0;
	}

	// This is the radius of the earth as an idealized sphere in km
	double ERE = 6378.388;

	// Cos of the difference in longitude
	double q1 = cos(radianCoords[1] - radianCoords[3]);
	// Cos of the difference in latitude
	double q2 = cos(radianCoords[0] - radianCoords[2]);
	// Cos of the sum of latitudes
	double q3 = cos(radianCoords[0] + radianCoords[2]);

	//                                            _  (")  _
	// Then somehow convert that to a distance...  \/| |\/
	return ((int)ERE * acos(0.5 * ((1.0 + q1) * q2 - (1.0 - q1) * q3)) + 1.0);
}

double pseudoEuclidianDistance2D(double x1, double y1, double x2, double y2)
{
	double dx = x1 - x2;
	double dy = y1 - y2;
	double distance = sqrt((dx * dx + dy * dy) / 10.0);
	// This appears to just be ceil but in case I am missing something
	//    I will implement it as described...
	double rDist = nint(distance);
	return distance > rDist ? rDist + 1 : rDist;
}


void TSPMatrix::_initializeMatrix(std::vector<double> coords, uint8 step, std::function<void(uint64, double*, double*)> fun)
{
	matrixWidth = coords.size() / step;
	// We may catch the possible exception thrown here somewhere
	//    else or we may just let the program terminate...
	matrix = new double[matrixWidth * matrixWidth];

	for (uint64 i = 0; i < matrixWidth; ++i)
	{
		for (uint64 j = 0; j < matrixWidth; ++j)
		{
			uint64 n = j + i * matrixWidth;
			if (i == j)
				matrix[n] = 0;
			else
				fun(n, &coords[i * step], &coords[j * step]);
		}
	}
}

TSPMatrix::TSPMatrix()
{
	matrix = 0;
	matrixWidth = 0;
}

TSPMatrix::~TSPMatrix()
{
	if(matrix != 0)
		delete[] matrix;
}

TSPPath TSPMatrix::solve()
{
	if (matrix == 0 || matrixWidth == 0)
		throw std::runtime_error("Attempted to solve an uninitialized TSPMatrix");


	std::vector<uint64> indices;
	for (uint64 i = 1; i < matrixWidth; ++i)
		indices.push_back(i);

	TSPPath path; 
	path.count = matrixWidth;
	path.fullLength = DBL_MAX;
	path.path = std::vector<uint64>(matrixWidth);
	std::vector<uint64> currentPath(matrixWidth);
	do
	{
		double pathLength = 0;
		uint64 matX = 0;
		uint64 matY = indices[0];
		for (uint64 i = 0; i < indices.size(); ++i)
		{
			matY = indices[i];
			pathLength += matrix[matY * matrixWidth + matX];
			currentPath[i] = matX;
			matX = indices[i];
		}
		pathLength += matrix[matY * matrixWidth];
		currentPath[matrixWidth - 1] = matY;

		if (pathLength < path.fullLength)
		{
			path.fullLength = pathLength;
			std::copy(currentPath.begin(), currentPath.end(), path.path.begin());
		}
	} while (std::next_permutation(indices.begin(), indices.end()));

	return path;
}

TSPMatrix2D::TSPMatrix2D(std::vector<double> coords, DistanceFunction2D distanceFunction)
{
	_initializeMatrix(coords, 2, [&](uint64 n, double* coord1, double* coord2)
		{
			matrix[n] = distanceFunction(coord1[0], coord1[1], coord2[0], coord2[1]);
		});
}

TSPMatrix2D::TSPMatrix2D(const char* filepath)
{
	std::string line;
	std::ifstream file(filepath, std::ios::in);
	if (!file.is_open())
		throw std::runtime_error(std::string("Could not open file: ").append(filepath));


	// Parse Specification
	uint64 expectedNodeCount = 0;
	DistanceFunction2D distanceFunction = 0;
	bool firstComment = true;
	for (std::getline(file, line); file.good(); std::getline(file,line))
	{
		bool valid = false;
		for (uint8 i = 0; i < TSPLIB_SPEC.size(); ++i)
		{
			size_t pos = line.find(TSPLIB_SPEC[i]);
			if (pos == std::string::npos) continue;
			valid = true;
			pos = line.find(':', pos);
			if (pos > line.length() - 1) throw std::runtime_error(std::string("Invalid TSPLIB specification: ").append(filepath));
			std::string value = line.substr(pos + 1);
			if (i == 0)
			{
				pos = value.find_first_not_of(" \t\r\v\f\n");
				if (pos > line.length() - 1) throw std::runtime_error(std::string("Invalid TSPLIB specification: ").append(filepath));
				std::cout << "TSP Problem Name: " << value.substr(pos) << std::endl;
			}
			else if (i == 7)
			{
				pos = line.find("TSP", pos + 1);
				if (pos > line.length() - 1) throw std::runtime_error(std::string("Problems other than TSP are unsupported: ").append(filepath));
			}
			else if (i == 2)
			{
				pos = value.find_first_not_of(" \t\r\v\f\n");
				if (pos > line.length() - 1) throw std::runtime_error(std::string("Invalid TSPLIB specification: ").append(filepath));
				if (firstComment)
				{
					firstComment = false;
					std::cout << "Comments:" << std::endl;
				}
				std::cout << value.substr(pos) << std::endl;
			}
			else if (i == 3)
			{
				expectedNodeCount = std::stoull(value);
			}
			else if (i == 1)
			{
				pos = value.find_first_not_of(" \t\r\v\f\n");
				if (pos >= line.length() - 1) throw std::runtime_error(std::string("Invalid TSPLIB specification: ").append(filepath));
				auto it = TSPLIB_FUN_MAP2D.find(value.substr(pos));
				if (it == TSPLIB_FUN_MAP2D.end()) throw std::runtime_error(std::string("Unknown TSPLIB specification: ")
					.append(filepath).append("\nUnknown specification key: ").append(value.substr(pos)));
				distanceFunction = it->second;
			}
			else
			{
				std::cout << "Ignoring TSP Specification Key: " << TSPLIB_SPEC[i] << std::endl;
			}
			break;
		}

		if (valid) continue;

		if (distanceFunction == 0)
			throw std::runtime_error(std::string("EDGE_WEIGHT_TYPE not specified in: ").append(filepath));

		// Skip unused data sections until first NODE_COORD_SECTION
		for (; file.good(); std::getline(file, line))
		{
			size_t pos = line.find(TSPLIB_DATA[0]);
			if (pos == std::string::npos) continue;
			break;
		}
		if(!file.good() || file.eof()) throw std::runtime_error(std::string("Node data missing from: ").append(filepath));

		// If we get here we have processed the specification and the next line
		//    is the start of the node data section
		break;
	}

	// Parse the node data section
	bool eofFound = false;
	bool oOOIFlag = false; //Out Of Order Indices Flag
	uint64 nodeCount = 0;
	std::vector<double> coords(expectedNodeCount * 2);

	for (std::getline(file, line); file.good() && !file.eof(); std::getline(file, line))
	{
		if (line.find("EOF") != std::string::npos)
		{
			eofFound = true;
			break;
		}

		// Get index
		size_t pos = line.find_first_not_of(" \t\r\v\f\n");
		if (pos == std::string::npos) continue; // <- Skip blank lines
		uint64 index = std::stoull(line) - 1;
		
		double x, y;
		// Get x (first coord)
		pos = line.find_first_of(" \t\r\v\f\n", pos);
		if (pos != std::string::npos)
			pos = line.find_first_not_of(" \t\r\v\f\n", pos);

		if (pos > line.length() - 1) throw std::runtime_error(std::string("Invalid TSPLIB data, missing coordinate in file: ")
			.append(filepath).append(" with index: ").append(std::to_string(index)));
		x = stod(line.substr(pos));

		// Get y (second coord)
		pos = line.find_first_of(" \t\r\v\f\n", pos);
		if(pos != std::string::npos)
			pos = line.find_first_not_of(" \t\r\v\f\n", pos);

		if (pos > line.length() - 1) throw std::runtime_error(std::string("Invalid TSPLIB data, missing coordinate in file: ")
			.append(filepath).append(" with index: ").append(std::to_string(index)));
		y = stod(line.substr(pos));


		if (!oOOIFlag && index != nodeCount)
			oOOIFlag = true;

		if (index * 2 + 1 > coords.size())
			coords.resize((index + 1) * 2);

		coords[index * 2] = x;
		coords[index * 2 + 1] = y;

		++nodeCount;
	}


	
	if (!(eofFound || file.eof()))
		throw std::runtime_error(std::string("There was an file stream error loading the file: ").append(filepath));

	if (nodeCount < 1)
		throw std::runtime_error(std::string("0 nodes were loaded from the file: ").append(filepath));

	if (coords.size() != nodeCount * 2)
		throw std::runtime_error(std::string("Invalid node numbering in file: ").append(filepath));
	
	if (expectedNodeCount > 0 && nodeCount != expectedNodeCount)
		std::cerr << "Warning: DIMENSION specified does not match actual DIMENSION in: " << filepath << std::endl;

	if (oOOIFlag)
		std::cerr << "Warning: Node indices were out of order: " << filepath << std::endl;

	std::cout << "Successfully loaded: " << filepath << std::endl;


	_initializeMatrix(coords, 2, [&](uint64 n, double* coord1, double* coord2)
		{
			matrix[n] = distanceFunction(coord1[0], coord1[1], coord2[0], coord2[1]);
		});
}

TSPMatrix3D::TSPMatrix3D(std::vector<double> coords, DistanceFunction3D distanceFunction)
{
	_initializeMatrix(coords, 3, [&](uint64 n, double* coord1, double* coord2)
		{
			matrix[n] = distanceFunction(coord1[0], coord1[1], coord1[2], coord2[0], coord2[1], coord2[2]);
		});
}

TSPMatrix3D::TSPMatrix3D(const char* filepath)
{
	// TODO: Implement
	throw std::runtime_error("Initializing a TSPMatrix3D with a file is not currently implemented");
}

void TSPMatrix::print(uint8 precision)
{
	if (matrix == 0 || matrixWidth == 0)
		throw std::runtime_error("Attempted to print an uninitialized TSPMatrix");
		
	std::cout << " Matrix: " << std::endl << ' ';
	for (uint64 y = 0; y < matrixWidth; ++y)
	{
		for (uint64 x = 0; x < matrixWidth; ++x)
		{
			std::cout << std::left << std::setw(10) << std::fixed << std::setprecision(precision)  <<  matrix[x + y * matrixWidth];
		}
		std::cout << std::endl << ' ';
	}
}

void TSPPath::print()
{
	std::cout << " Path Length: " << fullLength << std::endl;
	std::cout << " Node Count: " << count << std::endl;
	std::cout << " Path: " << std::endl << " ( " << path[0];
	for (uint64 i = 1; i < path.size(); ++i)
		std::cout << ", " << path[i];
	std::cout << ')' << std::endl;
}
