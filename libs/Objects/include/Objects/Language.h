#pragma once
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "BackRefRegex.h"
#include "FiniteAutomaton.h"
#include "MemoryFiniteAutomaton.h"
#include "PushdownAutomaton.h"
#include "Regex.h"
#include "Symbol.h"
#include "TransformationMonoid.h"

class Language {
  private:
	struct Regex_model {
	  private:
		std::string str;
		std::weak_ptr<Language> language;

	  public:
		Regex_model(std::string str, std::weak_ptr<Language> language);

		const std::string& get_str() const;
		std::shared_ptr<Language> get_language() const;
	};

	inline static bool allow_retrieving_from_cache = true;

	Alphabet alphabet;
	// регулярка, описывающая язык
	// optional<Regex> regular_expression;
	std::optional<int> pump_length;
	std::optional<FA_model> min_dfa;
	std::optional<TransformationMonoid> syntactic_monoid;
	// нижняя граница размера НКА для языка
	std::optional<int> nfa_minimum_size;
	// классы эквивалентности минимального дка TODO
	// аппроксимации минимальных НКА и регулярок TODO
	std::optional<bool> is_one_unambiguous;
	std::optional<Regex_model> one_unambiguous_regex;

  public:
	Language();
	explicit Language(Alphabet alphabet);

	static void enable_retrieving_from_cache();
	static void disable_retrieving_from_cache();

	const Alphabet& get_alphabet();
	void set_alphabet(Alphabet);
	int get_alphabet_size();
	// регулярка, описывающая язык
	bool is_regular_expression_cached() const;
	void set_regular_expression(int);
	int get_regular_expression();
	// накачка
	bool is_pump_length_cached() const;
	void set_pump_length(int);
	int get_pump_length();
	// минимальный дка
	bool is_min_dfa_cached() const;
	void set_min_dfa(const FiniteAutomaton&);
	FiniteAutomaton get_min_dfa();
	// синтаксический моноид
	bool is_syntactic_monoid_cached() const;
	void set_syntactic_monoid(TransformationMonoid);
	TransformationMonoid get_syntactic_monoid();
	// нижняя граница размера НКА для языка
	bool is_nfa_minimum_size_cached() const;
	void set_nfa_minimum_size(int);
	int get_nfa_minimum_size();
	// 1-однозначная регулярка
	bool is_one_unambiguous_flag_cached() const;
	void set_one_unambiguous_flag(bool);
	bool get_one_unambiguous_flag();
	bool is_one_unambiguous_regex_cached() const;
	void set_one_unambiguous_regex(std::string, const std::shared_ptr<Language>&);
	Regex get_one_unambiguous_regex();
	//  и тд
};