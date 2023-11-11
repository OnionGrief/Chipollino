#pragma once
#include <string>
using std::string;

class AutomatonToImage {
  public:
	AutomatonToImage();
	~AutomatonToImage();
	static string to_image(string automat);
};