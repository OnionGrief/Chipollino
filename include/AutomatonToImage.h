#pragma once
#include <string>
using namespace std;

class AutomatonToImage {
public:
	AutomatonToImage();
	~AutomatonToImage();
	static void to_image(string automat, int name);
};