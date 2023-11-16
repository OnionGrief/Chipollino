#include <unordered_set>

#include "Objects/Language.h"
#include "Objects/Regex.h"
#include "Objects/iLogTemplate.h"

Regex::Regex(const string& str) : Regex() {
	try {
		bool res = from_string(str);
		if (!res) {
			throw std::runtime_error("Regex::from_string() ERROR");
		}
	} catch (const std::runtime_error& re) {
		cout << re.what() << "\n";
		exit(EXIT_FAILURE);
	}
}

Regex::Regex(const string& str, const std::shared_ptr<Language>& new_language) : Regex(str) {
	language = new_language;
}

void Regex::copy(const AlgExpression* other) {
	auto* tmp = cast(other);
	alphabet = tmp->alphabet;
	type = tmp->type;
	value = tmp->value;
	language = tmp->language;
	if (tmp->term_l != nullptr)
		term_l = tmp->term_l->make_copy();
	if (tmp->term_r != nullptr)
		term_r = tmp->term_r->make_copy();
}

Regex* Regex::make_copy() const {
	auto* c = new Regex;
	c->copy(this);
	return c;
}

Regex* Regex::make() const {
	return new Regex;
}

template <typename T> Regex* Regex::cast(T* ptr, bool not_null_ptr) {
	auto* r = dynamic_cast<Regex*>(ptr);
	if (!r && not_null_ptr) {
		throw std::runtime_error("Failed to cast to Regex");
	}

	return r;
}

template <typename T> const Regex* Regex::cast(const T* ptr, bool not_null_ptr) {
	auto* r = dynamic_cast<const Regex*>(ptr);
	if (!r && not_null_ptr) {
		throw std::runtime_error("Failed to cast to Regex");
	}

	return r;
}

template <typename T> vector<Regex*> Regex::cast(vector<T*> ptrs, bool not_null_ptr) {
	vector<Regex*> regexPointers;
	for (T* ptr : ptrs) {
		auto* r = dynamic_cast<Regex*>(ptr);
		if (!r && not_null_ptr) {
			throw std::runtime_error("Failed to cast to Regex");
		}

		regexPointers.push_back(r);
	}

	return regexPointers;
}

Regex* Regex::expr(const vector<AlgExpression::Lexeme>& lexemes, int index_start, int index_end) {
	AlgExpression* p;
	p = scan_symb(lexemes, index_start, index_end);
	if (!p) {
		p = scan_eps(lexemes, index_start, index_end);
	}

	if (!p) {
		p = scan_alt(lexemes, index_start, index_end);
	}
	if (!p) {
		p = scan_conc(lexemes, index_start, index_end);
	}
	if (!p) {
		p = scan_minus(lexemes, index_start, index_end);
	}
	if (!p) {
		p = scan_star(lexemes, index_start, index_end);
	}
	if (!p) {
		p = scan_par(lexemes, index_start, index_end);
	}

	return cast(p, false);
}

Regex* Regex::scan_minus(const vector<AlgExpression::Lexeme>& lexemes, int index_start,
						 int index_end) {
	Regex* p = nullptr;

	if (lexemes[index_start].type != Lexeme::Type::negative) {
		return nullptr;
	}

	Regex* l = expr(lexemes, index_start + 1, index_end);
	if (l == nullptr) {
		delete l;
		return nullptr;
	}
	p = make();
	p->term_l = l;
	p->value = lexemes[index_start];
	p->type = negative;

	p->alphabet = l->alphabet;
	return p;
}

