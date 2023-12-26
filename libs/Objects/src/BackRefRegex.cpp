#include "Objects/BackRefRegex.h"
#include "Objects/MemoryFiniteAutomaton.h"

using std::cerr;
using std::pair;
using std::set;
using std::string;
using std::to_string;
using std::unordered_map;
using std::unordered_set;
using std::vector;

BackRefRegex::BackRefRegex(const string& str) : BackRefRegex() {
	try {
		bool res = from_string(str, true, false);
		if (!res) {
			throw std::runtime_error("BackRefRegex::from_string() ERROR");
		}
	} catch (const std::runtime_error& re) {
		cerr << re.what() << "\n";
		exit(EXIT_FAILURE);
	}
}

BackRefRegex::BackRefRegex(const BackRefRegex& other) : AlgExpression(other) {
	cell_number = other.cell_number;
}

BackRefRegex::BackRefRegex(Type type, AlgExpression* term_l, AlgExpression* term_r)
	: AlgExpression(type, term_l, term_r) {}

void BackRefRegex::copy(const AlgExpression* other) {
	auto* tmp = cast(other);
	alphabet = tmp->alphabet;
	type = tmp->type;
	symbol = tmp->symbol;
	language = tmp->language;
	cell_number = tmp->cell_number;
	if (tmp->term_l != nullptr)
		term_l = tmp->term_l->make_copy();
	if (tmp->term_r != nullptr)
		term_r = tmp->term_r->make_copy();
}

BackRefRegex* BackRefRegex::make_copy() const {
	auto* c = new BackRefRegex;
	c->copy(this);
	return c;
}

BackRefRegex* BackRefRegex::make() const {
	return new BackRefRegex;
}

template <typename T> BackRefRegex* BackRefRegex::cast(T* ptr, bool not_null_ptr) {
	auto* r = dynamic_cast<BackRefRegex*>(ptr);
	if (!r && not_null_ptr) {
		throw std::runtime_error("Failed to cast to BackRefRegex");
	}

	return r;
}

template <typename T> const BackRefRegex* BackRefRegex::cast(const T* ptr, bool not_null_ptr) {
	auto* r = dynamic_cast<const BackRefRegex*>(ptr);
	if (!r && not_null_ptr) {
		throw std::runtime_error("Failed to cast to BackRefRegex");
	}

	return r;
}

template <typename T> vector<BackRefRegex*> BackRefRegex::cast(vector<T*> ptrs, bool not_null_ptr) {
	vector<BackRefRegex*> regexPointers;
	for (T* ptr : ptrs) {
		auto* r = dynamic_cast<BackRefRegex*>(ptr);
		if (!r && not_null_ptr) {
			throw std::runtime_error("Failed to cast to BackRefRegex");
		}

		regexPointers.push_back(r);
	}

	return regexPointers;
}

string BackRefRegex::to_txt() const {
	BackRefRegex *br_term_l, *br_term_r;
	string str1, str2;
	if (term_l) {
		str1 = term_l->to_txt();
		br_term_l = cast(term_l);
	}
	if (term_r) {
		str2 = term_r->to_txt();
		br_term_r = cast(term_r);
	}
	string symb;
	switch (type) {
	case Type::conc:
		if (term_l && br_term_l->type == Type::alt) {
			str1 = "(" + str1 + ")";
		}
		if (term_r && br_term_r->type == Type::alt) {
			str2 = "(" + str2 + ")";
		}
		break;
	case Type::eps:
		symb = "";
		break;
	case Type::alt:
		symb = '|';
		break;
	case Type::star:
		symb = '*';
		if (!is_terminal_type(br_term_l->type))
			// ставим скобки при итерации, если символов > 1
			str1 = "(" + str1 + ")";
		break;
	case Type::memoryWriter:
		str1 = "[" + str1 + "]:" + to_string(cell_number);
		break;
	case Type::symb:
	case Type::ref:
		symb = symbol;
		break;
	}

	return str1 + symb + str2;
}

string BackRefRegex::type_to_str() const {
	if (symbol != "")
		return symbol;
	switch (type) {
	case Type::eps:
		return "ε";
	case Type::alt:
		return "|";
	case Type::conc:
		return ".";
	case Type::star:
		return "*";
	case Type::symb:
		return "symb";
	case Type::ref:
		return "&" + to_string(cell_number);
	case Type::memoryWriter:
		return "[" + to_string(cell_number) + "]";
	default:
		break;
	}
	return {};
}

