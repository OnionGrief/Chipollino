#include <unordered_set>

#include "Objects/Language.h"
#include "Objects/Regex.h"
#include "Objects/iLogTemplate.h"

using std::cerr;
using std::cout;
using std::make_shared;
using std::map;
using std::pair;
using std::set;
using std::string;
using std::to_string;
using std::unordered_map;
using std::vector;

Regex::Regex(const string& str) : Regex() {
	try {
		bool res = from_string(str);
		if (!res) {
			throw std::runtime_error("Regex::from_string() ERROR");
		}
	} catch (const std::runtime_error& re) {
		cerr << re.what() << "\n";
		exit(EXIT_FAILURE);
	}
}

Regex::Regex(const string& str, const std::shared_ptr<Language>& new_language) : Regex(str) {
	language = new_language;
}

Regex::Regex(Type type, AlgExpression* term_l, AlgExpression* term_r)
	: AlgExpression(type, term_l, term_r) {}

void Regex::copy(const AlgExpression* other) {
	auto* tmp = cast(other);
	alphabet = tmp->alphabet;
	type = tmp->type;
	symbol = tmp->symbol;
	language = tmp->language;
	if (tmp->term_l != nullptr)
		term_l = tmp->term_l->make_copy();
	if (tmp->term_r != nullptr)
		term_r = tmp->term_r->make_copy();
}

Regex* Regex::make_copy() const {
	auto c = new Regex;
	c->copy(this);
	return c;
}

Regex* Regex::make() const {
	return new Regex;
}

template <typename T> Regex* Regex::cast(T* ptr, bool not_null_ptr) {
	auto r = dynamic_cast<Regex*>(ptr);
	if (!r && not_null_ptr) {
		throw std::runtime_error("Failed to cast to Regex");
	}

	return r;
}

template <typename T> const Regex* Regex::cast(const T* ptr, bool not_null_ptr) {
	auto r = dynamic_cast<const Regex*>(ptr);
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
		p = scan_star(lexemes, index_start, index_end);
	}
	if (!p) {
		p = scan_minus(lexemes, index_start, index_end);
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
	p->type = negative;

	p->alphabet = l->alphabet;
	return p;
}

