#include "Objects/BackRefRegex.h"
#include "Objects/Language.h"

using std::cerr;
using std::pair;
using std::set;
using std::string;
using std::swap;
using std::to_string;
using std::tuple;
using std::unordered_map;
using std::unordered_set;
using std::vector;

BackRefRegex::BackRefRegex(const string& str) : BackRefRegex() {
	try {
		bool res = from_string(str, true, false);
		if (!res) {
			throw std::logic_error("BackRefRegex::from_string() ERROR");
		}
	} catch (const std::runtime_error& re) {
		cerr << re.what() << "\n";
		exit(EXIT_FAILURE);
	}
}

BackRefRegex::BackRefRegex(const BackRefRegex& other) : AlgExpression(other) {
	cell_number = other.cell_number;
	lin_number = other.lin_number;
}

BackRefRegex::BackRefRegex(const Regex* regex, const Alphabet& _alphabet) : BackRefRegex(regex) {
	alphabet = _alphabet;
	language = std::make_shared<Language>(_alphabet);
}

BackRefRegex::BackRefRegex(const Regex* regex)
	: AlgExpression(regex->get_type(), regex->get_symbol()) {
	const Regex* regex_term_l = Regex::cast(regex->get_term_l(), false);
	const Regex* regex_term_r = Regex::cast(regex->get_term_r(), false);
	if (regex_term_l)
		term_l = new BackRefRegex(regex_term_l);
	if (regex_term_r)
		term_r = new BackRefRegex(regex_term_r);
}

BackRefRegex& BackRefRegex::operator=(const BackRefRegex& other) {
	if (this != &other) {
		clear();
		copy(&other);
	}
	return *this;
}

