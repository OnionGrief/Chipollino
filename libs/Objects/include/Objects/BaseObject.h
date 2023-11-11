#pragma once
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include "AlphabetSymbol.h"

using std::cout;
using std::vector;
using std::string;
using std::set;

class Language;

class BaseObject {
  protected:
	std::shared_ptr<Language> language;

  public:
	BaseObject();
	BaseObject(std::shared_ptr<Language>);	// NOLINT(runtime/explicit)
	BaseObject(set<alphabet_symbol>);	// NOLINT(runtime/explicit)
	virtual string to_txt() const = 0;
};