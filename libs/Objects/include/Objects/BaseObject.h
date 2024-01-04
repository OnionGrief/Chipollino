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
	explicit BaseObject(std::shared_ptr<Language>);
	explicit BaseObject(std::set<Symbol>);
	virtual std::string to_txt() const = 0;

	std::shared_ptr<Language> get_language() const;
};