// возвращает пару <вектор сотсояний, max_index>
std::pair<vector<State>, int> Regex::get_thompson(int max_index) const {
	string str;			  // идентификатор состояния
	vector<State> s = {}; // вектор состояний нового автомата
	map<alphabet_symbol, set<int>> m, p, map_l, map_r; // словари автоматов
	set<int> trans;									   // новые транзишены
	int offset; // сдвиг для старых индексов состояний в новом автомате
	std::pair<vector<State>, int> al; // для левого автомата относительно операции
	std::pair<vector<State>, int> ar; // для правого автомата относительно операции
	Language* alp; // Новый язык для автомата
	switch (type) {
	case Type::alt: // |
		al = Regex::cast(term_l)->get_thompson(max_index);
		ar = Regex::cast(term_r)->get_thompson(al.second);
		max_index = ar.second;

		str = "q" + std::to_string(max_index + 1);
		m[alphabet_symbol::epsilon()] = {1, int(al.first.size()) + 1};
		s.push_back(State(0, {}, str, false, m));

		for (size_t i = 0; i < al.first.size(); i++) {
			State test = al.first[i];
			for (const auto& el : test.transitions) {
				alphabet_symbol elem = el.first; // al->alphabet[i];
				trans = {};
				for (int transition_to : test.transitions[elem]) {
					trans.insert(transition_to + 1);
				}
				map_l[elem] = trans;
			}

			if (test.is_terminal) {
				map_l[alphabet_symbol::epsilon()] = {int(al.first.size() + ar.first.size()) + 1};
			}
			s.push_back(State(al.first[i].index + 1, {}, al.first[i].identifier, false, map_l));
			map_l = {};
		}
		offset = s.size();
		for (size_t i = 0; i < ar.first.size(); i++) {
			State test = ar.first[i];
			for (const auto& el : test.transitions) {
				alphabet_symbol elem = el.first;
				trans = {};
				for (int transition_to : test.transitions[elem]) {
					trans.insert(transition_to + offset);
				}
				map_r[elem] = trans;
			}
			if (test.is_terminal) {
				map_r[alphabet_symbol::epsilon()] = {offset + int(ar.first.size())};
			}

			s.push_back(
				State(ar.first[i].index + offset, {}, ar.first[i].identifier, false, map_r));
			map_r = {};
		}

		str = "q" + std::to_string(max_index + 2);
		s.push_back(State(int(al.first.size() + ar.first.size()) + 1, {}, str, true, p));

		return {s, max_index + 2};
	case Type::conc: // .
		al = Regex::cast(term_l)->get_thompson(max_index);
		ar = Regex::cast(term_r)->get_thompson(al.second);
		max_index = ar.second;

		for (size_t i = 0; i < al.first.size(); i++) {
			State test = al.first[i];
			for (const auto& el : test.transitions) {
				alphabet_symbol elem = el.first; // al->alphabet[i];
				trans = {};
				for (int transition_to : test.transitions[elem]) {
					trans.insert(transition_to);
				}
				map_l[elem] = trans;
			}

			if (test.is_terminal) {
				State test_r = ar.first[0];
				for (const auto& el : test_r.transitions) {
					alphabet_symbol elem = el.first; // al->alphabet[i];
					for (int transition_to : test_r.transitions[elem]) {
						// trans.push_back(test.transitions[elem][j] + 1);
						map_l[elem].insert(transition_to + al.first.size() - 1);
					}
					// map_l[elem] = trans;
				}
			}
			s.push_back(State(al.first[i].index, {}, al.first[i].identifier, false, map_l));
			map_l = {};
		}
		offset = s.size();
		for (size_t i = 1; i < ar.first.size(); i++) {
			State test = ar.first[i];
			for (const auto& el : test.transitions) {
				alphabet_symbol elem = el.first; // al->alphabet[i];
				trans = {};
				// alfa.push_back(elem);
				for (int transition_to : test.transitions[elem]) {
					trans.insert(transition_to + offset - 1);
				}
				map_r[elem] = trans;
			}

			s.push_back(State(ar.first[i].index + offset - 1,
							  {},
							  ar.first[i].identifier,
							  test.is_terminal,
							  map_r));
			map_r = {};
		}

		return {s, max_index};
	case Type::star: // *
		al = Regex::cast(term_l)->get_thompson(max_index);
		max_index = al.second;

		str = "q" + std::to_string(max_index + 1);
		m[alphabet_symbol::epsilon()] = {1, int(al.first.size()) + 1};
		s.push_back(State(0, {}, str, false, m));

		for (size_t i = 0; i < al.first.size(); i++) {
			State test;
			test = al.first[i];
			for (const auto& el : test.transitions) {
				alphabet_symbol elem = el.first; // al->alphabet[i];
				trans = {};
				for (int transition_to : test.transitions[elem]) {
					trans.insert(transition_to + 1);
				}
				map_l[elem] = trans;
			}

			if (test.is_terminal) {
				map_l[alphabet_symbol::epsilon()] = {1, int(al.first.size()) + 1};
			}
			s.push_back(State(al.first[i].index + 1, {}, al.first[i].identifier, false, map_l));
			map_l = {};
		}
		offset = s.size();

		str = "q" + std::to_string(max_index + 2);
		s.push_back(State(int(al.first.size()) + 1, {}, str, true, p));

		return {s, max_index + 2};
	case Type::eps:
		str = "q" + std::to_string(max_index + 1);

		m[alphabet_symbol::epsilon()] = {1};
		s.push_back(State(0, {}, str, false, m));
		str = "q" + std::to_string(max_index + 2);
		s.push_back(State(1, {}, str, true, p));

		return {s, max_index + 2};
	default:

		str = "q" + std::to_string(max_index + 1);
		// m[char_to_alphabet_symbol(value.symbol)] = {1};
		m[value.symbol] = {1};
		s.push_back(State(0, {}, str, false, m));
		str = "q" + std::to_string(max_index + 2);
		s.push_back(State(1, {}, str, true, p));

		return {s, max_index + 2};
	}
	return {};
}

FiniteAutomaton Regex::to_thompson(iLogTemplate* log) const {
	FiniteAutomaton fa(0, get_thompson(-1).first, language);
	if (log) {
		log->set_parameter("oldregex", *this);
		log->set_parameter("result", fa);
	}
	return fa;
}

Regex Regex::linearize(iLogTemplate* log) const {
	Regex temp_copy(*this);
	vector<Regex*> list = Regex::cast(temp_copy.pre_order_travers());
	set<alphabet_symbol> lang_l;
	for (size_t i = 0; i < list.size(); i++) {
		list[i]->value.symbol.linearize(i);
		lang_l.insert(list[i]->value.symbol);
	}
	temp_copy.set_language(lang_l);
	if (log) {
		log->set_parameter("oldregex", *this);
		log->set_parameter("linearised regex", temp_copy);
	}
	return temp_copy;
}

