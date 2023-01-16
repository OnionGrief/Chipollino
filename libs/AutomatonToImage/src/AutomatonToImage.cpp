#include "AutomatonToImage/AutomatonToImage.h"
#include <iostream>
using namespace std;

AutomatonToImage::AutomatonToImage() {}

AutomatonToImage::~AutomatonToImage() {}

void AutomatonToImage::to_image(string automat, int name) {
	char cmd[1024];
	sprintf(cmd,
			"dot -Tpng ./resources/input.dot > "
			"./resources/output%d.png && rm ./resources/input.dot",
			name);
	FILE* fo;
	fo = fopen("./resources/input.dot", "wt");
	fprintf(fo, "%s", automat.c_str());
	fclose(fo);
	system(
		"cd refal && refgo Preprocess+MathMode input.dot > error_refal0.txt");
	system("cd refal && dot2tex -ftikz -tmath \"Mod_input.dot\" > input.tex");
	system("cd refal && refgo Postprocess+MathMode input.tex > error_refal.txt "
		   "2>&1");

	system("cd refal && rm input.dot && rm input.tex");
	
	//автомат
	ifstream infile_for_R("./refal/R_input.tex");
	stringstream graph;
	string s;
	if (infile_for_R) {
		for (; !infile_for_R.eof();) {
			getline(infile_for_R, s);
			graph << s << endl;
		}
	}
	infile_for_R.close();


	//таблица
	ifstream infile_for_L("./refal/L_input.tex");
	if (infile_for_L) {
		for (; !infile_for_L.eof();) {
			getline(infile_for_L, s);
			graph << s << endl;
		}
	}
	infile_for_L.close();
	

	

	return graph.str();
}