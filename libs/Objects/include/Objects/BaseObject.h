#pragma once
#include <iostream>
#include <memory>
#include <set>
#include <string>

#include "AlphabetSymbol.h"

class Language;

class BaseObject {
  protected:
	std::shared_ptr<Language> language;

  public:
	BaseObject();
	BaseObject(std::shared_ptr<Language>); // NOLINT(runtime/explicit)
	BaseObject(std::set<alphabet_symbol>); // NOLINT(runtime/explicit)
	virtual std::string to_txt() const = 0;
};