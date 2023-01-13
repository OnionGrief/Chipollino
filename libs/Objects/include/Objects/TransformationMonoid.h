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
	struct Transition { // переход (индекс состояния - индекс состояния)
		int first;
		int second;
		bool operator==(const Transition& a) const {
			return this->first == a.first && this->second == a.second;
		}
		bool operator>(const Transition& a) const {
			return this->first > a.first && this->second > a.second;
		}
		bool operator<(const Transition& a) const {
			return this->first * 1000 + this->second <
				   a.first * 1000 + a.second;
		}
	};

	struct Term {
		bool isFinal = false;
		vector<alphabet_symbol> name;
		vector<Transition> transitions;
		bool operator==(const Term& a) const {
			return this->transitions == a.transitions &&
				   this->transitions == a.transitions;
		}
	};

	struct TermDouble { // двойной терм
		Term first;
		Term second;
	};
	TransformationMonoid();
	TransformationMonoid(
		const FiniteAutomaton& in); // Автомат и макс длина перехода
	void OutAllTransformationMonoid();
	vector<Term> get_equalence_classes(); // получаем все термы
	vector<Term> get_equalence_classes_vw(
		const Term& w); // получаем термы, что vw - в языке
	vector<Term> get_equalence_classes_wv(
		const Term& w); // получаем термы, что wv - в языке
	vector<TermDouble> get_equalence_classes_vwv(
		const Term& w); // получаем термы, что vwv - в языке
	map<vector<alphabet_symbol>, vector<vector<alphabet_symbol>>>
	get_rewriting_rules(); // получаем правила переписывания
	string get_equalence_classes_txt(); // вывод эквивалентных классов
	map<string, vector<string>> get_equalence_classes_map();
	string get_rewriting_rules_txt(); // вывод правил переписывания
	string to_txt() const;
	int is_synchronized(
		const Term& w); // Вернет	-1	если	не	синхронизирован	или
	// номер состояния	с	которым синхронизирован
	int class_card(); // Вернет число классов эквивалентности
	int class_length(); // Вернет самое длинное слово в классе
	bool is_minimal(); // Вычисление Минимальности по М-Н(1 если минимальный)
	int get_classes_number_MyhillNerode(); // Вычисление размера по М-Н
	string to_txt_MyhillNerode(); // вывод таблицы М-Н
	vector<alphabet_symbol> rewriting(
		vector<alphabet_symbol>,
		map<vector<alphabet_symbol>, vector<vector<alphabet_symbol>>>);
	vector<vector<bool>> get_equivalence_classes_table(
		vector<string>& table_rows,
		vector<string>& table_columns); // возвращает
										// таблицу М-Н

  private:
	bool searchrewrite(vector<alphabet_symbol>);
	queue<Term> queueTerm;
	void get_transition_by_symbol(vector<TransformationMonoid::Transition>,
								  vector<alphabet_symbol>,
								  const set<alphabet_symbol>&);
	set<int> search_transition_by_word(vector<alphabet_symbol> word,
									   int init_state);
	FiniteAutomaton automat; // Автомат
	vector<Term> terms;		 // Эквивалентные классы
	map<vector<alphabet_symbol>, vector<vector<alphabet_symbol>>>
		rules; // Правила переписывания
	vector<vector<bool>> equivalence_classes_table_bool; // Taблица М-Н
	vector<string> equivalence_classes_table_left; // Левая часть таблицы
	vector<string> equivalence_classes_table_top; // шапка таблицы

	//   | t o p
	// l |--------
	// e | 0 1 0 0
	// f | 0 bool0
	// t | 1 0 1 1
	bool trap_not_minimal = false;
};
