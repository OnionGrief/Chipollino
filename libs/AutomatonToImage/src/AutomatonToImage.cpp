#include "AutomatonToImage/AutomatonToImage.h"
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

AutomatonToImage::AutomatonToImage() {}

AutomatonToImage::~AutomatonToImage() {}

/*
void AutomatonToImage::to_image(string automat, int name) {
	char cmd[1024];
	sprintf(cmd,
			"dot -Tpng ./resources/input.dot > "
			"./resources/output%d.png && rm ./resources/input.dot",
			name);
	system(cmd);
*/
string AutomatonToImage::to_image(string automat) {
	FILE* fo;
	fo = fopen("./refal/input.dot", "wt");
	fprintf(fo, "%s", automat.c_str());
	fclose(fo);
	system(
		"cd refal && refgo Preprocess+MathMode input.dot > error_refal0.txt");
	system("cd refal && dot2tex -ftikz -tmath \"Mod_input.dot\" > input.tex");
	system("cd refal && refgo Postprocess+MathMode input.tex > error_refal.txt "
		   "2>&1");
	ifstream infile("./refal/R_input.tex");
	stringstream graph;

	string s;
	for (; !infile.eof();) {
		getline(infile, s);
		graph << s << endl;
	}
	infile.close();

	return graph.str();
}