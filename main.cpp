#include  <iostream>
#include <fstream>
#include "j2k.h"
#include "j2p.h"
using namespace std;
using namespace BJPEG;

int main(int argc, char* argv[])
{
	ErrorCode errorCode;

	J2KFile jpeg;
	errorCode = jpeg.loadFile("e:\\bretagne1.j2k");
	
	if (errorCode == SUCCESS)
	{
		jpeg.header.Xsiz *= 2;
		jpeg.header.Ysiz *= 2;
		vector<TilePart> tiles = jpeg.tiles;
		TilePart& tile = *tiles.begin();
		jpeg.tiles.begin()->markers.clear();

		for (int i = 0; i < 3; i++)
		{
			TilePart clone = TilePart(tile);
			clone.markers.clear();
			
			jpeg.tiles.push_back(clone);
		}
		jpeg.tiles.begin()->Isot = 2;
		(jpeg.tiles.begin() + 1)->Isot = 3;
		(jpeg.tiles.begin() + 2)->Isot = 0;
		(jpeg.tiles.begin() + 3)->Isot = 1;

		jpeg.saveFile("e:\\clone.j2k");
	}

	return 0;
}