vector<FAState> Regex::_to_thompson(const set<Symbol>& root_alphabet) const {
	vector<FAState> fa_states; // вектор состояний нового автомата
	// для формирования поля transitions состояния автомата (структуры State)
	FAState::Transitions state_transitions;
	int offset; // сдвиг для старых индексов состояний в новом автомате
	// список состояний и макс индекс состояния для левого автомата относительно операции
	vector<FAState> fa_left;
	// список состояний и макс индекс состояния для правого автомата относительно операции
	vector<FAState> fa_right;
	// автомат для отрицания, строится обычный томпсон и берется дополнение
	FiniteAutomaton fa_negative;
	vector<FAState> fa_negative_states;

	switch (type) {
	case Type::eps:
		fa_states.emplace_back(0, false);
		fa_states[0].set_transition(1, Symbol::Epsilon);
		fa_states.emplace_back(1, true);
		return fa_states;
	case Type::symb:
		fa_states.emplace_back(0, false);
		fa_states.emplace_back(1, true);
		fa_states[0].set_transition(1, symbol);
		return fa_states;
	case Type::alt: // |
		fa_left = Regex::cast(term_l)->_to_thompson(root_alphabet);
		fa_right = Regex::cast(term_r)->_to_thompson(root_alphabet);

		fa_states.emplace_back(0, false);
		fa_states.back().set_transition(1, Symbol::Epsilon);
		fa_states.back().set_transition(int(fa_left.size()) + 1, Symbol::Epsilon);

		for (const auto& state : fa_left) {
			state_transitions = {};
			for (const auto& [symb, states] : state.transitions) {
				for (int transition_to : states) {
					state_transitions[symb].insert(transition_to + 1);
				}
			}

			if (state.is_terminal) {
				state_transitions[Symbol::Epsilon] = {int(fa_left.size() + fa_right.size()) + 1};
			}

			fa_states.emplace_back(state.index + 1, false, state_transitions);
		}

		offset = fa_states.size();
		for (const auto& state : fa_right) {
			state_transitions = {};
			for (const auto& [symb, states] : state.transitions) {
				for (int transition_to : states) {
					state_transitions[symb].insert(transition_to + offset);
				}
			}

			if (state.is_terminal) {
				state_transitions[Symbol::Epsilon] = {offset + int(fa_right.size())};
			}

			fa_states.emplace_back(state.index + offset, false, state_transitions);
		}

		fa_states.emplace_back(int(fa_left.size() + fa_right.size()) + 1, "q", true);
		return fa_states;
	case Type::conc: // .
		fa_left = Regex::cast(term_l)->_to_thompson(root_alphabet);
		fa_right = Regex::cast(term_r)->_to_thompson(root_alphabet);

		for (const auto& state : fa_left) {
			state_transitions = {};
			for (const auto& [symb, states] : state.transitions) {
				for (int transition_to : states) {
					state_transitions[symb].insert(transition_to);
				}
			}

			if (state.is_terminal) {
				for (const auto& [symb, states] : fa_right[0].transitions) {
					for (int transition_to : states) {
						state_transitions[symb].insert(transition_to + fa_left.size() - 1);
					}
				}
			}

			fa_states.emplace_back(state.index, false, state_transitions);
		}

		offset = fa_states.size();
		for (size_t i = 1; i < fa_right.size(); i++) {
			const FAState& state = fa_right[i];
			state_transitions = {};
			for (const auto& [symb, states] : state.transitions) {
				for (int transition_to : states) {
					state_transitions[symb].insert(transition_to + offset - 1);
				}
			}

			fa_states.emplace_back(state.index + offset - 1, state.is_terminal, state_transitions);
		}

		return fa_states;
	case Type::star: // *
		fa_left = Regex::cast(term_l)->_to_thompson(root_alphabet);

		fa_states.emplace_back(0, false);
		fa_states.back().set_transition(1, Symbol::Epsilon);
		fa_states.back().set_transition(int(fa_left.size()) + 1, Symbol::Epsilon);

		for (const auto& state : fa_left) {
			state_transitions = {};
			for (const auto& [symb, states] : state.transitions) {
				for (int transition_to : states) {
					state_transitions[symb].insert(transition_to + 1);
				}
			}

			if (state.is_terminal) {
				state_transitions[Symbol::Epsilon] = {1, int(fa_left.size()) + 1};
			}

			fa_states.emplace_back(state.index + 1, false, state_transitions);
		}

		fa_states.emplace_back(int(fa_left.size()) + 1, true);
		return fa_states;
	case Type::negative:
		// строим автомат для отрицания
		fa_negative_states = Regex::cast(term_l)->_to_thompson(root_alphabet);

		fa_negative = FiniteAutomaton(0, fa_negative_states, root_alphabet);
		fa_negative = fa_negative.minimize();
		// берем дополнение автомата
		fa_negative = fa_negative.complement();

		// если автомат имеет начальное состояние не 0 то исправляем это
		if (fa_negative.get_initial() != 0)
			fa_negative.set_initial_state_to_zero();

		// нумеруем состояния
		for (auto& state : fa_negative.states) {
			if (state.is_terminal) {
				state.is_terminal = false;
				state.set_transition(fa_negative.size(), Symbol::Epsilon);
			}
		}

		fa_negative.states.emplace_back(int(fa_negative.size()), true);

		// возвращаем состояния и макс индекс
		return fa_negative.states;
	default:
		break;
	}
	return {};
}

FiniteAutomaton Regex::to_thompson(iLogTemplate* log) const {
	vector<FAState> res = _to_thompson(alphabet);
	vector<FAState> states;
	for (const auto& i : res) {
		states.emplace_back(i.index, "q" + to_string(i.index), i.is_terminal, i.transitions);
	}

	FiniteAutomaton fa(0, states, language);
	if (log) {
		log->set_parameter("oldregex", *this);
		log->set_parameter("result", fa);
	}
	return fa;
}

Regex Regex::linearize(iLogTemplate* log) const {
	Regex temp_copy(*this);
	vector<Regex*> list = Regex::cast(temp_copy.preorder_traversal());
	set<Symbol> new_alphabet;
	for (size_t i = 0; i < list.size(); i++) {
		list[i]->symbol.linearize(i);
		new_alphabet.insert(list[i]->symbol);
	}
	temp_copy.set_language(new_alphabet);
	if (log) {
		log->set_parameter("oldregex", *this);
		log->set_parameter("linearised regex", temp_copy);
	}
	return temp_copy;
}

