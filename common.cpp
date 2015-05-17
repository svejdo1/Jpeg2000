#include "common.h"
#include <fstream>

using namespace std;
using namespace BJPEG;

ErrorCode ImageFile::loadFile(const std::string& fileName)
{
	ifstream file(fileName, ios::binary);
	file.seekg(0, ios::end);
	streamsize size = file.tellg();
	if (size < 0)
	{
		return FILE_CANNOT_SEEK;
	}
	file.seekg(0, ios::beg);
	uint8_t* buffer = new uint8_t[size];
	file.read((char*)buffer, size);
	file.close();
	ErrorCode result = load(buffer, 0);
	delete buffer;
	return result;
}

void ImageFile::saveFile(const string& fileName) const
{
	ofstream outfile(fileName, ios::binary);
	save(outfile);
	outfile.close();
}