Regex Regex::delinearize(iLogTemplate* log) const {
	Regex temp_copy(*this);
	vector<Regex*> list = Regex::cast(temp_copy.pre_order_travers());
	set<alphabet_symbol> lang_del;
	for (auto& i : list) {
		i->value.symbol.delinearize();
		lang_del.insert(i->value.symbol);
	}
	temp_copy.set_language(lang_del);
	if (log) {
		log->set_parameter("oldregex", *this);
		log->set_parameter("result", temp_copy);
	}
	return temp_copy;
}

FiniteAutomaton Regex::to_glushkov(iLogTemplate* log) const {
	Regex temp_copy(*this);
	vector<Regex*> symbols = Regex::cast(temp_copy.pre_order_travers());
	for (size_t i = 0; i < symbols.size(); i++) {
		symbols[i]->value.number = i;
		symbols[i]->value.symbol.linearize(i);
	}

	vector<Regex*> first = cast(temp_copy.get_first_nodes()); // Множество начальных состояний
	vector<Regex*> last = cast(temp_copy.get_last_nodes()); // Множество конечных состояний
	unordered_map<int, vector<int>> following_states =
		temp_copy.pairs(); // множество состояний, которым предшествует key-int
	int eps_in = this->contains_eps();
	vector<State> st; // Список состояний в автомате

	string str_first;
	string str_end;
	string str_pair;
	for (auto& i : first) {
		str_first += string(i->value.symbol) + "\\ ";
	}

	set<string> end_set;
	for (auto& i : last) {
		end_set.insert(string(i->value.symbol));
	}
	for (const auto& elem : end_set) {
		str_end = str_end + elem + "\\ ";
	}
	if (eps_in) {
		str_end = "eps" + str_end;
	}

	for (auto& i : following_states) {
		for (auto& to : i.second) {
			str_pair = str_pair + "(" + string(symbols[i.first]->value.symbol) + "," +
					   string(symbols[to]->value.symbol) + ")" + "\\ ";
		}
	}

	// cout << temp_copy.to_str_log() << endl;
	// cout << "First " << str_first << endl;
	// cout << "End " << str_end << endl;
	// cout << "Pairs " << str_pair << endl;

	vector<string> linearized_symbols;
	for (auto& i : symbols) {
		linearized_symbols.push_back(i->value.symbol);
		i->value.symbol.delinearize();
	}

	map<alphabet_symbol, set<int>> start_state_transitions;
	for (auto& i : first) {
		start_state_transitions[i->value.symbol].insert(i->value.number + 1);
	}

	if (eps_in) {
		st.push_back(State(0, {}, "S", true, start_state_transitions));
	} else {
		st.push_back(State(0, {}, "S", false, start_state_transitions));
	}

	std::unordered_set<int> last_lexemes;
	for (auto& i : last) {
		last_lexemes.insert(i->value.number);
	}

	for (size_t i = 0; i < symbols.size(); i++) {
		Lexeme lexeme = symbols[i]->value;
		map<alphabet_symbol, set<int>> transitions;

		for (int j : following_states[lexeme.number]) {
			transitions[symbols[j]->value.symbol].insert(j + 1);
		}
		string id_str = linearized_symbols[i];

		// В last_lexemes номера конечных лексем => last_lexemes.count проверяет есть ли
		// номер лексемы в списке конечных лексем (является ли состояние конечным)
		st.push_back(State(i + 1, {}, id_str, last_lexemes.count(lexeme.number), transitions));
	}

	FiniteAutomaton fa(0, st, language);
	if (log) {
		log->set_parameter("oldregex", temp_copy);
		log->set_parameter("linearised regex", temp_copy.linearize());
		log->set_parameter("first", str_first);
		log->set_parameter("last", str_end);
		log->set_parameter("pairs", str_pair);
		log->set_parameter("result", fa);
	}
	return fa;
}

FiniteAutomaton Regex::to_ilieyu(iLogTemplate* log) const {
	FiniteAutomaton glushkov = this->to_glushkov();
	vector<State> states = glushkov.states;
	vector<int> follow;
	for (size_t i = 0; i < states.size(); i++) {
		State st1 = states[i];
		map<alphabet_symbol, set<int>> map1 = st1.transitions;
		for (size_t j = i + 1; j < states.size(); j++) {
			State st2 = states[j];
			map<alphabet_symbol, set<int>> map2 = st2.transitions;
			bool flag = true;
			if (i == j || map2.size() != map1.size() || st1.is_terminal != st2.is_terminal) {
				continue;
			}

			for (auto& it1 : map1) {
				set<int> v1 = it1.second;
				set<int> v2 = map2[it1.first];
				if (v1 != v2 /*equal(v1.begin(), v1.end(), v2.begin())*/) {
					flag = false;
					break;
				}
			}
			if (flag) {
				follow.push_back(j);
				states[i].label.insert(j);
			}
		}
	}

	vector<State> new_states;
	for (auto& state : states) {
		if (find(follow.begin(), follow.end(), state.index) == follow.end()) {
			new_states.push_back(state);
		}
	}

	string str_follow;
	for (auto& new_state : new_states) {
		int state_ind = new_state.index;
		str_follow = str_follow + states[state_ind].identifier + ":\\ ";
		for (auto j = states[state_ind].label.begin(); j != states[state_ind].label.end(); j++) {
			str_follow = str_follow + states[*j].identifier + "\\ ";
		}
		str_follow = str_follow + ";\\\\";
	}

	// cout << str_follow;

	for (size_t i = 0; i < new_states.size(); i++) {
		State v1 = new_states[i];
		map<alphabet_symbol, set<int>> old_map = v1.transitions;
		map<alphabet_symbol, set<int>> new_map;
		for (auto& it1 : old_map) {
			set<int> v1 = it1.second;
			for (int transition_to : v1) {
				for (size_t k = 0; k < new_states.size(); k++) {
					if (new_states[k].label.find(transition_to) != new_states[k].label.end() ||
						transition_to == new_states[k].index) {
						new_map[it1.first].insert(k);
					}
				}
			}
		}
		new_states[i].transitions = new_map;
	}

	for (size_t i = 0; i < new_states.size(); i++) {
		new_states[i].index = i;
	}
	FiniteAutomaton fa(0, new_states, glushkov.language);
	if (log) {
		log->set_parameter("oldregex", *this);
		log->set_parameter("glushkov", glushkov);
		log->set_parameter("follow", str_follow);
		log->set_parameter("result", fa);
	}
	return fa;
}

