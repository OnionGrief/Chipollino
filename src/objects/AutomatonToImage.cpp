#include "AutomatonToImage.h"
#include <iostream>
using namespace std;

AutomatonToImage::AutomatonToImage() {}

AutomatonToImage::~AutomatonToImage() {}

void AutomatonToImage::to_image(string automat, string name){
    char cmd[1024];
    sprintf(cmd, "dot -Tpng \"./../resources/images/input.dot\" > ./../resources/images/output%s.png && del \"./../resources/images/input.dot\"", name.c_str());
    // sprintf(cmd, "dot -Tpng \"./../resources/input.dot\" > ./../resources/output.png && del \"./../resources/input.dot\"");
    FILE * fo;
    fo = fopen("./../resources/images/input.dot", "wt");
    fprintf(fo, automat.c_str());
    fclose(fo);
    system(cmd);
    
}

string AutomatonToImage::to_txt() {}