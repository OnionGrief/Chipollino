#include "Objects/BaseObject.h"
#include "Objects/Language.h"

using std::make_shared;
using std::set;

BaseObject::BaseObject() {}

BaseObject::BaseObject(std::shared_ptr<Language> language) : language(language) {}

BaseObject::BaseObject(set<alphabet_symbol> alphabet) : language(make_shared<Language>(alphabet)) {}