void Regex::get_prefix(int len, set<string>& prefs) const {
	set<string> prefs1, prefs2;
	if (len == 0) {
		prefs.insert("");
		return;
	}
	switch (type) {
	case Type::eps:
		if (len == 0)
			prefs.insert("");
		return;
	case Type::symb:
		if (len == 1) {
			string res;
			res += value.symbol;
			prefs.insert(res);
		}
		return;
	case Type::alt:
		Regex::cast(term_l)->get_prefix(len, prefs1);
		Regex::cast(term_r)->get_prefix(len, prefs2);
		for (auto i = prefs1.begin(); i != prefs1.end(); i++) {
			prefs.insert(*i);
		}
		for (auto i = prefs2.begin(); i != prefs2.end(); i++) {
			prefs.insert(*i);
		}
		return;
	case Type::conc:
		for (int k = 0; k <= len; k++) {
			Regex::cast(term_l)->get_prefix(k, prefs1);
			Regex::cast(term_r)->get_prefix(len - k, prefs2);
			for (auto i = prefs1.begin(); i != prefs1.end(); i++) {
				for (auto j = prefs2.begin(); j != prefs2.end(); j++) {
					prefs.insert(*i + *j);
				}
			}
			prefs1.clear();
			prefs2.clear();
		}
		return;
	case Type::star:
		if (len == 0) {
			prefs.insert("");
			return;
		}
		for (int k = 1; k <= len; k++) {
			Regex::cast(term_l)->get_prefix(k, prefs1);
			get_prefix(len - k, prefs2);
			for (auto i = prefs1.begin(); i != prefs1.end(); i++) {
				for (auto j = prefs2.begin(); j != prefs2.end(); j++) {
					prefs.insert(*i + *j);
				}
			}
			prefs1.clear();
			prefs2.clear();
		}
		return;
	}
}

bool Regex::derivative_with_respect_to_sym(Regex* respected_sym, const Regex* reg_e,
										   Regex& result) const {
	if (respected_sym->type != Type::eps && respected_sym->type != Type::symb) {
		cout << "Invalid input: unexpected regex instead of symbol\n";
		return false;
	}
	if (respected_sym->type == Type::eps) {
		result = *reg_e;
		return true;
	}
	Regex subresult, subresult1;
	bool answer = true, answer1, answer2;
	switch (reg_e->type) {
	case Type::eps:
		if (respected_sym->type != Type::eps)
			return false;
		result.type = Type::eps;
		return answer;
	case Type::symb:
		if (respected_sym->value.symbol != reg_e->value.symbol) {
			return false;
		}
		result.type = Type::eps;
		return answer;
	case Type::alt:
		answer1 =
			derivative_with_respect_to_sym(respected_sym, Regex::cast(reg_e->term_l), subresult);
		answer2 =
			derivative_with_respect_to_sym(respected_sym, Regex::cast(reg_e->term_r), subresult1);
		// cout << "alt of " << reg_e->term_l->to_txt() << " and "
		//	 << reg_e->term_r->to_txt() << "\n";
		// cout << answer1 << " " << answer2 << "\n";
		if (answer1 && answer2) {
			result.type = Type::alt;
			result.term_l = subresult.make_copy();
			result.term_r = subresult1.make_copy();
			// cout << result.to_txt() << "\n";
			return true;
		}
		if (!answer1 && !answer2) {
			// cout << result.to_txt() << "\n";
			return false;
		}
		if (answer1) {
			result = subresult;
			// cout << result.to_txt() << "\n";
			return true;
		}
		if (answer2 && !answer1) {
			result = subresult1;
			// cout << result.to_txt() << "\n";
			return true;
		}
		return answer;
	case Type::conc:
		subresult.type = Type::conc;
		if (subresult.term_l == nullptr)
			subresult.term_l = new Regex();
		answer1 = derivative_with_respect_to_sym(
			respected_sym, Regex::cast(reg_e->term_l), *Regex::cast(subresult.term_l));
		subresult.term_r = reg_e->term_r->make_copy();
		if (Regex::cast(reg_e->term_l)->contains_eps()) {
			answer2 = derivative_with_respect_to_sym(
				respected_sym, Regex::cast(reg_e->term_r), subresult1);
			if (answer1 && answer2) {
				result.type = Type::alt;
				result.term_l = subresult.make_copy();
				result.term_r = subresult1.make_copy();
			}
			if (answer1 && !answer2) {
				result.type = subresult.type;
				if (subresult.term_l != nullptr)
					result.term_l = subresult.term_l->make_copy();
				if (subresult.term_r != nullptr)
					result.term_r = subresult.term_r->make_copy();
			}
			if (answer2 && !answer1) {
				result.type = subresult1.type;
				if (subresult1.term_l != nullptr)
					result.term_l = subresult1.term_l->make_copy();
				if (subresult1.term_r != nullptr)
					result.term_r = subresult1.term_r->make_copy();
			}
			// cout << "conc of " << reg_e->term_l->to_txt() << " and "
			//	 << reg_e->term_r->to_txt() << "\n";
			// cout << answer1 << " " << answer2 << " " << result.to_txt()
			//	 << "\n";
			answer = answer1 | answer2;
		} else {
			// cout << "conc of " << reg_e->term_l->to_txt() << " and "
			//	 << reg_e->term_r->to_txt() << "\n";
			// cout << answer1 << "\n";
			answer = answer1;
			result.type = subresult.type;
			if (subresult.term_l != nullptr)
				result.term_l = subresult.term_l->make_copy();
			if (subresult.term_r != nullptr)
				result.term_r = subresult.term_r->make_copy();
		}
		return answer;
	case Type::star:
		result.type = Type::conc;
		if (result.term_l == nullptr)
			result.term_l = new Regex();
		bool answer = derivative_with_respect_to_sym(
			respected_sym, Regex::cast(reg_e->term_l), *Regex::cast(result.term_l));
		result.term_r = reg_e->make_copy();
		return answer;
	}
}

