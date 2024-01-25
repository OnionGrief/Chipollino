#include "Objects/BackRefRegex.h"
#include "Objects/Language.h"
#include "Objects/MemoryFiniteAutomaton.h"
#include "Objects/Regex.h"

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
			throw std::runtime_error("BackRefRegex::from_string() ERROR");
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

bool BackRefRegex::equals(const AlgExpression* other) const {
	return cell_number == cast(other)->cell_number;
}

bool BackRefRegex::equal(const BackRefRegex& r1, const BackRefRegex& r2, iLogTemplate* log) {
	bool result = equality_checker(&r1, &r2);
	if (log) {
		//		log->set_parameter("regex1", r1);
		//		log->set_parameter("regex2", r2);
		//		log->set_parameter("result", result);
	}
	return result;
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

		states.emplace_back(left_states[0].is_terminal);
		for (const auto& [symbol, states_to] : left_states[0].transitions)
			for (const auto& transition : states_to)
				states[0].set_transition(transition, symbol);

		// переход по пустому слову в новое конечное состояние (случай нуля итераций)
		states[0].set_transition(MFATransition(left_states.size()), Symbol::Epsilon);

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

		// добавляем финальное состояние для пустого перехода
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
	}
	return mfa;
}

size_t PairHasher::operator()(const pair<int, int>& p) const {
	size_t h1 = std::hash<int>{}(p.first);
	size_t h2 = std::hash<int>{}(p.second);
	return h1 ^ (h2 << 1);
}