Regex Regex::delinearize(iLogTemplate* log) const {
	Regex temp_copy(*this);
	vector<Regex*> list = cast(temp_copy.preorder_traversal());
	set<Symbol> new_alphabet;
	for (auto& i : list) {
		i->symbol.delinearize();
		new_alphabet.insert(i->symbol);
	}
	temp_copy.set_language(new_alphabet);
	if (log) {
		log->set_parameter("oldregex", *this);
		log->set_parameter("result", temp_copy);
	}
	return temp_copy;
}

vector<Regex*> Regex::preorder_traversal() {
	vector<Regex*> res;
	if (AlgExpression::symb == type) {
		res.push_back(this);
		return res;
	}

	if (term_l) {
		vector<Regex*> l = cast(term_l)->preorder_traversal();
		res.insert(res.end(), l.begin(), l.end());
	}

	if (term_r) {
		vector<Regex*> r = cast(term_r)->preorder_traversal();
		res.insert(res.end(), r.begin(), r.end());
	}

	return res;
}

unordered_map<int, vector<int>> Regex::get_follow() const {
	unordered_map<int, vector<int>> l, r;
	vector<AlgExpression*> first, last;
	switch (type) {
	case Type::alt:
		l = cast(term_l)->get_follow();
		r = cast(term_r)->get_follow();
		for (const auto& i : r) {
			l[i.first].insert(l[i.first].end(), i.second.begin(), i.second.end());
		}

		return l;
	case Type::conc:
		l = cast(term_l)->get_follow();
		r = cast(term_r)->get_follow();
		for (const auto& i : r) {
			l[i.first].insert(l[i.first].end(), i.second.begin(), i.second.end());
		}

		last = cast(term_l)->get_last_nodes();
		first = cast(term_r)->get_first_nodes();
		for (auto& i : last) {
			for (auto& j : first) {
				l[i->get_symbol().last_linearization_number()].push_back(
					j->get_symbol().last_linearization_number());
			}
		}

		return l;
	case Type::star:
		l = cast(term_l)->get_follow();
		last = cast(term_l)->get_last_nodes();
		first = cast(term_l)->get_first_nodes();
		for (auto& i : last) {
			for (auto& j : first) {
				l[i->get_symbol().last_linearization_number()].push_back(
					j->get_symbol().last_linearization_number());
			}
		}

		return l;
	default:
		return {};
	}
}

