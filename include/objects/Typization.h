#pragma once
#include <variant>
#include <set>
#include <map>
#include <deque>
#include "Regex.h"
#include "FiniteAutomat.h"

// Типизация входный данных
namespace Typization {
	// Перечисление типов объектов
	enum class ObjectType {
		NFA,      // недетерминированный КА
		DFA,      // детерминированный КА
		Regex,    // регулярное выражение
		Int,      // целое число
		FileName, // имя файла для чтения
		Boolean   // true/false
	};

	// Структуры объектов для хранения в интерпретаторе
	template <ObjectType T, class V>
	struct ObjectHolder {
		V value;
		ObjectType type() const { return  T };
		// Кэширование
		optional<bool> minimal; // и так далее
	};

	// Универсальный объект
	using GeneralObject = variant<
		ObjectHolder<ObjectType::NFA, FiniteAutomat>,
		ObjectHolder<ObjectType::DFA, FiniteAutomat>,
		ObjectHolder<ObjectType::Regex, Regex>,
		ObjectHolder<ObjectType::Int, int>,
		ObjectHolder<ObjectType::FileName, string>,
		ObjectHolder<ObjectType::Boolean, bool>
	>;

	// Функция, состоит из имени и сигнатуры
	// Предикат - тоже функция, но на выходе boolean
	struct Function {
		// Имя функции
		string name;
		// Типы воходных аргументов
		vector<ObjectType> input;
		// Тип выходного аргумента
		ObjectType output;
	};
};