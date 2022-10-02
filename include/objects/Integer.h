#pragma once
#include "BaseObject.h"
#include <string>

class Integer: public BaseObject {
private:
	int value = 0;
public:
	std::string to_txt() override;
};