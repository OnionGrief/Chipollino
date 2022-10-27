#pragma once
#include "AlphabetSymbol.h"
#include <iostream>
#include <memory>
#include <set>
#include <string>

class Language;

class BaseObject {
  protected:
	shared_ptr<Language> language;

  public:
	BaseObject();
	BaseObject(shared_ptr<Language>);
	BaseObject(set<alphabet_symbol>);
	virtual std::string to_txt() const = 0;
};