BackRefRegex* BackRefRegex::expr(const vector<AlgExpression::Lexeme>& lexemes, int index_start,
								 int index_end) {
	AlgExpression* p;
	p = scan_alt(lexemes, index_start, index_end);
	if (!p) {
		p = scan_conc(lexemes, index_start, index_end);
	}
	if (!p) {
		p = scan_star(lexemes, index_start, index_end);
	}
	if (!p) {
		p = scan_symb(lexemes, index_start, index_end);
	}
	if (!p) {
		p = scan_ref(lexemes, index_start, index_end);
	}
	if (!p) {
		p = scan_eps(lexemes, index_start, index_end);
	}
	if (!p) {
		p = scan_par(lexemes, index_start, index_end);
	}
	if (!p) {
		p = scan_square_br(lexemes, index_start, index_end);
	}
	return cast(p, false);
}

BackRefRegex* BackRefRegex::scan_ref(const vector<AlgExpression::Lexeme>& lexemes, int index_start,
									 int index_end) {
	BackRefRegex* p = nullptr;
	if (index_start >= lexemes.size() || (index_end - index_start > 1) ||
		lexemes[index_start].type != Lexeme::Type::ref) {
		return nullptr;
	}

	p = new BackRefRegex();
	p->symbol = lexemes[index_start].symbol;
	p->type = AlgExpression::ref;
	p->cell_number = lexemes[index_start].number;
	return p;
}

BackRefRegex* BackRefRegex::scan_square_br(const vector<AlgExpression::Lexeme>& lexemes,
										   int index_start, int index_end) {
	BackRefRegex* p = nullptr;
	if ((index_end - 1) >= lexemes.size() ||
		(lexemes[index_start].type != Lexeme::Type::squareBrL ||
		 lexemes[index_end - 1].type != Lexeme::Type::squareBrR)) {
		return nullptr;
	}

	BackRefRegex* l = expr(lexemes, index_start + 1, index_end - 1);
	if (l == nullptr || l->type == AlgExpression::eps) {
		delete l;
		return p;
	}

	p = new BackRefRegex();
	p->term_l = l;
	p->type = AlgExpression::memoryWriter;
	p->cell_number = lexemes[index_start].number;
	p->alphabet = l->alphabet;
	return p;
}

