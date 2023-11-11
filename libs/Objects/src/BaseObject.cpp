#include "Objects/BaseObject.h"
#include "Objects/Language.h"

BaseObject::BaseObject() {}

BaseObject::BaseObject(std::shared_ptr<Language> language) : language(language) {}

BaseObject::BaseObject(set<alphabet_symbol> alphabet)
	: language(std::make_shared<Language>(alphabet)) {}