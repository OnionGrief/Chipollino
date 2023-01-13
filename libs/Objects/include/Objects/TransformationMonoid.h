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

	struct Term { // Терм (флаг оставляет ли в языке, имя, вектор переходов)
		bool isFinal = false;
		vector<alphabet_symbol> name;
		vector<Transition> transitions;
		bool operator==(const Term& a) const {
			return this->transitions == a.transitions &&
				   this->transitions == a.transitions;
		}
	};

	struct TermDouble { // двойной терм (нужен для uwu переходов)
		Term first;
		Term second;
	};
	TransformationMonoid();
	TransformationMonoid(const FiniteAutomaton& in); // Строим моноид
	vector<Term> get_equalence_classes(); // получаем все эквивалентные классы
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
	string to_txt(); // Вывод всей информации о Моноиде
	int is_synchronized(
		const Term& w); // Вернет	-1	если	не	синхронизирован	или
	// номер состояния	с	которым синхронизирован
	int class_card(); // Вернет число классов эквивалентности
	int class_length(); // Вернет длину самого длинного слова в классе
	bool is_minimal(); // Вычисление Минимальности по М-Н(1 если минимальный)
	int get_classes_number_MyhillNerode(); // Вычисление размера по М-Н
	string to_txt_MyhillNerode(); // вывод таблицы М-Н
	vector<alphabet_symbol> rewriting(
		const vector<alphabet_symbol>&,
		const map<vector<alphabet_symbol>,
				  vector<vector<alphabet_symbol>>>&); // переписываем имя терма
													  // в минимальное
	vector<vector<bool>> get_equivalence_classes_table(
		vector<string>& table_rows,
		vector<string>& table_columns); // возвращает
										// таблицу М-Н

  private:
	static bool wasrewrite(
		const vector<alphabet_symbol>&,
		const vector<alphabet_symbol>&); // проверяем имя терма на
										 // переписываемость (вспомогательный)
	static bool wasTransition(
		const set<TransformationMonoid::Transition>&,
		TransformationMonoid::Transition); // проверка на присутствие терма
	bool searchrewrite(const vector<alphabet_symbol>&); // проверяем имя терма
														// на переписываемость
	queue<Term> queueTerm; // очередь на проверку термов (в ней лежат
						   // непроверенные кандидаты)
	void get_new_transition(
		const vector<TransformationMonoid::Transition>&,
		const vector<alphabet_symbol>&,
		const set<alphabet_symbol>&); // генерируем новые переходы по алфавиту
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
	bool trap_not_minimal = false; // флаг (неминимальны ли ловушки)
};