void BackRefRegex::preorder_traversal(
	vector<BackRefRegex*>& terms, int& lin_counter,
	std::vector<std::unordered_set<int>>& in_lin_cells,
	std::vector<std::unordered_set<pair<int, int>, PairHasher>>& first_in_cells,
	std::vector<std::unordered_set<pair<int, int>, PairHasher>>& last_in_cells,
	unordered_set<int> cur_in_lin_cells,
	unordered_set<pair<int, int>, PairHasher> cur_first_in_cells,
	unordered_set<pair<int, int>, PairHasher> cur_last_in_cells) {
	bool l_contains_eps, r_contains_eps;

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
	case conc:
		l_contains_eps = cast(term_l)->contains_eps();
		r_contains_eps = cast(term_r)->contains_eps();
		cast(term_l)->preorder_traversal(
			terms,
			lin_counter,
			in_lin_cells,
			first_in_cells,
			last_in_cells,
			cur_in_lin_cells,
			cur_first_in_cells,
			r_contains_eps ? cur_last_in_cells : unordered_set<pair<int, int>, PairHasher>());
		cast(term_r)->preorder_traversal(
			terms,
			lin_counter,
			in_lin_cells,
			first_in_cells,
			last_in_cells,
			cur_in_lin_cells,
			l_contains_eps ? cur_first_in_cells : unordered_set<pair<int, int>, PairHasher>(),
			cur_last_in_cells);
		return;
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
		cur_first_in_cells.insert({cell_number, lin_number});
		cur_last_in_cells.insert({cell_number, lin_number});
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

void BackRefRegex::calculate_may_be_eps(unordered_map<int, vector<BackRefRegex*>>& memory_writers) {
	unordered_map<int, vector<BackRefRegex*>> memory_writers_copy;
	unordered_map<int, vector<BackRefRegex*>>::iterator it_ref_to;
	switch (type) {
	case alt:
		memory_writers_copy = memory_writers;
		cast(term_l)->calculate_may_be_eps(memory_writers);
		cast(term_r)->calculate_may_be_eps(memory_writers_copy);
		for (const auto& [num, refs_to] : memory_writers_copy)
			for (const auto& memory_writer : refs_to)
				memory_writers[num].push_back(memory_writer);
		return;
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
	case ref:
		it_ref_to = memory_writers.find(cell_number);
		if (it_ref_to != memory_writers.end())
			for (const auto& memory_writer : it_ref_to->second)
				may_be_eps |= memory_writer->contains_eps();
		else
			may_be_eps = true;
		return;
	default:
		return;
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
		iteration_over_cells.insert(lin_number);
		cast(term_l)->get_cells_under_iteration(iteration_over_cells);
		return;
	default:
		return;
	}
}

bool BackRefRegex::contains_eps_tracking_resets(unordered_set<int>& to_reset) const {
	bool res;
	switch (type) {
	case Type::alt:
		return cast(term_l)->contains_eps_tracking_resets(to_reset) ||
			   cast(term_r)->contains_eps_tracking_resets(to_reset);
	case Type::conc:
		return cast(term_l)->contains_eps_tracking_resets(to_reset) &&
			   cast(term_r)->contains_eps_tracking_resets(to_reset);
	case Type::star:
	case Type::eps:
		return true;
	case Type::memoryWriter:
		res = cast(term_l)->contains_eps_tracking_resets(to_reset);
		if (res)
			to_reset.insert(cell_number);
		return res;
	case Type::ref:
		return may_be_eps;
	default:
		return false;
	}
}

std::vector<std::pair<AlgExpression*, std::unordered_set<int>>> BackRefRegex::
	get_first_nodes_tracking_resets() {
	vector<pair<AlgExpression*, unordered_set<int>>> l, r;
	unordered_set<int> to_reset;
	switch (type) {
	case Type::alt:
		l = cast(term_l)->get_first_nodes_tracking_resets();
		r = cast(term_r)->get_first_nodes_tracking_resets();
		l.insert(l.end(), r.begin(), r.end());
		return l;
	case Type::conc:
		l = cast(term_l)->get_first_nodes_tracking_resets();
		if (cast(term_l)->contains_eps_tracking_resets(to_reset)) {
			r = cast(term_r)->get_first_nodes_tracking_resets();
			for (auto& [i, prev_reset] : r) {
				prev_reset.insert(to_reset.begin(), to_reset.end());
				l.emplace_back(i, prev_reset);
			}
		}
		return l;
	case Type::star:
	case Type::memoryWriter:
		return cast(term_l)->get_first_nodes_tracking_resets();
	case AlgExpression::eps:
		return {};
	default:
		return {{this, {}}};
	}
}

std::vector<std::pair<AlgExpression*, std::unordered_set<int>>> BackRefRegex::
	get_last_nodes_tracking_resets() {
	vector<pair<AlgExpression*, unordered_set<int>>> l, r;
	unordered_set<int> to_reset;
	switch (type) {
	case Type::alt:
		l = cast(term_l)->get_last_nodes_tracking_resets();
		r = cast(term_r)->get_last_nodes_tracking_resets();
		l.insert(l.end(), r.begin(), r.end());
		return l;
	case Type::conc:
		r = cast(term_r)->get_last_nodes_tracking_resets();
		if (cast(term_r)->contains_eps_tracking_resets(to_reset)) {
			l = cast(term_l)->get_last_nodes_tracking_resets();
			for (auto& [i, prev_reset] : l) {
				prev_reset.insert(to_reset.begin(), to_reset.end());
				r.emplace_back(i, prev_reset);
			}
		}
		return r;
	case Type::star:
	case Type::memoryWriter:
		return cast(term_l)->get_last_nodes_tracking_resets();
	case AlgExpression::eps:
		return {};
	default:
		return {{this, {}}};
	}
}

unordered_set<int> merge_sets(const unordered_set<int>& set1, const unordered_set<int>& set2) {
	std::unordered_set<int> result = set1;
	result.insert(set2.begin(), set2.end());
	return result;
}

void BackRefRegex::get_follow(
	vector<vector<tuple<int, unordered_set<int>, unordered_set<int>>>>& following_states) const {
	vector<pair<AlgExpression*, unordered_set<int>>> first, last;
	unordered_set<int> iteration_over_cells;
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
				following_states[i->get_symbol().last_linearization_number()].emplace_back(
					j->get_symbol().last_linearization_number(),
					unordered_set<int>(),
					merge_sets(last_to_reset, first_to_reset));
			}
		}
		return;
	case Type::star:
		cast(term_l)->get_follow(following_states);
		last = cast(term_l)->get_last_nodes_tracking_resets();
		first = cast(term_l)->get_first_nodes_tracking_resets();
		get_cells_under_iteration(iteration_over_cells);
		for (auto& [i, last_to_reset] : last) {
			for (auto& [j, first_to_reset] : first) {
				following_states[i->get_symbol().last_linearization_number()].emplace_back(
					j->get_symbol().last_linearization_number(),
					iteration_over_cells,
					merge_sets(last_to_reset, first_to_reset));
			}
		}
		return;
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
	vector<unordered_set<pair<int, int>, PairHasher>> first_in_cells;
	// номера ячеек, содержимое которых может заканчиваться на терм
	vector<unordered_set<pair<int, int>, PairHasher>> last_in_cells;
	int lin_counter = 0;
	temp_copy.preorder_traversal(
		terms, lin_counter, in_lin_cells, first_in_cells, last_in_cells, {}, {}, {});
	// множество начальных состояний
	vector<BackRefRegex*> first = cast(temp_copy.get_first_nodes());
	// множество конечных состояний
	vector<BackRefRegex*> last = cast(temp_copy.get_last_nodes());
	// множество состояний, которым предшествует символ (ключ - линеаризованный номер)
	vector<vector<tuple<int, unordered_set<int>, unordered_set<int>>>> following_states(
		terms.size());
	temp_copy.get_follow(following_states);
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
		str_last += Symbol::Epsilon;
	}

	for (int i = 0; i < following_states.size(); i++) {
		for (const auto& [to, iteration_in_cell, to_reset] : following_states[i]) {
			str_follow +=
				"(" + string(terms[i]->symbol) + "," + string(terms[to]->symbol) + ")" + "\\ ";
		}
	}

	vector<Symbol> delinearized_symbols;
	for (int i = 0; i < terms.size(); i++) {
		delinearized_symbols.push_back(terms[i]->symbol);
		delinearized_symbols[i].delinearize();
	}

	MFAState::Transitions start_state_transitions;
	for (auto& i : first) {
		unordered_set<int> cells_to_open;
		for (auto [cell_num, lin_num] : first_in_cells[i->symbol.last_linearization_number()])
			cells_to_open.insert(cell_num);
		// можно не сбрасывать память, так как начальная конфигурация и так пустая
		start_state_transitions[delinearized_symbols[i->symbol.last_linearization_number()]].insert(
			MFATransition(i->symbol.last_linearization_number() + 1, cells_to_open, {}));
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

		for (const auto& [to, iteration_over_cells, to_reset] :
			 following_states[symb.last_linearization_number()]) {
			transitions[delinearized_symbols[to]].insert(MFATransition(to + 1,
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
	unordered_map<int, unordered_set<int>>::iterator refs_in_cell;
	unordered_map<int, unordered_set<int>> refs_in_cells_copy;
	switch (type) {
	case alt:
		refs_in_cells_copy = refs_in_cells;
		if (!cast(term_l)->_is_acreg(in_cells, in_lin_cells, refs_in_cells))
			return false;
		if (!cast(term_r)->_is_acreg(in_cells, in_lin_cells, refs_in_cells_copy))
			return false;
		for (const auto& [num, refs] : refs_in_cells_copy)
			refs_in_cells[num].insert(refs.begin(), refs.end());
		return true;
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
		refs_in_cell = refs_in_cells.find(cell_number);
		if (refs_in_cell != refs_in_cells.end()) {
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
	return temp._is_acreg({}, {}, refs_in_cells);
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

BackRefRegex BackRefRegex::reverse() {
	BackRefRegex temp(*this);

	unordered_map<int, BackRefRegex*> memory_writers;
	temp._reverse(memory_writers);
	unordered_set<BackRefRegex*> already_swapped;
	temp.swap_memory_operations(already_swapped);

	return temp;
}