FiniteAutomaton Regex::to_glushkov(iLogTemplate* log) const {
	Regex temp_copy(*this);
	vector<Regex*> terms = temp_copy.preorder_traversal();
	for (size_t i = 0; i < terms.size(); i++) {
		terms[i]->symbol.linearize(static_cast<int>(i));
	}

	vector<AlgExpression*> first = temp_copy.get_first_nodes(); // Множество начальных состояний
	vector<AlgExpression*> last = temp_copy.get_last_nodes(); // Множество конечных состояний
	// множество состояний, которым предшествует символ (ключ - линеаризованный номер)
	unordered_map<int, vector<int>> following_states = temp_copy.get_follow();
	int eps_in = this->contains_eps();
	vector<FAState> states; // состояния автомата

	string str_first, str_last, str_follow;
	for (auto& i : first) {
		str_first += string(i->get_symbol()) + "\\ ";
	}

	set<string> last_set;
	for (auto& i : last) {
		last_set.insert(string(i->get_symbol()));
	}
	for (const auto& elem : last_set) {
		str_last += elem + "\\ ";
	}
	if (eps_in) {
		str_last += Symbol::Epsilon;
	}

	for (const auto& i : following_states) {
		for (auto& to : i.second) {
			str_follow += "(" + string(terms[i.first]->symbol) + "," + string(terms[to]->symbol) +
						  ")" + "\\ ";
		}
	}

	// cout << temp_copy.to_str_log() << endl;
	// cout << "First " << str_first << endl;
	// cout << "End " << str_last << endl;
	// cout << "Pairs " << str_follow << endl;

	vector<Symbol> delinearized_symbols;
	for (int i = 0; i < terms.size(); i++) {
		delinearized_symbols.push_back(terms[i]->symbol);
		delinearized_symbols[i].delinearize();
	}

	FAState::Transitions start_state_transitions;
	for (auto& i : first) {
		start_state_transitions[delinearized_symbols[i->get_symbol().last_linearization_number()]]
			.insert(i->get_symbol().last_linearization_number() + 1);
	}

	if (eps_in) {
		states.emplace_back(0, "S", true, start_state_transitions);
	} else {
		states.emplace_back(0, "S", false, start_state_transitions);
	}

	std::unordered_set<int> last_terms;
	for (auto& i : last) {
		last_terms.insert(i->get_symbol().last_linearization_number());
	}

	for (size_t i = 0; i < terms.size(); i++) {
		Symbol& symb = terms[i]->symbol;
		FAState::Transitions transitions;

		for (int to : following_states[symb.last_linearization_number()]) {
			transitions[delinearized_symbols[to]].insert(to + 1);
		}

		// В last_terms номера конечных лексем => last_terms.count проверяет есть ли
		// номер лексемы в списке конечных лексем (является ли состояние конечным)
		states.emplace_back(
			i + 1, symb, last_terms.count(symb.last_linearization_number()), transitions);
	}

	FiniteAutomaton fa(0, states, language);
	if (log) {
		log->set_parameter("oldregex", *this);
		log->set_parameter("linearised regex", temp_copy);
		log->set_parameter("first", str_first);
		log->set_parameter("last", str_last);
		log->set_parameter("get_follow", str_follow);
		log->set_parameter("result", fa);
	}
	return fa;
}

