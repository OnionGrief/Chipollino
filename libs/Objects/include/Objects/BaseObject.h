#pragma once
#include <iostream>
#include <memory>
#include <set>
#include <string>

#include "Symbol.h"

class Language;

class BaseObject {
  protected:
	std::shared_ptr<Language> language;

  public:
	BaseObject();
	BaseObject(std::shared_ptr<Language>); // NOLINT(runtime/explicit)
	BaseObject(std::set<Symbol>);		   // NOLINT(runtime/explicit)
	virtual std::string to_txt() const = 0;

	std::shared_ptr<Language> get_language() const;
};