vector<MFAState> BackRefRegex::_to_mfa() const {
	vector<MFAState> states; // вектор состояний нового автомата
	// состояния левого автомата относительно операции
	vector<MFAState> left_states;
	// состояния правого автомата относительно операции
	vector<MFAState> right_states;
	// сдвиг индексов состояний
	int offset;

	switch (type) {
	case eps:
		states.emplace_back(true);
		return states;
	case symb:
	case ref:
		states.emplace_back(false);
		states.emplace_back(true);

		states[0].set_transition(MFATransition(1), symbol);
		return states;
	case memoryWriter:
		left_states = BackRefRegex::cast(term_l)->_to_mfa();

		// ставим на переходы из начального состояния открытие памяти
		states.emplace_back(left_states[0].is_terminal);
		for (const auto& [symbol, states_to] : left_states[0].transitions)
			for (const auto& transition : states_to) {
				MFATransition::MemoryActions new_memory_actions = transition.memory_actions;
				new_memory_actions[cell_number] = MFATransition::MemoryAction::open;
				states[0].set_transition(MFATransition(transition.to, new_memory_actions), symbol);
			}

		for (int i = 1; i < left_states.size(); i++) {
			const MFAState& state = left_states[i];
			states.emplace_back(false);
			MFAState& new_state = states[i];

			for (const auto& [symbol, states_to] : state.transitions)
				for (const auto& transition : states_to)
					new_state.set_transition(transition, symbol);

			// из конечных состояний добавляем эпсилон-переход, закрывающий память
			if (state.is_terminal)
				new_state.set_transition(
					MFATransition(left_states.size(),
								  {{cell_number, MFATransition::MemoryAction::close}}),
					Symbol::epsilon());
		}

		// добавляем финальное состояние
		states.emplace_back(true);

		return states;
	case alt:
		left_states = BackRefRegex::cast(term_l)->_to_mfa();
		right_states = BackRefRegex::cast(term_r)->_to_mfa();
		offset = left_states.size() - 1;

		states.emplace_back(left_states[0].is_terminal || right_states[0].is_terminal);
		for (const auto& [symbol, states_to] : left_states[0].transitions)
			for (const auto& transition : states_to)
				states[0].set_transition(transition, symbol);
		for (const auto& [symbol, states_to] : right_states[0].transitions)
			for (const auto& transition : states_to)
				states[0].set_transition(
					MFATransition(transition.to + offset, transition.memory_actions), symbol);

		for (int i = 1; i < left_states.size(); i++) {
			const MFAState& state = left_states[i];
			states.emplace_back(state.is_terminal);
			MFAState& new_state = states[i];

			for (const auto& [symbol, states_to] : state.transitions)
				for (const auto& transition : states_to)
					new_state.set_transition(transition, symbol);
		}

		for (int i = 1; i < right_states.size(); i++) {
			const MFAState& state = right_states[i];
			states.emplace_back(state.is_terminal);
			MFAState& new_state = states[i + offset];

			for (const auto& [symbol, states_to] : state.transitions)
				for (const auto& transition : states_to)
					new_state.set_transition(
						MFATransition(transition.to + offset, transition.memory_actions), symbol);
		}

		return states;
	case conc:
		left_states = BackRefRegex::cast(term_l)->_to_mfa();
		right_states = BackRefRegex::cast(term_r)->_to_mfa();
		offset = left_states.size() - 1;

		for (int i = 0; i < left_states.size(); i++) {
			const MFAState& state = left_states[i];
			states.emplace_back(state.is_terminal && right_states[0].is_terminal);
			MFAState& new_state = states[i];

			for (const auto& [symbol, states_to] : state.transitions)
				for (const auto& transition : states_to)
					new_state.set_transition(transition, symbol);

			if (state.is_terminal)
				for (const auto& [symbol, states_to] : right_states[0].transitions)
					for (const auto& transition : states_to)
						new_state.set_transition(
							MFATransition(transition.to + offset, transition.memory_actions),
							symbol);
		}

		for (int i = 1; i < right_states.size(); i++) {
			const MFAState& state = right_states[i];
			states.emplace_back(state.is_terminal);
			MFAState& new_state = states[i + offset];

			for (const auto& [symbol, states_to] : state.transitions)
				for (const auto& transition : states_to)
					new_state.set_transition(
						MFATransition(transition.to + offset, transition.memory_actions), symbol);
		}

		return states;
	case star:
		left_states = BackRefRegex::cast(term_l)->_to_mfa();

		states.emplace_back(true);
		for (const auto& [symbol, states_to] : left_states[0].transitions)
			for (const auto& transition : states_to)
				states[0].set_transition(transition, symbol);

		for (int i = 1; i < left_states.size(); i++) {
			const MFAState& state = left_states[i];
			states.emplace_back(state.is_terminal);
			MFAState& new_state = states[i];

			for (const auto& [symbol, states_to] : state.transitions)
				for (const auto& transition : states_to)
					new_state.set_transition(transition, symbol);

			// дублируем переходы из начального для конечных состояний
			if (state.is_terminal)
				for (const auto& [symbol, states_to] : left_states[0].transitions)
					for (const auto& transition : states_to)
						new_state.set_transition(transition, symbol);
		}

		return states;
	default:
		break;
	}
	return {};
}

MemoryFiniteAutomaton BackRefRegex::to_mfa(iLogTemplate* log) const {
	auto states = _to_mfa();
	for (int i = 0; i < states.size(); i++) {
		states[i].index = i;
		states[i].identifier = to_string(i);
	}
	MemoryFiniteAutomaton mfa(0, states, language);
	if (log) {
	}
	return mfa;
}

vector<BackRefRegex*> BackRefRegex::preorder_traversal(std::unordered_set<int> _in_cells,
													   std::unordered_set<int> _first_in_cells,
													   std::unordered_set<int> _last_in_cells) {
	vector<BackRefRegex*> l, r;
	bool l_contains_eps, r_contains_eps;

	switch (type) {
	case alt:
		l = cast(term_l)->preorder_traversal(_in_cells, _first_in_cells, _last_in_cells);
		r = cast(term_r)->preorder_traversal(_in_cells, _first_in_cells, _last_in_cells);
		l.insert(l.end(), r.begin(), r.end());
		return l;
	case conc:
		l_contains_eps = cast(term_l)->contains_eps();
		r_contains_eps = cast(term_r)->contains_eps();
		l = cast(term_l)->preorder_traversal(
			_in_cells, _first_in_cells, r_contains_eps ? _last_in_cells : unordered_set<int>());
		r = cast(term_r)->preorder_traversal(
			_in_cells, l_contains_eps ? _first_in_cells : unordered_set<int>(), _last_in_cells);
		l.insert(l.end(), r.begin(), r.end());
		return l;
	case eps:
		return {};
	case star:
		l = cast(term_l)->preorder_traversal(_in_cells, _first_in_cells, _last_in_cells);
		return l;
	case memoryWriter:
		_in_cells.insert(cell_number);
		_first_in_cells.insert(cell_number);
		_last_in_cells.insert(cell_number);
		return cast(term_l)->preorder_traversal(_in_cells, _first_in_cells, _last_in_cells);
	case symb:
	case ref:
		in_cells = _in_cells;
		first_in_cells = _first_in_cells;
		last_in_cells = _last_in_cells;
		return {this};
	}
}

