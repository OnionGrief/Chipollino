#pragma once
#include "Objects/FiniteAutomaton.h"
#include "Objects/Grammar.h"
#include "Objects/Regex.h"
#include <deque>
#include <map>
#include <optional>
#include <set>
#include <variant>

// Типизация входный данных
namespace Typization {

// Перечисление типов объектов
enum class ObjectType {
	NFA,			// недетерминированный КА
	DFA,			// детерминированный КА
	Regex,			// регулярное выражение
	Int,			// целое число
	FileName,		// имя файла для чтения
	Boolean,		// true/false
	OptionalBool,	// optional<bool>
	AmbiguityValue, // yes/no/ы/ь
	PrefixGrammar	// префиксная грамматика
};

// Структуры объектов для хранения в интерпретаторе
template <ObjectType T, class V> struct ObjectHolder {
	V value;
	ObjectType type() const {
		return T;
	};
	// Кэширование
	optional<bool> minimal; // и так далее
	ObjectHolder(){};
	ObjectHolder(V value) : value(value){};
};

using ObjectNFA = ObjectHolder<ObjectType::NFA, FiniteAutomaton>;
using ObjectDFA = ObjectHolder<ObjectType::DFA, FiniteAutomaton>;
using ObjectRegex = ObjectHolder<ObjectType::Regex, Regex>;
using ObjectInt = ObjectHolder<ObjectType::Int, int>;
using ObjectFileName = ObjectHolder<ObjectType::FileName, string>;
using ObjectBoolean = ObjectHolder<ObjectType::Boolean, bool>;
using ObjectAmbiguityValue =
	ObjectHolder<ObjectType::AmbiguityValue, FiniteAutomaton::AmbiguityValue>;
using ObjectOptionalBool =
	ObjectHolder<ObjectType::OptionalBool, optional<bool>>;
using ObjectPrefixGrammar = ObjectHolder<ObjectType::PrefixGrammar, Grammar>;

// Универсальный объект
using GeneralObject = variant<
	ObjectHolder<ObjectType::NFA, FiniteAutomaton>,
	ObjectHolder<ObjectType::DFA, FiniteAutomaton>,
	ObjectHolder<ObjectType::Regex, Regex>, ObjectHolder<ObjectType::Int, int>,
	ObjectHolder<ObjectType::FileName, string>,
	ObjectHolder<ObjectType::Boolean, bool>,
	ObjectHolder<ObjectType::AmbiguityValue, FiniteAutomaton::AmbiguityValue>,
	ObjectHolder<ObjectType::OptionalBool, optional<bool>>,
	ObjectHolder<ObjectType::PrefixGrammar, Grammar>>;

// Функция, состоит из имени и сигнатуры
// Предикат - тоже функция, но на выходе boolean
struct Function {
	// Имя функции
	string name;
	// Типы входных аргументов
	vector<ObjectType> input;
	// Тип выходного аргумента
	ObjectType output;
	Function(){};
	Function(string name, vector<ObjectType> input, ObjectType output)
		: name(name), input(input), output(output){};
};

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