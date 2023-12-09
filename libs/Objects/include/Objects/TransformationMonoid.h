#pragma once
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "BaseObject.h"
#include "FiniteAutomaton.h"
#include "Symbol.h"
#include "iLogTemplate.h"

class Language;
class FiniteAutomaton;

// нужна, чтобы хранить weak_ptr на язык
struct FA_model {
	int initial_state;
	std::vector<FAState> states;
	// если не хранить этот указатель,
	// будут созданы разные shared_ptr
	std::weak_ptr<Language> language;

	FA_model() = default;
	FA_model(int initial_state, std::vector<FAState> states, std::weak_ptr<Language> language);

	FiniteAutomaton make_fa();
};

class TransformationMonoid {
  public:
	// переход (индекс состояния - индекс состояния)
	struct Transition {
		int first;
		int second;
		bool operator==(const Transition& a) const {
			return first == a.first && second == a.second;
		}
		bool operator>(const Transition& a) const {
			return first > a.first && second > a.second;
		}
		bool operator<(const Transition& a) const {
			return first < a.first || first == a.first && second < a.second;
		}
	};
	// Терм (флаг оставляет ли в языке, имя, вектор переходов)
	struct Term {
		bool isFinal = false;
		std::vector<Symbol> name;
		std::vector<Transition> transitions;
		bool operator==(const Term& a) const {
			return transitions == a.transitions;
		}
	};
	// двойной терм (нужен для uwu переходов)
	struct TermDouble {
		Term first;
		Term second;
	};
	TransformationMonoid() = default;
	TransformationMonoid(const FiniteAutomaton& in); // NOLINT(runtime/explicit)
	// получаем все классы эквивалентности
	std::vector<Term> get_equalence_classes();
	// получаем термы, что vw - в языке
	std::vector<Term> get_equalence_classes_vw(const Term& w);
	// получаем термы, что wv - в языке
	std::vector<Term> get_equalence_classes_wv(const Term& w);
	// получаем термы, что vwv - в языке
	std::vector<TermDouble> get_equalence_classes_vwv(const Term& w);
	// получаем правила переписывания
	std::map<std::vector<Symbol>, std::vector<std::vector<Symbol>>> get_rewriting_rules();
	// вывод классов эквивалентных
	std::string get_equalence_classes_txt();
	std::map<std::string, std::vector<std::string>> get_equalence_classes_map();
	// вывод правил переписывания
	std::string get_rewriting_rules_txt(iLogTemplate* log = nullptr);
	// Вывод всей информации о Моноиде
	std::string to_txt();
	// Вернет -1 если не синхронизирован или
	// номер состояния с которым синхронизирован
	int is_synchronized(const Term& w);
	// Вернет число классов эквивалентности
	int class_card(iLogTemplate* log = nullptr);
	// Вернет длину самого длинного слова в классе
	int class_length(iLogTemplate* log = nullptr);
	// Вычисление Минимальности по М-Н (true если минимальный)
	bool is_minimal(iLogTemplate* log = nullptr);
	// Вычисление размера по М-Н
	int get_classes_number_MyhillNerode(iLogTemplate* log = nullptr);
	// вывод таблицы М-Н
	std::string to_txt_MyhillNerode();
	// переписываем имя терма в  минимальное
	std::vector<Symbol> rewriting(
		const std::vector<Symbol>&,
		const std::map<std::vector<Symbol>, std::vector<std::vector<Symbol>>>&);
	// возвращает таблицу М-Н
	std::vector<std::vector<bool>> get_equivalence_classes_table(
		std::vector<std::string>& table_rows,	  // NOLINT(runtime/references)
		std::vector<std::string>& table_columns); // NOLINT(runtime/references)

  private:
	// Автомат
	FA_model automaton;
	// Классы эквивалентности
	std::vector<Term> terms;
	// Правила переписывания
	std::map<std::vector<Symbol>, std::vector<std::vector<Symbol>>> rules;
	// Taблица М-Н
	std::vector<std::vector<bool>> equivalence_classes_table_bool;
	// Левая часть таблицы
	std::vector<std::string> equivalence_classes_table_left;
	// шапка таблицы
	std::vector<std::string> equivalence_classes_table_top;

	//   | t o p
	// l |--------
	// e | 0 1 0 0
	// f | 0 bool0
	// t | 1 0 1 1

	// очередь на проверку термов (в ней лежат непроверенные кандидаты)
	std::queue<Term> queueTerm;
	// флаг (неминимальны ли ловушки)
	bool trap_not_minimal = false;
	// проверяем имя терма на  переписываемость (вспомогательный)
	static bool was_rewrite(const std::vector<Symbol>&, const std::vector<Symbol>&);
	// проверка на присутствие терма
	static bool was_transition(const std::set<TransformationMonoid::Transition>&,
							   const TransformationMonoid::Transition&);
	// проверяем имя терма на переписываемость
	bool searchrewrite(const std::vector<Symbol>&);
	// генерируем новые переходы по алфавиту
	void get_new_transition(const std::vector<TransformationMonoid::Transition>&,
							const std::vector<Symbol>&, const std::set<Symbol>&);
};
