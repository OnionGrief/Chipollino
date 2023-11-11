#pragma once
#include <string>
using std::string;

class AutomatonToImage {
  public:
	AutomatonToImage();
	~AutomatonToImage();
	static string to_image(string automat);
	// метод порождения слоёв раскраски для графа
	static string colorize(string automat, string metadata);
};