#pragma once
#include "AlphabetSymbol.h"
#include <iostream>
#include <memory>
#include <set>
#include <string>

using std::cout;
using std::set;
using std::string;
using std::vector;

class Language;

class BaseObject {
  protected:
	std::shared_ptr<Language> language;

  public:
	BaseObject();
	BaseObject(std::shared_ptr<Language>); // NOLINT(runtime/explicit)
	BaseObject(set<alphabet_symbol>);	   // NOLINT(runtime/explicit)
	virtual string to_txt() const = 0;
};