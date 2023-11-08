#pragma once
#include "Objects/FiniteAutomaton.h"
#include "Objects/Grammar.h"
#include "Objects/Regex.h"
#include <deque>
#include <map>
#include <optional>
#include <set>
#include <variant>

// Типизация входныx данных
namespace Typization {

// Перечисление типов объектов
enum class ObjectType {
	NFA,		 // недетерминированный КА
	DFA,		 // детерминированный КА
	Regex,		 // регулярное выражение
	RandomRegex, // место для подстановки сгенерированных регулярных выражений
	Int,		 // целое число
	String,		 // имя файла для чтения
	Boolean,	 // true/false
	OptionalBool,	// optional<bool>
	AmbiguityValue, // yes/no/ы/ь
	PrefixGrammar,	// префиксная грамматика
	Array			// массив
};

// Структуры объектов для хранения в интерпретаторе
template <ObjectType T, class V> struct ObjectHolder {
	V value;
	ObjectType type() const {
		return T;
	};

	explicit ObjectHolder(){};
	explicit ObjectHolder(V value) : value(value){};
};

// Сами структуры
struct ObjectNFA;
struct ObjectDFA;
struct ObjectRegex;
struct ObjectInt;
struct ObjectString;
struct ObjectBoolean;
struct ObjectOptionalBool;
struct ObjectAmbiguityValue;
struct ObjectPrefixGrammar;
struct ObjectArray;

// Универсальный объект
using GeneralObject =
	variant<ObjectNFA, ObjectDFA, ObjectRegex, ObjectInt, ObjectString, ObjectBoolean,
			ObjectOptionalBool, ObjectAmbiguityValue, ObjectPrefixGrammar, ObjectArray>;

#define OBJECT_DEFINITION(type, value)                                                             \
	struct Object##type : public ObjectHolder<ObjectType::type, value> {                           \
		using ObjectHolder::ObjectHolder;                                                          \
	};

// Определение структур объектов
OBJECT_DEFINITION(NFA, FiniteAutomaton)
OBJECT_DEFINITION(DFA, FiniteAutomaton)
OBJECT_DEFINITION(Regex, Regex)
OBJECT_DEFINITION(Int, int)
OBJECT_DEFINITION(String, string)
OBJECT_DEFINITION(Boolean, bool)
OBJECT_DEFINITION(OptionalBool, optional<bool>)
OBJECT_DEFINITION(AmbiguityValue, FiniteAutomaton::AmbiguityValue)
OBJECT_DEFINITION(PrefixGrammar, Grammar)
OBJECT_DEFINITION(Array, vector<GeneralObject>)
}; // namespace Typization

/*
Считаем дедлайном 6 часов утра 24 октября. Успехов!
*/

/*
Заика в бар заходит, подходит к стойке
— Ддай ммнее крружжкку ппива.
Бармен:
— Ппожаллусттта.
— Бблаггоддаррю.
Подходит другой человек:
— Кружку пива.
Бармен:
— Пожалуйста.
— Благодарю.
Снова подходит заика:
— Тты чче, нна ддо ммной пприккалывваешшся.
— Дда ннет, этто я ннад нним пприкколлолся
*/