FiniteAutomaton Regex::to_ilieyu(iLogTemplate* log) const {
	FiniteAutomaton glushkov = this->to_glushkov();
	vector<FAState> states = glushkov.states;
	vector<int> follow;
	for (size_t i = 0; i < states.size(); i++) {
		FAState st1 = states[i];
		FAState::Transitions map1 = st1.transitions;
		for (size_t j = i + 1; j < states.size(); j++) {
			FAState st2 = states[j];
			FAState::Transitions map2 = st2.transitions;
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

	vector<FAState> new_states;
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
		FAState v1 = new_states[i];
		FAState::Transitions old_map = v1.transitions;
		FAState::Transitions new_map;
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
			res += symbol;
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
		if (respected_sym->symbol != reg_e->symbol) {
			return false;
		}
		result.type = Type::eps;
		return answer;
	case Type::alt:
		answer1 =
			derivative_with_respect_to_sym(respected_sym, Regex::cast(reg_e->term_l), subresult);
		answer2 =
			derivative_with_respect_to_sym(respected_sym, Regex::cast(reg_e->term_r), subresult1);
		if (answer1 && answer2) {
			result.type = Type::alt;
			result.term_l = subresult.make_copy();
			result.term_r = subresult1.make_copy();
			return true;
		}
		if (!answer1 && !answer2) {
			return false;
		}
		if (answer1) {
			result = subresult;
			return true;
		}
		if (answer2 && !answer1) {
			result = subresult1;
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

			answer = answer1 | answer2;
		} else {
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

Regex* Regex::add_alt(std::vector<Regex> res, Regex* root) {
	if (res.size() == 1) {
		return res[0].make_copy();
	}

	root = new Regex;
	root->type = Type::alt;
	root->term_l = res[0].make_copy();
	res.erase(res.begin());
	root->term_r = add_alt(res, Regex::cast(root->term_r, false));
	return root;
}

Regex* Regex::to_aci(std::vector<Regex>& res) {
	// отсортировали вектор регулярок
	std::sort(res.begin(), res.end(), [](const Regex& i, const Regex& j) {
		return i._to_txt(false) < j._to_txt(false);
	});

	// rule w | w = w
	for (size_t i = 0; i < res.size(); i++) {
		for (size_t j = i + 1; j < res.size(); j++) {
			if (i == j)
				continue;

			if (res[i]._to_txt(false) == res[j]._to_txt(false)) {
				res.erase(res.begin() + j);
			}
		}
	}
	// rule w1 | w2 = w2 | w1 не имеют смысла тк наборы отсортированы
	// rule (w1 | w2) | w3 = w1 | (w2 | w3)

	// Regex собранный из вектора через альтерантивы
	// и всё ставится под отрицание
	Regex* res_alt;
	res_alt = new Regex;
	res_alt->type = Type::negative;
	if (!res.size())
		return nullptr;

	res_alt->term_l = add_alt(res, cast(res_alt->term_l, false));
	return res_alt;
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
		if (respected_sym->symbol != reg_e->symbol) {
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
		answer = partial_derivative_with_respect_to_sym(
			respected_sym, Regex::cast(reg_e->term_l), subresult);
		cur_result.term_r = reg_e->make_copy();
		for (auto& i : subresult) {
			cur_result.term_l = i.make_copy();
			result.push_back(cur_result);
			delete cur_result.term_l;
			cur_result.term_l = nullptr;
		}
		return answer;
	case Type::negative:
		cur_result.type = Type::negative;
		answer = partial_derivative_with_respect_to_sym(
			respected_sym, Regex::cast(reg_e->term_l), subresult);

		if (!answer) {
			cur_subresult.type = Type::symb;
			cur_subresult.symbol = Symbol::EmptySet;
			cur_result.term_l = cur_subresult.make_copy();
			result.push_back(cur_result);
			delete cur_result.term_l;
			cur_result.term_l = nullptr;
			return !answer;
		} else {
			auto r_alt = to_aci(subresult);
			if (r_alt == nullptr)
				return answer;
			result.push_back(*r_alt);
			delete r_alt;
		}

		return answer;
	}
}

bool Regex::derivative_with_respect_to_str(string str, const Regex* reg_e, Regex& result) const {
	bool success = true;
	Regex cur = *reg_e;
	Regex next = *reg_e;
	for (char i : str) {
		Regex sym;
		sym.type = Type::symb;
		sym.symbol = i;
		next.clear();
		success &= derivative_with_respect_to_sym(&sym, &cur, next);
		if (!success) {
			return false;
		}
		cur = next;
	}
	result = next;
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
					string pumped_prefix;
					pumped_prefix += it->substr(0, j);
					pumped_prefix += "(" + it->substr(j, k - j) + ")*";
					pumped_prefix += it->substr(k, it->size() - k + j);
					Regex a(pumped_prefix);
					Regex b;
					Regex pumping(Type::conc, &a, &b);
					if (!derivative_with_respect_to_str(*it, this, *Regex::cast(pumping.term_r)))
						continue;
					pumping.make_language();
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
		cerr << "Regex::equivalent: regular expressions are in the same language\n";
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
	// список состояний в итоговом автомате
	// собирается в процессе работы алгоритма
	vector<Regex> fa_states;
	// список букв, по которым будут браться частные производные
	vector<Regex> symbols;
	// спсиок информации о производных {{regex, производная от r, сивол по которому
	// дифференцировали}}
	vector<vector<Regex>> partial_derivatives_by_regex;
	// мнжество всех частных производных в строковом формате,
	// чтобы не добавлять в fa_states повторяющиеся regex
	std::unordered_set<string> check;
	for (const Symbol& as : language->get_alphabet()) {
		symbols.emplace_back(as);
	}

	fa_states.push_back(*this);
	check.insert(_to_txt(false));
	for (size_t i = 0; i < fa_states.size(); i++) {
		Regex regex_state = fa_states[i];
		for (const auto& s : symbols) {
			// список частных производных от fa_states[i] по символу s
			vector<Regex> regs_der;
			regex_state.partial_symbol_derivative(s, regs_der);
			for (const auto& reg_der : regs_der) {
				partial_derivatives_by_regex.push_back({regex_state, reg_der, s});
				size_t old_checks = check.size();
				check.insert(reg_der._to_txt(false));
				if (old_checks != check.size()) {
					fa_states.push_back(reg_der);
				}
			}
		}
	}

	vector<string> name_states;

	for (auto& state : fa_states) {
		name_states.push_back(state._to_txt(false));
	}

	vector<FAState> automat_state;

	string deriv_log;

	for (size_t i = 0; i < name_states.size(); i++) {
		string state = name_states[i];
		FAState::Transitions transit;
		for (const auto& partial_derivativ : partial_derivatives_by_regex) {
			// cout << partial_derivativ[0].to_txt() << " ";
			// cout << partial_derivativ[1].to_txt() << " ";
			// cout << partial_derivativ[2].to_txt() << endl;
			deriv_log += partial_derivativ[2]._to_txt(false) + "(" +
						 partial_derivativ[0]._to_txt(false) + ")" + "\\ =\\ ";
			if (partial_derivativ[1].to_txt() == "") {
				deriv_log += "eps\\\\";
			} else {
				deriv_log += partial_derivativ[1]._to_txt(false) + "\\\\";
			}

			if (partial_derivativ[0]._to_txt(false) == state) {
				// поиск индекс состояния в которое переходим по символу из state
				auto elem_iter = find(
					name_states.begin(), name_states.end(), partial_derivativ[1]._to_txt(false));
				// записываем расстояние между begin и итератором, который указывает на состояние
				transit[partial_derivativ[2]._to_txt(false)].insert(
					std::distance(name_states.begin(), elem_iter));
			}
		}

		if (state.empty() || fa_states[i].contains_eps()) {

			if (state.empty()) {
				state = Symbol::Epsilon;
			}
			automat_state.emplace_back(int(i), state, true, transit);
		} else {
			automat_state.emplace_back(int(i), state, false, transit);
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
	vector<Regex*> list = Regex::cast(temp_copy.preorder_traversal());
	set<Symbol> lang_deann;
	for (auto& i : list) {
		i->symbol.deannote();
		lang_deann.insert(i->symbol);
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
	bool is_one_unambiguous;
	if (fa.language->is_one_unambiguous_flag_cached())
		is_one_unambiguous = fa.language->get_one_unambiguous_flag();
	else
		is_one_unambiguous = fa.is_one_unambiguous();
	if (!is_one_unambiguous) {
		if (log) {
			log->set_parameter("result", "Язык не является 1-однозначным");
		}
		return *this;
	}
	string regl;
	if (!fa.language->is_min_dfa_cached() && log) {
		log->set_parameter("cachedMINDFA", "Минимальный автомат сохранен в кэше");
	}

	FiniteAutomaton min_fa = fa.minimize(true);

	set<FAState::Transitions> final_states_transitions;
	for (int i = 0; i < min_fa.size(); i++) {
		if (min_fa.states[i].is_terminal) {
			final_states_transitions.insert(min_fa.states[i].transitions);
		}
	}

	set<Symbol> min_fa_consistent;
	// calculate a set of min_fa_consistent symbols
	for (const Symbol& symb : min_fa.language->get_alphabet()) {
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
			FAState::Transitions new_transitions;
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
	for (const Symbol& consistent_symb : min_fa_consistent) {
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
			if (!min_fa.states[i].transitions.count(consistent_symb))
				continue;
			for (int consistent_symb_transition :
				 min_fa.states[i].transitions.at(consistent_symb)) {
				reachable_by_consistent_symb.insert(consistent_symb_transition);
			}
		}
		for (int elem : reachable_by_consistent_symb) {
			FiniteAutomaton consistent_symb_automaton(0, {}, set<Symbol>());
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
			set<Symbol> consistent_symb_automaton_alphabet;
			for (int j = 0; j < consistent_symb_automaton.size(); j++) {
				consistent_symb_automaton.states[j].index = j;
				FAState::Transitions consistent_symb_automaton_state_transitions;
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
				make_shared<Language>(consistent_symb_automaton_alphabet);
			FiniteAutomaton consistent_symb_automaton_cut =
				FiniteAutomaton(consistent_symb_automaton.initial_state,
								consistent_symb_automaton.states,
								consistent_symb_automaton.language);
			for (int j = 0; j < consistent_symb_automaton.size(); j++) {
				if (consistent_symb_automaton.states[j].is_terminal) {
					FAState::Transitions new_transitions;
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
	Regex res = Regex(regl, fa.language);
	if (log) {
		log->set_parameter("result", res);
	}
	return res;
}

Regex Regex::normalize_regex(const vector<pair<Regex, Regex>>& rules, iLogTemplate* log) const {
	Regex regex = *this;
	if (log) {
		log->set_parameter("oldregex", *this);
	}
	return regex;
}