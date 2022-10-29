#include "AutomatonToImage.h"
#include <iostream>
using namespace std;

AutomatonToImage::AutomatonToImage() {}

AutomatonToImage::~AutomatonToImage() {}

void AutomatonToImage::to_image(string automat, int name) {
	char cmd[1024];
	sprintf(cmd,
			"dot -Tpng \".\\resources\\input.dot\" > "
			".\\resources\\output%d.png && del \".\\resources\\input.dot\"",
			name);
	FILE* fo;
	fo = fopen("./resources/input.dot", "wt");
	fprintf(fo, automat.c_str());
	fclose(fo);
	system(cmd);
}