bool Regex::partial_derivative_with_respect_to_sym(Regex* respected_sym, const Regex* reg_e,
												   vector<Regex>& result) const {
	Regex cur_result;
	if (respected_sym->type != Type::eps && respected_sym->type != Type::symb) {
		cout << "Invalid input: unexpected regex instead of symbol\n";
		return false;
	}
	if (respected_sym->type == Type::eps) {
		cur_result.type = reg_e->type;
		if (reg_e->term_l != nullptr)
			cur_result.term_l = reg_e->term_l->make_copy();
		if (reg_e->term_l)
			cur_result.term_r = reg_e->term_l->make_copy();
		result.push_back(cur_result);
		return true;
	}
	Regex cur_subresult, cur_subresult1;
	vector<Regex> subresult, subresult1;
	bool answer = true, answer1, answer2;
	switch (reg_e->type) {
	case Type::eps:
		return false;
	case Type::symb:
		if (respected_sym->value.symbol != reg_e->value.symbol) {
			return false;
		}
		cur_result.type = Type::eps;
		result.push_back(cur_result);
		return answer;
	case Type::alt:
		answer1 = partial_derivative_with_respect_to_sym(
			respected_sym, Regex::cast(reg_e->term_l), subresult);
		answer2 = partial_derivative_with_respect_to_sym(
			respected_sym, Regex::cast(reg_e->term_r), subresult1);
		for (const auto& i : subresult) {
			result.push_back(i);
		}
		for (const auto& i : subresult1) {
			result.push_back(i);
		}
		answer = answer1 | answer2;
		return answer;
	case Type::conc:
		cur_subresult.type = Type::conc;
		answer1 = partial_derivative_with_respect_to_sym(
			respected_sym, Regex::cast(reg_e->term_l), subresult);
		cur_subresult.term_r = reg_e->term_r->make_copy();
		for (auto& i : subresult) {
			cur_subresult.term_l = i.make_copy();
			result.push_back(cur_subresult);
			delete cur_subresult.term_l;
			cur_subresult.term_l = nullptr;
		}
		if (Regex::cast(reg_e->term_l)->contains_eps()) {
			answer2 = partial_derivative_with_respect_to_sym(
				respected_sym, Regex::cast(reg_e->term_r), subresult1);
			for (const auto& i : subresult1) {
				result.push_back(i);
			}
			answer = answer1 | answer2;
		} else {
			answer = answer1;
		}
		return answer;
	case Type::star:
		cur_result.type = Type::conc;
		bool answer = partial_derivative_with_respect_to_sym(
			respected_sym, Regex::cast(reg_e->term_l), subresult);
		cur_result.term_r = reg_e->make_copy();
		for (auto& i : subresult) {
			cur_result.term_l = i.make_copy();
			result.push_back(cur_result);
			delete cur_result.term_l;
			cur_result.term_l = nullptr;
		}
		return answer;
	}
}

bool Regex::derivative_with_respect_to_str(string str, const Regex* reg_e, Regex& result) const {
	bool success = true;
	Regex cur = *reg_e;
	Regex next = *reg_e;
	// cout << "start getting derevative for prefix " << str << " in "
	//	 << reg_e->to_txt() << "\n";
	for (char i : str) {
		Regex sym;
		sym.type = Type::symb;
		sym.value.symbol = i;
		next.clear();
		success &= derivative_with_respect_to_sym(&sym, &cur, next);
		// cout << "derevative for prefix " << sym->to_txt() << " in "
		//	 << cur.to_txt() << " is " << next.to_txt() << "\n";
		if (!success) {
			return false;
		}
		cur = next;
	}
	result = next;
	// cout << " answer is " << result.to_txt();
	return success;
}

