#include "AutomatonToImage.h"
#include <iostream>
using namespace std;

void AutomatToImage::to_image(string automat){
    string s = "echo";
    string end = "| dot -Tpng > ../../resources/image/output.png";
    s += automat;
    s += end;
    system(s.c_str());
}