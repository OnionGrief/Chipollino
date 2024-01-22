#include <utility>

#include "Objects/BaseObject.h"
#include "Objects/Language.h"

using std::make_shared;
using std::set;

BaseObject::BaseObject() {}

BaseObject::BaseObject(std::shared_ptr<Language> language) : language(std::move(language)) {}

BaseObject::BaseObject(const Alphabet& alphabet) : language(make_shared<Language>(alphabet)) {}

std::shared_ptr<Language> BaseObject::get_language() const {
	return language;
}