// Производная по символу
std::optional<Regex> Regex::symbol_derivative(const Regex& respected_sym) const {
	auto rs = respected_sym.make_copy();
	Regex result;
	std::optional<Regex> ans;
	if (derivative_with_respect_to_sym(Regex::cast(rs), this, result))
		ans = result;
	else
		ans = std::nullopt;
	delete rs;
	return ans;
}
// Частичная производная по символу
void Regex::partial_symbol_derivative(const Regex& respected_sym, vector<Regex>& result) const {
	auto rs = respected_sym.make_copy();
	partial_derivative_with_respect_to_sym(Regex::cast(rs), this, result);
	delete rs;
}
// Производная по префиксу
std::optional<Regex> Regex::prefix_derivative(string respected_str) const {
	Regex result;
	std::optional<Regex> ans;
	if (derivative_with_respect_to_str(respected_str, this, result))
		ans = result;
	else
		ans = std::nullopt;
	return ans;
}
// Длина накачки
int Regex::pump_length(iLogTemplate* log) const {
	if (language->is_pump_length_cached()) {
		if (log) {
			log->set_parameter("pumplength", language->get_pump_length());
			log->set_parameter("cach", "(!) результат получен из кэша");
		}
		return language->get_pump_length();
	}
	map<string, bool> checked_prefixes;
	for (int i = 1;; i++) {
		set<string> prefs;
		get_prefix(i, prefs);
		if (prefs.empty()) {
			language->set_pump_length(i);
			if (log) {
				log->set_parameter("pumplength", i);
			}
			return i;
		}
		for (auto it = prefs.begin(); it != prefs.end(); it++) {
			bool was = false;
			for (int j = 0; j < it->size(); j++) {
				if (checked_prefixes[it->substr(0, j)]) {
					was = true;
					break;
				}
			}
			if (was)
				continue;
			for (int j = 0; j < it->size(); j++) {
				for (int k = j + 1; k <= it->size(); k++) {
					Regex pumping;
					string pumped_prefix;
					pumped_prefix += it->substr(0, j);
					pumped_prefix += "(" + it->substr(j, k - j) + ")*";
					pumped_prefix += it->substr(k, it->size() - k + j);
					Regex a(pumped_prefix);
					Regex b;
					pumping.regex_union(&a, &b);
					if (!derivative_with_respect_to_str(*it, this, *Regex::cast(pumping.term_r)))
						continue;
					pumping.generate_alphabet(pumping.alphabet);
					pumping.language = std::make_shared<Language>(pumping.alphabet);
					// cout << pumped_prefix << " " << pumping.term_r->to_txt();
					if (subset(pumping)) {
						checked_prefixes[*it] = true;
						language->set_pump_length(i);
						/*cout << *it << "\n";
						cout << pumping.to_txt() << "\n";
						cout << to_txt() << "\n";
						cout << subset(pumping) << "\n";
						Regex pump2;
						cout << subset(pump2);*/
						if (log) {
							log->set_parameter("pumplength", i);
						}
						return i;
					}
				}
			}
		}
	}
}

bool Regex::equal(const Regex& r1, const Regex& r2, iLogTemplate* log) {
	bool result = equality_checker(&r1, &r2);
	if (log) {
		log->set_parameter("regex1", r1);
		log->set_parameter("regex2", r2);
		log->set_parameter("result", result);
	}
	return result;
}

bool Regex::equivalent(const Regex& r1, const Regex& r2, iLogTemplate* log) {
	bool result = true;
	if (r1.language == r2.language) {
		if (log)
			log->set_parameter("samelanguage",
							   "(!) регулярные выражения изначально принадлежат одному языку");
	} else {
		FiniteAutomaton fa1 = r1.to_ilieyu();
		FiniteAutomaton fa2 = r2.to_ilieyu();
		result = FiniteAutomaton::equivalent(fa1, fa2);
	}
	if (log) {
		log->set_parameter("regex1", r1);
		log->set_parameter("regex2", r2);
		log->set_parameter("result", result);
	}
	return result;
}

bool Regex::subset(const Regex& r, iLogTemplate* log) const {
	bool result = to_ilieyu().subset(r.to_ilieyu());
	if (log) {
		log->set_parameter("regex1", *this);
		log->set_parameter("regex2", r);
		log->set_parameter("result", result);
	}
	return result;
}

