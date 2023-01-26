#include "AutomatonToImage/AutomatonToImage.h"
#include <iostream>
using namespace std;

AutomatonToImage::AutomatonToImage() {}

AutomatonToImage::~AutomatonToImage() {}

void AutomatonToImage::to_image(string automat, int name) {
	char cmd[1024];

	// для Linux:

	/*sprintf(cmd,
			"dot -Tpng ./resources/input.dot > "
			"./resources/output%d.png && rm ./resources/input.dot",
			name);*/

	// для Windows:

	sprintf(cmd,
			"dot -Tpng ./resources/input.dot > "
			"./resources/output%d.png && del \".\\resources\\input.dot\"",
			name);

	FILE* fo;
	fo = fopen("./resources/input.dot", "wt");
	fprintf(fo, "%s", automat.c_str());
	fclose(fo);
	system(cmd);
}