void BackRefRegex::get_cells_under_iteration(unordered_set<int>& iteration_over_cells) const {
	switch (type) {
	case Type::alt:
		cast(term_l)->get_cells_under_iteration(iteration_over_cells);
		cast(term_r)->get_cells_under_iteration(iteration_over_cells);
		return;
	case Type::star:
		cast(term_l)->get_cells_under_iteration(iteration_over_cells);
		return;
	case Type::memoryWriter:
		iteration_over_cells.insert(cell_number);
		cast(term_l)->get_cells_under_iteration(iteration_over_cells);
		return;
	default:
		return;
	}
}

unordered_map<int, vector<pair<int, unordered_set<int>>>> BackRefRegex::get_follow() const {
	unordered_map<int, vector<pair<int, unordered_set<int>>>> l, r;
	vector<AlgExpression*> first, last;
	unordered_set<int> iteration_over_cells;
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
				l[i->get_symbol().last_linearization_number()].emplace_back(
					j->get_symbol().last_linearization_number(), false);
			}
		}

		return l;
	case Type::star:
		l = cast(term_l)->get_follow();
		last = cast(term_l)->get_last_nodes();
		first = cast(term_l)->get_first_nodes();
		get_cells_under_iteration(iteration_over_cells);
		for (auto& i : last) {
			for (auto& j : first) {
				l[i->get_symbol().last_linearization_number()].emplace_back(
					j->get_symbol().last_linearization_number(), iteration_over_cells);
			}
		}

		return l;
	case Type::memoryWriter:
		return cast(term_l)->get_follow();
	default:
		return {};
	}
}

MemoryFiniteAutomaton BackRefRegex::to_mfa_additional(iLogTemplate* log) const {
	BackRefRegex temp_copy(*this);
	vector<BackRefRegex*> terms = temp_copy.preorder_traversal({}, {}, {});
	for (size_t i = 0; i < terms.size(); i++) {
		terms[i]->symbol.linearize(static_cast<int>(i));
	}
	// Множество начальных состояний
	vector<BackRefRegex*> first = cast(temp_copy.get_first_nodes());
	// Множество конечных состояний
	vector<BackRefRegex*> last = cast(temp_copy.get_last_nodes());
	// множество состояний, которым предшествует символ (ключ - линеаризованный номер)
	unordered_map<int, vector<pair<int, unordered_set<int>>>> following_states =
		temp_copy.get_follow();
	int eps_in = this->contains_eps();
	vector<MFAState> states; // состояния автомата

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
		str_last += Symbol::epsilon();
	}

	for (const auto& i : following_states) {
		for (const auto& [to, iteration_in_cell] : i.second) {
			str_follow += "(" + string(terms[i.first]->symbol) + "," + string(terms[to]->symbol) +
						  ")" + "\\ ";
		}
	}

	vector<Symbol> delinearized_symbols;
	for (int i = 0; i < terms.size(); i++) {
		delinearized_symbols.push_back(terms[i]->symbol);
		delinearized_symbols[i].delinearize();
	}

	MFAState::Transitions start_state_transitions;
	for (auto& i : first) {
		start_state_transitions[delinearized_symbols[i->symbol.last_linearization_number()]].insert(
			MFATransition(i->symbol.last_linearization_number() + 1, i->first_in_cells, {}));
	}

	if (eps_in) {
		states.emplace_back(0, "S", true, start_state_transitions);
	} else {
		states.emplace_back(0, "S", false, start_state_transitions);
	}

	unordered_set<int> last_terms;
	for (auto& i : last) {
		last_terms.insert(i->symbol.last_linearization_number());
	}

	for (size_t i = 0; i < terms.size(); i++) {
		Symbol& symb = terms[i]->symbol;
		MFAState::Transitions transitions;

		for (const auto& [to, iteration_over_cells] :
			 following_states[symb.last_linearization_number()]) {
			transitions[delinearized_symbols[to]].insert(MFATransition(to + 1,
																	   terms[to]->first_in_cells,
																	   terms[i]->in_cells,
																	   iteration_over_cells,
																	   terms[i]->last_in_cells,
																	   terms[to]->in_cells));
		}

		// В last_terms номера конечных лексем => last_terms.count проверяет есть ли
		// номер лексемы в списке конечных лексем (является ли состояние конечным)
		states.emplace_back(
			i + 1, symb, last_terms.count(symb.last_linearization_number()), transitions);
	}

	MemoryFiniteAutomaton mfa(0, states, language);
	if (log) {
	}
	return mfa;
}