FiniteAutomaton Regex::to_antimirov(iLogTemplate* log) const {
	map<set<string>, set<string>> trans;
	vector<Regex> states;
	vector<Regex> alph_regex;
	vector<vector<Regex>> out;
	set<string> check;
	for (const alphabet_symbol& as : language->get_alphabet()) {
		string symbol = as;
		Regex r(symbol);
		alph_regex.push_back(r);
	}

	states.push_back(*this);
	check.insert(to_txt());
	for (size_t i = 0; i < states.size(); i++) {
		Regex state = states[i];
		for (auto j = alph_regex.begin(); j != alph_regex.end(); j++) {
			vector<Regex> regs;
			state.partial_symbol_derivative(*j, regs);
			for (auto k = regs.begin(); k != regs.end(); k++) {
				out.push_back({state, *k, *j});
				if (check.find(k->to_txt()) == check.end()) {
					states.push_back(*k);
					check.insert(k->to_txt());
				}
			}
		}
	}

	vector<string> name_states;

	for (auto& state : states) {
		name_states.push_back(state.to_txt());
	}

	vector<State> automat_state;

	string deriv_log;

	for (size_t i = 0; i < name_states.size(); i++) {
		string state = name_states[i];
		map<alphabet_symbol, set<int>> transit;
		for (size_t j = 0; j < out.size(); j++) {
			// cout << out[j][0].to_txt() << " ";
			// cout << out[j][1].to_txt() << " ";
			// cout << out[j][2].to_txt() << endl;
			deriv_log += out[j][2].to_txt() + "(" + out[j][0].to_txt() + ")" + "\\ =\\ ";
			if (out[j][1].to_txt() == "") {
				deriv_log += "eps\\\\";
			} else {
				deriv_log += out[j][1].to_txt() + "\\\\";
			}
			if (out[j][0].to_txt() == state) {
				auto n = find(name_states.begin(), name_states.end(), out[j][1].to_txt());
				alphabet_symbol s = out[j][2].to_txt();
				transit[s].insert(n - name_states.begin());
			}
		}

		if (state.empty() || states[i].contains_eps()) {
			if (state.empty()) {
				state = alphabet_symbol::epsilon();
			}
			automat_state.push_back({int(i), {}, state, true, transit});
		} else {
			automat_state.push_back({int(i), {}, state, false, transit});
		}
	}
	string str_state;
	for (auto& i : automat_state) {
		str_state += i.identifier + "\\\\ ";
	}

	FiniteAutomaton fa(0, automat_state, language);
	if (log) {
		log->set_parameter("oldregex", *this);
		log->set_parameter("derivative", deriv_log);
		log->set_parameter("state", str_state);
		log->set_parameter("result", fa);
	}
	return fa;
}

Regex Regex::deannote(iLogTemplate* log) const {
	Regex temp_copy(*this);
	vector<Regex*> list = Regex::cast(temp_copy.pre_order_travers());
	set<alphabet_symbol> lang_deann;
	for (auto& i : list) {
		i->value.symbol.deannote();
		lang_deann.insert(i->value.symbol);
	}
	temp_copy.set_language(lang_deann);
	if (log) {
		log->set_parameter("oldregex", *this);
		log->set_parameter("result", temp_copy);
	}
	return temp_copy;
}

bool Regex::is_one_unambiguous(iLogTemplate* log) const {
	if (log) {
		log->set_parameter("oldregex", *this);
	}
	FiniteAutomaton fa = to_glushkov();
	bool res = fa.is_deterministic();
	if (log) {
		log->set_parameter("result", res ? "True" : "False");
	}
	return fa.is_deterministic();
}

