#pragma once
#include <string>

class AutomatonToImage {
  public:
	AutomatonToImage();
	~AutomatonToImage();
	static std::string to_image(std::string automat);
	// метод порождения слоёв раскраски для графа
	static std::string colorize(std::string automat, std::string metadata);
};