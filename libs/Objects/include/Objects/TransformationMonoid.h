#pragma once
#include "AlphabetSymbol.h"
#include "BaseObject.h"
#include "Logger.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <math.h>
#include <queue>
#include <sstream>
#include <vector>
using namespace std;

class Language;
class FiniteAutomaton;

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
		vector<alphabet_symbol> name;
		vector<Transition> transitions;
		bool operator==(const Term& a) const {
			return transitions == a.transitions;
		}
	};
	// двойной терм (нужен для uwu переходов)
	struct TermDouble {
		Term first;
		Term second;
	};
	TransformationMonoid();
	TransformationMonoid(const FiniteAutomaton& in);
	// получаем все эквивалентные классы
	vector<Term> get_equalence_classes();
	// получаем термы, что vw - в языке
	vector<Term> get_equalence_classes_vw(const Term& w);
	// получаем термы, что wv - в языке
	vector<Term> get_equalence_classes_wv(const Term& w);
	// получаем термы, что vwv - в языке
	vector<TermDouble> get_equalence_classes_vwv(const Term& w);
	// получаем правила переписывания
	map<vector<alphabet_symbol>, vector<vector<alphabet_symbol>>>
	get_rewriting_rules();
	// вывод эквивалентных классов
	string get_equalence_classes_txt();
	map<string, vector<string>> get_equalence_classes_map();
	// вывод правил переписывания
	string get_rewriting_rules_txt();
	// Вывод всей информации о Моноиде
	string to_txt();
	// Вернет	-1	если	не	синхронизирован	или
	// номер состояния	с	которым синхронизирован
	int is_synchronized(const Term& w);
	// Вернет число классов эквивалентности
	int class_card();
	// Вернет длину самого длинного слова в классе
	int class_length();
	// Вычисление Минимальности по М-Н (true если минимальный)
	bool is_minimal();
	// Вычисление размера по М-Н
	int get_classes_number_MyhillNerode();
	// вывод таблицы М-Н
	string to_txt_MyhillNerode();
	// переписываем имя терма в  минимальное
	vector<alphabet_symbol> rewriting(
		const vector<alphabet_symbol>&,
		const map<vector<alphabet_symbol>, vector<vector<alphabet_symbol>>>&);
	// возвращает таблицу М-Н
	vector<vector<bool>> get_equivalence_classes_table(
		vector<string>& table_rows, vector<string>& table_columns);

  private:
	// Автомат
	FiniteAutomaton automat;
	// Эквивалентные классы
	vector<Term> terms;
	// Правила переписывания
	map<vector<alphabet_symbol>, vector<vector<alphabet_symbol>>> rules;
	// Taблица М-Н
	vector<vector<bool>> equivalence_classes_table_bool;
	// Левая часть таблицы
	vector<string> equivalence_classes_table_left;
	// шапка таблицы
	vector<string> equivalence_classes_table_top;

	//   | t o p
	// l |--------
	// e | 0 1 0 0
	// f | 0 bool0
	// t | 1 0 1 1

	// очередь на проверку термов (в ней лежат непроверенные кандидаты)
	queue<Term> queueTerm;
	// флаг (неминимальны ли ловушки)
	bool trap_not_minimal = false;
	// проверяем имя терма на  переписываемость (вспомогательный)
	static bool was_rewrite(const vector<alphabet_symbol>&,
							const vector<alphabet_symbol>&);
	// проверка на присутствие терма
	static bool was_transition(const set<TransformationMonoid::Transition>&,
							   const TransformationMonoid::Transition&);
	// проверяем имя терма на переписываемость
	bool searchrewrite(const vector<alphabet_symbol>&);
	// генерируем новые переходы по алфавиту
	void get_new_transition(const vector<TransformationMonoid::Transition>&,
							const vector<alphabet_symbol>&,
							const set<alphabet_symbol>&);
};