Regex Regex::get_one_unambiguous_regex(iLogTemplate* log) const {
	if (log) {
		log->set_parameter("oldregex", *this);
	}
	FiniteAutomaton fa = to_glushkov();
	if (fa.language->is_one_unambiguous_regex_cached()) {
		if (log) {
			log->set_parameter("result", fa.language->get_one_unambiguous_regex());
			log->set_parameter("cache", "(!) результат OneUnambiguous получен из кэша");
		}
		return fa.language->get_one_unambiguous_regex();
	}
	if (!fa.language->is_one_unambiguous_flag_cached())
		fa.is_one_unambiguous();
	if (!fa.language->get_one_unambiguous_flag()) {
		if (log) {
			log->set_parameter("result", "Язык не является 1-однозначным");
		}
		return *this;
	}
	string regl;
	FiniteAutomaton min_fa;
	if (!fa.language->is_min_dfa_cached() && log) {
		log->set_parameter("cachedMINDFA", "Минимальный автомат сохранен в кэше");
	}
	if (fa.size() == 1)
		min_fa = fa.minimize();
	else
		min_fa = fa.minimize().remove_trap_states();

	set<map<alphabet_symbol, set<int>>> final_states_transitions;
	for (int i = 0; i < min_fa.size(); i++) {
		if (min_fa.states[i].is_terminal) {
			final_states_transitions.insert(min_fa.states[i].transitions);
		}
	}

	set<alphabet_symbol> min_fa_consistent;
	// calculate a set of min_fa_consistent symbols
	for (const alphabet_symbol& symb : min_fa.language->get_alphabet()) {
		set<int> reachable_by_symb;
		bool is_symb_min_fa_consistent = true;
		for (int i = 0; i < min_fa.size(); i++) {
			for (const auto& transition : min_fa.states[i].transitions) {
				if (transition.first == symb) {
					for (int elem : transition.second) {
						reachable_by_symb.insert(elem);
					}
				}
			}
		}
		for (int elem : reachable_by_symb) {
			is_symb_min_fa_consistent = true;
			for (const auto& final_state_transitions : final_states_transitions) {
				// Из автомата удаляется ловушка,
				// поэтому не по всем буквам есть переходы
				if (final_state_transitions.find(symb) == final_state_transitions.end())
					continue;
				if (find(final_state_transitions.at(symb).begin(),
						 final_state_transitions.at(symb).end(),
						 elem) == final_state_transitions.at(symb).end()) {
					is_symb_min_fa_consistent = false;
					break;
				}
			}
			if (is_symb_min_fa_consistent)
				min_fa_consistent.insert(symb);
		}
	}

	FiniteAutomaton min_fa_cut =
		FiniteAutomaton(min_fa.initial_state, min_fa.states, min_fa.language);

	for (int i = 0; i < min_fa.size(); i++) {
		if (min_fa.states[i].is_terminal) {
			map<alphabet_symbol, set<int>> new_transitions;
			for (const auto& transition : min_fa.states[i].transitions) {
				if (min_fa_consistent.find(transition.first) == min_fa_consistent.end()) {
					new_transitions[transition.first] = transition.second;
				}
			}
			min_fa_cut.states[i].transitions = new_transitions;
		}
	}

	min_fa_cut = min_fa_cut.remove_unreachable_states();
	regl = min_fa_cut.to_regex().to_txt();

	int counter = 0;
	for (const alphabet_symbol& consistent_symb : min_fa_consistent) {
		bool alternate_flag = false;
		// TODO
		// сборка регулярок из строк будет ошибочной, если символы размечены
		if (!counter) {
			regl += "(" + (string)consistent_symb;
		} else {
			regl += "|" + (string)consistent_symb;
			alternate_flag = true;
		}
		set<int> reachable_by_consistent_symb;
		for (int i = 0; i < min_fa.size(); i++) {
			for (int consistent_symb_transition : min_fa.states[i].transitions[consistent_symb]) {
				reachable_by_consistent_symb.insert(consistent_symb_transition);
			}
		}
		for (int elem : reachable_by_consistent_symb) {
			FiniteAutomaton consistent_symb_automaton(0, {}, std::make_shared<Language>());
			set<int> reachable_states = min_fa.closure({elem}, false);
			vector<int> inserted_states_indices;
			int consistent_symb_automaton_initial_state = 0;
			for (int reachable_state : reachable_states) {
				if (reachable_state == elem)
					consistent_symb_automaton.initial_state =
						consistent_symb_automaton_initial_state;
				consistent_symb_automaton.states.push_back(min_fa.states[reachable_state]);
				inserted_states_indices.push_back(reachable_state);
				consistent_symb_automaton_initial_state++;
			}
			set<alphabet_symbol> consistent_symb_automaton_alphabet;
			for (int j = 0; j < consistent_symb_automaton.size(); j++) {
				consistent_symb_automaton.states[j].index = j;
				map<alphabet_symbol, set<int>> consistent_symb_automaton_state_transitions;
				for (const auto& symb_transition :
					 consistent_symb_automaton.states[j].transitions) {
					for (int transition : symb_transition.second) {
						for (int k = 0; k < inserted_states_indices.size(); k++) {
							if (inserted_states_indices[k] == transition) {
								consistent_symb_automaton_state_transitions[symb_transition.first]
									.insert(k);
								consistent_symb_automaton_alphabet.insert(symb_transition.first);
							}
						}
					}
				}
				if (!consistent_symb_automaton_state_transitions.empty()) {
					consistent_symb_automaton.states[j].transitions =
						consistent_symb_automaton_state_transitions;
				}
			}
			consistent_symb_automaton.language =
				std::make_shared<Language>(consistent_symb_automaton_alphabet);
			FiniteAutomaton consistent_symb_automaton_cut =
				FiniteAutomaton(consistent_symb_automaton.initial_state,
								consistent_symb_automaton.states,
								consistent_symb_automaton.language);
			for (int j = 0; j < consistent_symb_automaton.size(); j++) {
				if (consistent_symb_automaton.states[j].is_terminal) {
					map<alphabet_symbol, set<int>> new_transitions;
					for (const auto& transition : consistent_symb_automaton.states[j].transitions) {
						if (min_fa_consistent.find(transition.first) == min_fa_consistent.end()) {
							new_transitions[transition.first] = transition.second;
						}
					}
					consistent_symb_automaton_cut.states[j].transitions = new_transitions;
				}
			}
			consistent_symb_automaton_cut =
				consistent_symb_automaton_cut.remove_unreachable_states();
			string consistent_symb_automaton_cut_to_regex =
				consistent_symb_automaton_cut.to_regex().to_txt();
			if (!consistent_symb_automaton_cut_to_regex.empty()) {
				if (alternate_flag)
					regl += "(";
				regl += consistent_symb_automaton_cut_to_regex;
				if (alternate_flag)
					regl += ")";
			}
		}
		counter++;
	}
	if (counter)
		regl += ")*";
	language->set_one_unambiguous_regex(regl, fa.language);
	Regex res = language->get_one_unambiguous_regex();
	if (log) {
		log->set_parameter("result", res);
	}
	return res;
}

Regex Regex::normalize_regex(const vector<std::pair<Regex, Regex>>& rules,
							 iLogTemplate* log) const {
	Regex regex = *this;
	if (log) {
		log->set_parameter("oldregex", *this);
	}
	return regex;
}