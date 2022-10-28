#include "BaseObject.h"
#include "Language.h"

BaseObject::BaseObject(){};

BaseObject::BaseObject(shared_ptr<Language> language) : language(language){};

BaseObject::BaseObject(set<alphabet_symbol> alphabet)
	: language(new Language(alphabet)){};