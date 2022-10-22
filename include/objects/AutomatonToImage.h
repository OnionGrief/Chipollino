#pragma once
#include "BaseObject.h"
#include <string>
using namespace std;

class AutomatonToImage: public BaseObject {
public:
	AutomatonToImage();
	~AutomatonToImage();
	void to_image(string automat, string name);
	string to_txt();
};