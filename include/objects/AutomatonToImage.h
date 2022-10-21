#pragma once
#include "BaseObject.h"
#include <string>
using namespace std;

class AutomatToImage: public BaseObject {
public:
	void to_image(string automat);
};