void BackRefRegex::copy(const AlgExpression* other) {
	auto* tmp = cast(other);
	alphabet = tmp->alphabet;
	type = tmp->type;
	symbol = tmp->symbol;
	language = tmp->language;
	cell_number = tmp->cell_number;
	lin_number = tmp->lin_number;
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
	if (!p)
		p = scan_conc(lexemes, index_start, index_end);
	if (!p)
		p = scan_star(lexemes, index_start, index_end);
	if (!p)
		p = scan_symb(lexemes, index_start, index_end);
	if (!p)
		p = scan_ref(lexemes, index_start, index_end);
	if (!p)
		p = scan_eps(lexemes, index_start, index_end);
	if (!p)
		p = scan_par(lexemes, index_start, index_end);
	if (!p)
		p = scan_square_br(lexemes, index_start, index_end);
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
	if (l == nullptr) {
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

bool BackRefRegex::equals(const AlgExpression* other) const {
	return cell_number == cast(other)->cell_number;
}

bool BackRefRegex::equal(const BackRefRegex& r1, const BackRefRegex& r2, iLogTemplate* log) {
	bool result = equality_checker(&r1, &r2);
	if (log) {
		log->set_parameter("brefregex1", r1);
		log->set_parameter("brefregex2", r2);
		log->set_parameter("result", result);
	}
	return result;
}

vector<MFAState> BackRefRegex::_to_mfa() const {
	// вектор состояний нового автомата
	vector<MFAState> states;
	// состояния левого автомата относительно операции
	vector<MFAState> left_states;
	// состояния правого автомата относительно операции
	vector<MFAState> right_states;
	// сдвиг индексов состояний
	int offset;

	switch (type) {
	case eps:
		states.emplace_back(false);
		// переход по пустому слову в конечное состояние
		states[0].add_transition(MFATransition(states.size()), Symbol::Epsilon);
		// добавляем конечное состояние для пустого перехода
		states.emplace_back(true);
		return states;
	case symb:
	case ref:
		states.emplace_back(false);
		states.emplace_back(true);

		states[0].add_transition(MFATransition(1), symbol);
		return states;
	case memoryWriter:
		left_states = BackRefRegex::cast(term_l)->_to_mfa();

		// ставим на переходы из начального состояния открытие памяти
		states.emplace_back(left_states[0].is_terminal);
		for (const auto& [symbol, states_to] : left_states[0].transitions)
			for (const auto& transition : states_to) {
				MFATransition::MemoryActions new_memory_actions = transition.memory_actions;
				new_memory_actions[cell_number] = MFATransition::MemoryAction::open;
				states[0].add_transition(MFATransition(transition.to, new_memory_actions), symbol);
			}

		for (int i = 1; i < left_states.size(); i++) {
			const MFAState& state = left_states[i];
			states.emplace_back(false);
			MFAState& new_state = states[i];

			for (const auto& [symbol, states_to] : state.transitions)
				for (const auto& transition : states_to)
					new_state.add_transition(transition, symbol);

			// из конечных состояний добавляем эпсилон-переход, закрывающий память
			if (state.is_terminal)
				new_state.add_transition(
					MFATransition(left_states.size(),
								  {{cell_number, MFATransition::MemoryAction::close}}),
					Symbol::Epsilon);
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
				states[0].add_transition(transition, symbol);
		for (const auto& [symbol, states_to] : right_states[0].transitions)
			for (const auto& transition : states_to)
				states[0].add_transition(
					MFATransition(transition.to + offset, transition.memory_actions), symbol);

		for (int i = 1; i < left_states.size(); i++) {
			const MFAState& state = left_states[i];
			states.emplace_back(state.is_terminal);
			MFAState& new_state = states[i];

			for (const auto& [symbol, states_to] : state.transitions)
				for (const auto& transition : states_to)
					new_state.add_transition(transition, symbol);
		}

		for (int i = 1; i < right_states.size(); i++) {
			const MFAState& state = right_states[i];
			states.emplace_back(state.is_terminal);
			MFAState& new_state = states[i + offset];

			for (const auto& [symbol, states_to] : state.transitions)
				for (const auto& transition : states_to)
					new_state.add_transition(
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
					new_state.add_transition(transition, symbol);

			if (state.is_terminal)
				for (const auto& [symbol, states_to] : right_states[0].transitions)
					for (const auto& transition : states_to)
						new_state.add_transition(
							MFATransition(transition.to + offset, transition.memory_actions),
							symbol);
		}

		for (int i = 1; i < right_states.size(); i++) {
			const MFAState& state = right_states[i];
			states.emplace_back(state.is_terminal);
			MFAState& new_state = states[i + offset];

			for (const auto& [symbol, states_to] : state.transitions)
				for (const auto& transition : states_to)
					new_state.add_transition(
						MFATransition(transition.to + offset, transition.memory_actions), symbol);
		}

		return states;
	case star:
		left_states = BackRefRegex::cast(term_l)->_to_mfa();

		states.emplace_back(left_states[0].is_terminal);
		for (const auto& [symbol, states_to] : left_states[0].transitions)
			for (const auto& transition : states_to)
				states[0].add_transition(transition, symbol);

		for (int i = 1; i < left_states.size(); i++) {
			const MFAState& state = left_states[i];
			states.emplace_back(state.is_terminal);
			MFAState& new_state = states[i];

			for (const auto& [symbol, states_to] : state.transitions)
				for (const auto& transition : states_to)
					new_state.add_transition(transition, symbol);

			// дублируем переходы из начального для конечных состояний
			if (state.is_terminal)
				for (const auto& [symbol, states_to] : left_states[0].transitions)
					for (const auto& transition : states_to)
						new_state.add_transition(transition, symbol);
		}

		// переход по пустому слову в новое конечное состояние (случай нуля итераций)
		states[0].add_transition(MFATransition(left_states.size()), Symbol::Epsilon);
		// добавляем конечное состояние для пустого перехода
		states.emplace_back(true);

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
		log->set_parameter("brefregex", *this);
		log->set_parameter("result", mfa);
	}
	return mfa;
}

Cell BackRefRegex::get_cell() const {
	return {cell_number, lin_number};
}

void BackRefRegex::preorder_traversal(vector<BackRefRegex*>& terms, int& lin_counter,
									  vector<unordered_set<int>>& in_lin_cells,
									  vector<CellSet>& first_in_cells,
									  vector<CellSet>& last_in_cells,
									  unordered_set<int> cur_in_lin_cells,
									  CellSet cur_first_in_cells, CellSet cur_last_in_cells) {
	switch (type) {
	case alt:
		cast(term_l)->preorder_traversal(terms,
										 lin_counter,
										 in_lin_cells,
										 first_in_cells,
										 last_in_cells,
										 cur_in_lin_cells,
										 cur_first_in_cells,
										 cur_last_in_cells);
		cast(term_r)->preorder_traversal(terms,
										 lin_counter,
										 in_lin_cells,
										 first_in_cells,
										 last_in_cells,
										 cur_in_lin_cells,
										 cur_first_in_cells,
										 cur_last_in_cells);
		return;
	case conc: {
		bool l_contains_eps = cast(term_l)->contains_eps();
		bool r_contains_eps = cast(term_r)->contains_eps();
		cast(term_l)->preorder_traversal(terms,
										 lin_counter,
										 in_lin_cells,
										 first_in_cells,
										 last_in_cells,
										 cur_in_lin_cells,
										 cur_first_in_cells,
										 r_contains_eps ? cur_last_in_cells : CellSet());
		cast(term_r)->preorder_traversal(terms,
										 lin_counter,
										 in_lin_cells,
										 first_in_cells,
										 last_in_cells,
										 cur_in_lin_cells,
										 l_contains_eps ? cur_first_in_cells : CellSet(),
										 cur_last_in_cells);
		return;
	}
	case star:
		cast(term_l)->preorder_traversal(terms,
										 lin_counter,
										 in_lin_cells,
										 first_in_cells,
										 last_in_cells,
										 cur_in_lin_cells,
										 cur_first_in_cells,
										 cur_last_in_cells);
		return;
	case memoryWriter:
		lin_number = lin_counter++;
		cur_in_lin_cells.insert(lin_number);
		cur_first_in_cells.insert(get_cell());
		cur_last_in_cells.insert(get_cell());
		cast(term_l)->preorder_traversal(terms,
										 lin_counter,
										 in_lin_cells,
										 first_in_cells,
										 last_in_cells,
										 cur_in_lin_cells,
										 cur_first_in_cells,
										 cur_last_in_cells);
		return;
	case symb:
	case ref:
		in_lin_cells.push_back(cur_in_lin_cells);
		first_in_cells.push_back(cur_first_in_cells);
		last_in_cells.push_back(cur_last_in_cells);
		symbol.linearize(terms.size());
		terms.push_back(this);
		return;
	default:
		return;
	}
}

void BackRefRegex::calculate_may_be_eps(unordered_map<int, vector<BackRefRegex*>>& memory_writers) {
	switch (type) {
	case alt: {
		auto memory_writers_copy = memory_writers;
		cast(term_l)->calculate_may_be_eps(memory_writers);
		cast(term_r)->calculate_may_be_eps(memory_writers_copy);
		for (const auto& [num, refs_to] : memory_writers_copy)
			for (const auto& memory_writer : refs_to)
				memory_writers[num].push_back(memory_writer);
		return;
	}
	case conc:
		cast(term_l)->calculate_may_be_eps(memory_writers);
		cast(term_r)->calculate_may_be_eps(memory_writers);
		return;
	case star:
		cast(term_l)->calculate_may_be_eps(memory_writers);
		cast(term_l)->calculate_may_be_eps(memory_writers);
		break;
	case memoryWriter:
		memory_writers[cell_number] = {this};
		cast(term_l)->calculate_may_be_eps(memory_writers);
		return;
	case ref: {
		if (auto it_ref_to = memory_writers.find(cell_number); it_ref_to != memory_writers.end())
			for (const auto& memory_writer : it_ref_to->second)
				may_be_eps |= memory_writer->contains_eps();
		else
			may_be_eps = true;
		return;
	}
	default:
		return;
	}
}

bool BackRefRegex::contains_eps() const {
	switch (type) {
	case Type::alt:
		return cast(term_l)->contains_eps() || cast(term_r)->contains_eps();
	case Type::conc:
		return cast(term_l)->contains_eps() && cast(term_r)->contains_eps();
	case Type::star:
	case Type::eps:
		return true;
	case Type::memoryWriter:
		return cast(term_l)->contains_eps();
	case Type::ref:
		return may_be_eps;
	default:
		return false;
	}
}

pair<bool, ToResetMap> BackRefRegex::contains_eps_tracking_resets() const {
	pair<bool, ToResetMap> l, r;
	switch (type) {
	case Type::alt:
		l = cast(term_l)->contains_eps_tracking_resets();
		r = cast(term_r)->contains_eps_tracking_resets();
		if (l.first)
			for (auto& i : r.second)
				i.second.first = false;
		if (r.first)
			for (auto& i : l.second)
				i.second.first = false;
		l.first |= r.first;
		l.second.insert(r.second.begin(), r.second.end());
		return l;
	case Type::conc:
		l = cast(term_l)->contains_eps_tracking_resets();
		r = cast(term_r)->contains_eps_tracking_resets();
		l.first &= r.first;
		if (l.first)
			l.second.insert(r.second.begin(), r.second.end());
		else
			l.second = {};
		return l;
	case Type::star: {
		auto t = to_txt();
		l = cast(term_l)->contains_eps_tracking_resets();
		for (auto& i : l.second)
			i.second.first = false;
		return {true, l.second};
	}
	case Type::eps:
		return {true, {}};
	case Type::memoryWriter:
		l = cast(term_l)->contains_eps_tracking_resets();
		if (l.first) {
			CellSet depends_on;
			for (auto& [cell, emptiness_info] : l.second) {
				// если ячейка не может быть пропущена, добавляем в список тех,
				// от пустоты которых зависит пустота текущей
				if (emptiness_info.first)
					depends_on.insert(cell);
				// вложенные ячейки не могут быть сброшены, если не сброшена внешняя
				emptiness_info.second.insert(get_cell());
			}
			l.second.insert({get_cell(), {true, depends_on}});
			return l;
		} else {
			return {false, {}};
		}
	case Type::ref:
		return {may_be_eps, {}};
	default:
		return {false, {}};
	}
}

std::vector<std::pair<AlgExpression*, ToResetMap>> BackRefRegex::get_first_nodes_tracking_resets() {
	vector<pair<AlgExpression*, ToResetMap>> l, r;
	pair<bool, ToResetMap> is_eps;
	switch (type) {
	case Type::alt:
		l = cast(term_l)->get_first_nodes_tracking_resets();
		r = cast(term_r)->get_first_nodes_tracking_resets();
		l.insert(l.end(), r.begin(), r.end());
		return l;
	case Type::conc:
		l = cast(term_l)->get_first_nodes_tracking_resets();
		is_eps = cast(term_l)->contains_eps_tracking_resets();
		if (is_eps.first) {
			r = cast(term_r)->get_first_nodes_tracking_resets();
			for (auto& [i, following_reset] : r) {
				following_reset.insert(is_eps.second.begin(), is_eps.second.end());
				l.emplace_back(i, following_reset);
			}
		}
		return l;
	case Type::star:
		is_eps = contains_eps_tracking_resets();
		l = cast(term_l)->get_first_nodes_tracking_resets();
		for (auto& [i, reset] : l) {
			reset.insert(is_eps.second.begin(), is_eps.second.end());
		}
		return l;
	case Type::memoryWriter:
		return cast(term_l)->get_first_nodes_tracking_resets();
	case AlgExpression::eps:
		return {};
	default:
		return {{this, {}}};
	}
}

std::vector<std::pair<AlgExpression*, ToResetMap>> BackRefRegex::get_last_nodes_tracking_resets() {
	vector<pair<AlgExpression*, ToResetMap>> l, r;
	pair<bool, ToResetMap> is_eps;
	switch (type) {
	case Type::alt:
		l = cast(term_l)->get_last_nodes_tracking_resets();
		r = cast(term_r)->get_last_nodes_tracking_resets();
		l.insert(l.end(), r.begin(), r.end());
		return l;
	case Type::conc:
		r = cast(term_r)->get_last_nodes_tracking_resets();
		is_eps = cast(term_r)->contains_eps_tracking_resets();
		if (is_eps.first) {
			l = cast(term_l)->get_last_nodes_tracking_resets();
			for (auto& [i, prev_reset] : l) {
				prev_reset.insert(is_eps.second.begin(), is_eps.second.end());
				r.emplace_back(i, prev_reset);
			}
		}
		return r;
	case Type::star:
		is_eps = contains_eps_tracking_resets();
		l = cast(term_l)->get_last_nodes_tracking_resets();
		for (auto& [i, reset] : l) {
			reset.insert(is_eps.second.begin(), is_eps.second.end());
		}
		return l;
	case Type::memoryWriter:
		return cast(term_l)->get_last_nodes_tracking_resets();
	case AlgExpression::eps:
		return {};
	default:
		return {{this, {}}};
	}
}

void generate_combinations(const CellSet& s, int length,
						   vector<Cell>& combination, // NOLINT(runtime/references)
						   int start,
						   vector<CellSet>& result) { // NOLINT(runtime/references)
	if (length == 0) {
		result.emplace_back(combination.begin(), combination.end());
		return;
	}

	for (int i = start; i < s.size(); ++i) {
		combination.push_back(*std::next(s.begin(), i));
		generate_combinations(s, length - 1, combination, i + 1, result);
		combination.pop_back();
	}
}

vector<CellSet> get_all_combinations(const CellSet& s) {
	vector<CellSet> result;
	for (int length = 1; length <= s.size(); ++length) {
		vector<Cell> combination;
		generate_combinations(s, length, combination, 0, result);
	}
	return result;
}

vector<CellSet> merge_to_reset_maps(const vector<ToResetMap>& maps) {
	ToResetMap merged;
	CellSet to_reset, maybe_to_reset;
	for (const auto& map : maps) {
		for (const auto& [cell, emptiness_info] : map) {
			if (emptiness_info.first)
				to_reset.insert(cell);
			else
				maybe_to_reset.insert(cell);

			if (auto it = merged.find(cell); it != merged.end()) {
				it->second.first &= emptiness_info.first;
				it->second.second = get_intersection(it->second.second, emptiness_info.second);
			} else {
				merged[cell] = emptiness_info;
			}
		}
	}

	unordered_map<Cell, CellSet, Cell::Hasher> must_have_same_actions;
	for (const auto& [cell, emptiness_info] : merged)
		for (const auto& depends_on_cell : emptiness_info.second) {
			must_have_same_actions[cell].insert(depends_on_cell);
		}

	vector<CellSet> res({to_reset});
	auto t = get_all_combinations(maybe_to_reset);
	for (const auto& i : get_all_combinations(maybe_to_reset)) {
		// пропускаем комбинации, которые не содержат всех зависимых друг от друга ячеек
		bool skip = std::any_of(i.begin(), i.end(), [&](const auto& cell) {
			return must_have_same_actions.count(cell) &&
				   std::any_of(must_have_same_actions.at(cell).begin(),
							   must_have_same_actions.at(cell).end(),
							   [&](const auto& must_have_same_actions_with) {
								   // если в комбинации или в to_reset нет ячейки,
								   // от которой зависит cell
								   return !i.count(must_have_same_actions_with) &&
										  !to_reset.count(must_have_same_actions_with);
							   });
		});
		if (skip)
			continue;

		CellSet temp(to_reset);
		temp.insert(i.begin(), i.end());
		res.emplace_back(temp);
	}
	return {res};
}

void BackRefRegex::get_cells_under_iteration(unordered_set<int>& iteration_over_cells) const {
	switch (type) {
	case Type::alt:
		cast(term_l)->get_cells_under_iteration(iteration_over_cells);
		cast(term_r)->get_cells_under_iteration(iteration_over_cells);
		return;
	case Type::conc:
		if (cast(term_l)->contains_eps() || cast(term_r)->contains_eps()) {
			cast(term_l)->get_cells_under_iteration(iteration_over_cells);
			cast(term_r)->get_cells_under_iteration(iteration_over_cells);
		}
		return;
	case Type::star:
		cast(term_l)->get_cells_under_iteration(iteration_over_cells);
		return;
	case Type::memoryWriter:
		iteration_over_cells.insert(lin_number);
		cast(term_l)->get_cells_under_iteration(iteration_over_cells);
		return;
	default:
		return;
	}
}

void BackRefRegex::get_follow(
	vector<vector<tuple<int, unordered_set<int>, CellSet>>>& following_states) const {
	vector<pair<AlgExpression*, ToResetMap>> first, last;
	switch (type) {
	case Type::alt:
		cast(term_l)->get_follow(following_states);
		cast(term_r)->get_follow(following_states);
		return;
	case Type::conc:
		cast(term_l)->get_follow(following_states);
		cast(term_r)->get_follow(following_states);

		last = cast(term_l)->get_last_nodes_tracking_resets();
		first = cast(term_r)->get_first_nodes_tracking_resets();
		for (auto& [i, last_to_reset] : last) {
			for (auto& [j, first_to_reset] : first) {
				for (const auto& k : merge_to_reset_maps({last_to_reset, first_to_reset}))
					following_states[i->get_symbol().last_linearization_number()].emplace_back(
						j->get_symbol().last_linearization_number(), unordered_set<int>(), k);
			}
		}
		return;
	case Type::star: {
		cast(term_l)->get_follow(following_states);
		pair<bool, ToResetMap> is_eps = contains_eps_tracking_resets();
		last = cast(term_l)->get_last_nodes_tracking_resets();
		first = cast(term_l)->get_first_nodes_tracking_resets();
		unordered_set<int> iteration_over_cells;
		get_cells_under_iteration(iteration_over_cells);
		for (auto& [i, last_to_reset] : last) {
			for (auto& [j, first_to_reset] : first) {
				vector<CellSet> to_reset;
				if (i != j)
					to_reset = merge_to_reset_maps({last_to_reset, first_to_reset, is_eps.second});
				else
					to_reset = merge_to_reset_maps({last_to_reset, first_to_reset});
				for (const auto& k : to_reset)
					following_states[i->get_symbol().last_linearization_number()].emplace_back(
						j->get_symbol().last_linearization_number(), iteration_over_cells, k);
			}
		}
		return;
	}
	case Type::memoryWriter:
		return cast(term_l)->get_follow(following_states);
	default:
		return;
	}
}

MemoryFiniteAutomaton BackRefRegex::to_mfa_additional(iLogTemplate* log) const {
	BackRefRegex temp_copy(*this);
	vector<BackRefRegex*> terms;
	// номера ячеек, в которых "находится" терм
	// (лежит в поддеревьях memoryWriter с этими номерами)
	vector<unordered_set<int>> in_lin_cells;
	// номера ячеек, содержимое которых может начинаться на терм
	vector<CellSet> first_in_cells;
	// номера ячеек, содержимое которых может заканчиваться на терм
	vector<CellSet> last_in_cells;
	int lin_counter = 0;
	temp_copy.preorder_traversal(
		terms, lin_counter, in_lin_cells, first_in_cells, last_in_cells, {}, {}, {});
	// множество начальных состояний
	vector<pair<AlgExpression*, ToResetMap>> first = temp_copy.get_first_nodes_tracking_resets();
	// множество конечных состояний
	vector<BackRefRegex*> last = cast(temp_copy.get_last_nodes());
	// множество состояний, которым предшествует символ (ключ - линеаризованный номер)
	vector<vector<tuple<int, unordered_set<int>, CellSet>>> following_states(terms.size());
	temp_copy.get_follow(following_states);
	bool recognizes_eps = this->contains_eps();
	vector<MFAState> states; // состояния автомата

	string str_first, str_last, str_follow;
	for (const auto& [i, _] : first) {
		str_first += string(i->get_symbol()) + "\\ ";
	}

	set<string> last_set;
	for (auto& i : last) {
		last_set.insert(string(i->get_symbol()));
	}
	for (const auto& elem : last_set) {
		str_last += elem + "\\ ";
	}
	if (recognizes_eps)
		str_last += Symbol::Epsilon;

	for (int i = 0; i < following_states.size(); i++) {
		for (const auto& [to, _, __] : following_states[i]) {
			str_follow +=
				"(" + string(terms[i]->symbol) + "," + string(terms[to]->symbol) + ")" + "\\ ";
		}
	}

	vector<Symbol> delinearized_symbols;
	for (int i = 0; i < terms.size(); i++) {
		delinearized_symbols.push_back(terms[i]->symbol);
		delinearized_symbols[i].delinearize();
	}

	MFAState::Transitions initial_state_transitions;
	for (const auto& [i, to_reset_map] : first) {
		int to = i->get_symbol().last_linearization_number();
		const CellSet* destination_first = &first_in_cells[to];
		const unordered_set<int>* destination_in_lin_cells = &in_lin_cells[to];

		for (const auto& to_reset : merge_to_reset_maps({to_reset_map})) {
			initial_state_transitions
				[delinearized_symbols[i->get_symbol().last_linearization_number()]]
					.insert(MFATransition(to + 1,
										  MFATransition::TransitionConfig{destination_first,
																		  nullptr,
																		  nullptr,
																		  nullptr,
																		  destination_in_lin_cells,
																		  &to_reset}));
		}
	}
	states.emplace_back(0, "S", recognizes_eps, initial_state_transitions);

	unordered_set<int> last_terms;
	for (auto& i : last) {
		last_terms.insert(i->symbol.last_linearization_number());
	}

	for (size_t i = 0; i < terms.size(); i++) {
		Symbol& symb = terms[i]->symbol;
		MFAState::Transitions transitions;

		for (const auto& [to, iteration_over_cells, to_reset] :
			 following_states[symb.last_linearization_number()]) {
			transitions[delinearized_symbols[to]].insert(
				MFATransition(to + 1,
							  MFATransition::TransitionConfig{&first_in_cells[to],
															  &in_lin_cells[i],
															  &iteration_over_cells,
															  &last_in_cells[i],
															  &in_lin_cells[to],
															  &to_reset}));
		}

		// В last_terms номера конечных лексем => last_terms.count проверяет есть ли
		// номер лексемы в списке конечных лексем (является ли состояние конечным)
		states.emplace_back(
			i + 1, symb, last_terms.count(symb.last_linearization_number()), transitions);
	}

	MemoryFiniteAutomaton mfa(0, states, language);
	if (log) {
		log->set_parameter("brefregex", *this);
		log->set_parameter("linearised bregex", temp_copy);
		log->set_parameter("first", str_first);
		log->set_parameter("last", str_last);
		log->set_parameter("follow", str_follow);
		log->set_parameter("result", mfa);
	}
	return mfa;
}

void BackRefRegex::unfold_iterations(int& number) {
	switch (type) {
	case alt:
	case conc:
		cast(term_l)->unfold_iterations(number);
		cast(term_r)->unfold_iterations(number);
		return;
	case star:
		cast(term_l)->unfold_iterations(number);
		type = Type::conc;
		term_r = term_l->make_copy();
		return;
	case memoryWriter:
		lin_number = number++;
		cast(term_l)->unfold_iterations(number);
		return;
	default:
		break;
	}
}

bool BackRefRegex::_is_acreg(unordered_set<int> in_cells, unordered_set<int> in_lin_cells,
							 unordered_map<int, unordered_set<int>>& refs_in_cells) const {
	switch (type) {
	case alt: {
		auto refs_in_cells_copy = refs_in_cells;
		if (!cast(term_l)->_is_acreg(in_cells, in_lin_cells, refs_in_cells))
			return false;
		if (!cast(term_r)->_is_acreg(in_cells, in_lin_cells, refs_in_cells_copy))
			return false;
		for (const auto& [num, refs] : refs_in_cells_copy)
			refs_in_cells[num].insert(refs.begin(), refs.end());
		return true;
	}
	case conc:
		if (!cast(term_l)->_is_acreg(in_cells, in_lin_cells, refs_in_cells))
			return false;
		return cast(term_r)->_is_acreg(in_cells, in_lin_cells, refs_in_cells);
	case memoryWriter:
		in_cells.insert(cell_number);
		in_lin_cells.insert(lin_number);
		refs_in_cells[cell_number] = {lin_number};
		return cast(term_l)->_is_acreg(in_cells, in_lin_cells, refs_in_cells);
	case ref:
		if (auto refs_in_cell = refs_in_cells.find(cell_number);
			refs_in_cell != refs_in_cells.end()) {
			for (auto cell_lin_num : in_lin_cells)
				// если ссылается на те же линеаризованные memoryWriter, в которых находится сама
				if (refs_in_cell->second.count(cell_lin_num))
					return false;
			for (auto cell_num : in_cells)
				refs_in_cells[cell_num].insert(refs_in_cell->second.begin(),
											   refs_in_cell->second.end());
		}
		break;
	case symb:
	case eps:
		break;
	default:
		return false;
	}
	return true;
}

bool BackRefRegex::is_acreg(iLogTemplate* log) const {
	BackRefRegex temp(*this);

	int lin_counter = 0;
	temp.unfold_iterations(lin_counter);

	// ставит в соответствие номеру ячейки множество линеаризованных номеров ячеек,
	// на которые она ссылается
	unordered_map<int, unordered_set<int>> refs_in_cells;
	bool res = temp._is_acreg({}, {}, refs_in_cells);

	if (log) {
		log->set_parameter("brefregex", *this);
		log->set_parameter("result", res ? "True" : "False");
	}
	return res;
}

void BackRefRegex::linearize_refs(int& number) {
	switch (type) {
	case alt:
	case conc:
		cast(term_l)->linearize_refs(number);
		cast(term_r)->linearize_refs(number);
		break;
	case star:
	case memoryWriter:
		cast(term_l)->linearize_refs(number);
		break;
	case ref:
		symbol.linearize(number++);
		break;
	default:
		break;
	}
}

void BackRefRegex::_check_memory_writers(
	unordered_map<int, unordered_set<int>>& found_memory_writers,
	unordered_set<int>& refs_check_set, unordered_set<int>& memory_writers_check_set) const {
	switch (type) {
	case alt: {
		auto found_copy = found_memory_writers;
		cast(term_l)->_check_memory_writers(
			found_memory_writers, refs_check_set, memory_writers_check_set);
		cast(term_r)->_check_memory_writers(found_copy, refs_check_set, memory_writers_check_set);
		for (const auto& [memory_writer_cell_number, memory_writer_lin_numbers] : found_copy) {
			found_memory_writers[memory_writer_cell_number].insert(
				memory_writer_lin_numbers.begin(), memory_writer_lin_numbers.end());
		}
		break;
	}
	case conc:
		cast(term_l)->_check_memory_writers(
			found_memory_writers, refs_check_set, memory_writers_check_set);
		cast(term_r)->_check_memory_writers(
			found_memory_writers, refs_check_set, memory_writers_check_set);
		break;
	case memoryWriter:
		found_memory_writers[cell_number] = {lin_number};
		cast(term_l)->_check_memory_writers(
			found_memory_writers, refs_check_set, memory_writers_check_set);
		break;
	case ref:
		if (auto it = found_memory_writers.find(cell_number); it != found_memory_writers.end()) {
			refs_check_set.insert(symbol.last_linearization_number());
			for (const auto& memory_writer_lin_num : it->second)
				memory_writers_check_set.insert(memory_writer_lin_num);
		}
		break;
	default:
		break;
	}
}

bool BackRefRegex::check_refs_and_memory_writers_usefulness() const {
	BackRefRegex temp(*this);

	int refs_lin_counter = 0;
	temp.linearize_refs(refs_lin_counter);

	int memory_writers_lin_counter = 0;
	temp.unfold_iterations(memory_writers_lin_counter);

	unordered_map<int, unordered_set<int>> found_memory_writers;
	unordered_set<int> refs_check_set;
	unordered_set<int> memory_writers_check_set;
	temp._check_memory_writers(found_memory_writers, refs_check_set, memory_writers_check_set);

	return refs_check_set.size() == refs_lin_counter &&
		   memory_writers_check_set.size() == memory_writers_lin_counter;
}

void BackRefRegex::_reverse(unordered_map<int, BackRefRegex*>& memory_writers) {
	unordered_map<int, BackRefRegex*> memory_writers_copy;
	unordered_map<int, BackRefRegex*>::iterator it_ref_to;
	switch (type) {
	case alt:
		memory_writers_copy = memory_writers;
		cast(term_l)->_reverse(memory_writers);
		cast(term_r)->_reverse(memory_writers_copy);
		return;
	case conc:
		cast(term_l)->_reverse(memory_writers);
		cast(term_r)->_reverse(memory_writers);
		swap(term_l, term_r);
		return;
	case star:
		cast(term_l)->_reverse(memory_writers);
		return;
	case memoryWriter:
		memory_writers[cell_number] = this;
		cast(term_l)->_reverse(memory_writers);
		return;
	case ref:
		it_ref_to = memory_writers.find(cell_number);
		if (it_ref_to != memory_writers.end())
			ref_to = it_ref_to->second;
		return;
	default:
		return;
	}
}

void BackRefRegex::swap_memory_operations(unordered_set<BackRefRegex*>& already_swapped) {
	switch (type) {
	case conc:
	case alt:
		cast(term_l)->swap_memory_operations(already_swapped);
		cast(term_r)->swap_memory_operations(already_swapped);
		break;
	case star:
		cast(term_l)->swap_memory_operations(already_swapped);
		break;
	case memoryWriter:
		throw std::logic_error("swap_memory_operations: trying to swap memoryWriter");
	case ref:
		if (ref_to && !already_swapped.count(ref_to)) {
			already_swapped.insert(ref_to);
			swap(alphabet, ref_to->alphabet);
			swap(type, ref_to->type);
			swap(symbol, ref_to->symbol);
			swap(language, ref_to->language);
			swap(term_l, ref_to->term_l);
			cast(term_l)->swap_memory_operations(already_swapped);
		}
		break;
	default:
		break;
	}
}

BackRefRegex BackRefRegex::reverse(iLogTemplate* log) const {
	BackRefRegex temp(*this);

	unordered_map<int, BackRefRegex*> memory_writers;
	temp._reverse(memory_writers);
	unordered_set<BackRefRegex*> already_swapped;
	temp.swap_memory_operations(already_swapped);

	if (log) {
		log->set_parameter("oldbrefregex", *this);
		log->set_parameter("result", temp);
	}
	return temp;
}

BackRefRegex BackRefRegex::rewrite_aci() const {
	BackRefRegex res(*this);
	vector<AlgExpression*> alts;
	res._rewrite_aci(alts, false, true);
	return res;
}
