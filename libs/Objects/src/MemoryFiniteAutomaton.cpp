#include <algorithm>
#include <random>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_set>
#include <utility>

#include "Objects/FiniteAutomaton.h"
#include "Objects/Language.h"
#include "Objects/MemoryFiniteAutomaton.h"
#include "Objects/iLogTemplate.h"

using std::map;
using std::multiset;
using std::optional;
using std::pair;
using std::set;
using std::stack;
using std::string;
using std::stringstream;
using std::tuple;
using std::uniform_int_distribution;
using std::unordered_map;
using std::unordered_set;
using std::vector;

MFATransition::MFATransition(int to) : to(to) {}

MFATransition::MFATransition(int to, MemoryActions memory_actions)
	: to(to), memory_actions(std::move(memory_actions)) {}

MFATransition::MFATransition(int to, const unordered_set<int>& opens,
							 const unordered_set<int>& closes)
	: MFATransition(to) {
	for (auto cell_num : opens)
		memory_actions[cell_num] = MFATransition::open;
	for (auto cell_num : closes) {
		if (memory_actions.count(cell_num))
			std::cerr << "!!! Memory cell actions conflict !!!" << cell_num << " "
					  << memory_actions.at(cell_num);
		memory_actions[cell_num] = MFATransition::close;
	}
}

MFATransition::MFATransition(int to, const unordered_set<int>& opens,
							 const unordered_set<int>& closes, const unordered_set<int>& resets)
	: MFATransition(to) {
	for (auto cell_num : opens)
		memory_actions[cell_num] = MFATransition::open;
	for (auto cell_num : closes) {
		if (memory_actions.count(cell_num))
			std::cerr << "!!! Memory cell actions conflict !!!" << cell_num << " "
					  << memory_actions.at(cell_num);
		memory_actions[cell_num] = MFATransition::close;
	}
	for (auto cell_num : resets) {
		if (memory_actions.count(cell_num))
			std::cerr << "!!! Memory cell actions conflict !!!" << cell_num << " "
					  << memory_actions.at(cell_num);
		memory_actions[cell_num] = MFATransition::reset;
	}
}

MFATransition::MFATransition(int to, const TransitionConfig& config) : MFATransition(to) {
	if (config.source_last)
		for (auto [cell_num, lin_num] : *config.source_last) {
			if (config.destination_in_lin_cells->count(lin_num))
				continue;
			memory_actions[cell_num] = MFATransition::close;
		}
	if (config.to_reset)
		for (auto [cell_num, lin_num] : *config.to_reset) {
			if (config.destination_in_lin_cells->count(lin_num))
				continue;
			memory_actions[cell_num] = MFATransition::reset;
		}
	// при конфликте действий над ячейкой, открытие имеет приоритет
	if (config.destination_first)
		for (auto [cell_num, lin_num] : *config.destination_first) {
			if (config.source_in_lin_cells->count(lin_num) &&
				!config.iteration_over_cells->count(lin_num))
				continue;
			memory_actions[cell_num] = MFATransition::open;
		}
}

bool MFATransition::operator==(const MFATransition& other) const {
	return (to == other.to) && (memory_actions == other.memory_actions);
}

std::size_t MFATransition::Hasher::operator()(const MFATransition& t) const {
	std::size_t result = std::hash<int>{}(t.to);
	for (const auto& pair : t.memory_actions) {
		result ^= std::hash<int>{}(pair.first) + std::hash<int>{}(static_cast<int>(pair.second));
	}
	return result;
}

MFAState::MFAState(bool is_terminal) : State::State(0, {}, is_terminal) {}

MFAState::MFAState(int index, string identifier, bool is_terminal)
	: State::State(index, std::move(identifier), is_terminal) {}

MFAState::MFAState(int index, string identifier, bool is_terminal,
				   MFAState::Transitions transitions)
	: State::State(index, std::move(identifier), is_terminal), transitions(std::move(transitions)) {
}

MFAState::MFAState(const FAState& state)
	: State::State(state.index, state.identifier, state.is_terminal) {
	for (const auto& [symbol, states_to] : state.transitions)
		for (auto to : states_to)
			transitions[symbol].insert(MFATransition(to));
}

void MFAState::add_transition(const MFATransition& tr, const Symbol& symbol) {
	transitions[symbol].insert(tr);
}

string MFAState::to_txt() const {
	return {};
}

bool MFAState::operator==(const MFAState& other) const {
	return State::operator==(other) && transitions == other.transitions;
}

string MFATransition::get_actions_str() const {
	stringstream ss;
	unordered_set<int> opens;
	unordered_set<int> closes;
	unordered_set<int> resets;

	for (const auto& [num, action] : memory_actions) {
		switch (action) {
		case MFATransition::open:
			opens.insert(num);
			break;
		case MFATransition::close:
			closes.insert(num);
			break;
		case MFATransition::reset:
			resets.insert(num);
			break;
		}
	}

	size_t count = 0;
	char memory_actions_separator = ';';
	if (!opens.empty()) {
		ss << memory_actions_separator << " o: ";
		count = 0;
		for (int num : opens) {
			ss << num;
			if (++count < opens.size()) {
				ss << ", ";
			}
		}
	}

	if (!closes.empty()) {
		ss << memory_actions_separator << " c: ";
		count = 0;
		for (int num : closes) {
			ss << num;
			if (++count < closes.size()) {
				ss << ", ";
			}
		}
	}

	if (!resets.empty()) {
		ss << memory_actions_separator << " r: ";
		count = 0;
		for (int num : resets) {
			ss << num;
			if (++count < resets.size()) {
				ss << ", ";
			}
		}
	}

	return ss.str();
}

MemoryFiniteAutomaton::MemoryFiniteAutomaton() : AbstractMachine() {}

MemoryFiniteAutomaton::MemoryFiniteAutomaton(int initial_state, vector<MFAState> states,
											 std::shared_ptr<Language> language)
	: AbstractMachine(initial_state, std::move(language)), states(std::move(states)) {
	for (int i = 0; i < this->states.size(); i++) {
		if (this->states[i].index != i)
			throw std::logic_error(
				"State.index must correspond to its ordinal number in the states vector");
	}
}

MemoryFiniteAutomaton::MemoryFiniteAutomaton(int initial_state, std::vector<MFAState> states,
											 Alphabet alphabet)
	: AbstractMachine(initial_state, std::move(alphabet)), states(std::move(states)) {
	for (int i = 0; i < this->states.size(); i++) {
		if (this->states[i].index != i)
			throw std::logic_error(
				"State.index must correspond to its ordinal number in the states vector");
	}
}

template <typename T>
MemoryFiniteAutomaton* MemoryFiniteAutomaton::cast(std::unique_ptr<T>&& uptr) {
	auto* mfa = dynamic_cast<MemoryFiniteAutomaton*>(uptr.get());
	if (!mfa) {
		throw std::runtime_error("Failed to cast to MemoryFiniteAutomaton");
	}

	return mfa;
}

string MemoryFiniteAutomaton::to_txt() const {
	stringstream ss;
	ss << "digraph {\n\trankdir = LR\n\tdummy [label = \"\", shape = none]\n\t";
	for (int i = 0; i < states.size(); i++) {
		ss << i << " [label = \"" << states[i].identifier << "\", shape = ";
		ss << (states[i].is_terminal ? "doublecircle]\n\t" : "circle]\n\t");
	}
	if (states.size() > initial_state)
		ss << "dummy -> " << states[initial_state].index << "\n";

	for (const auto& state : states) {
		for (const auto& [symbol, symbol_transitions] : state.transitions) {
			for (const auto& transition : symbol_transitions) {
				ss << "\t" << state.index << " -> " << transition.to << " [label = \""
				   << string(symbol) << transition.get_actions_str() << "\"]\n";
			}
		}
	}

	ss << "}\n";
	return ss.str();
}

size_t MemoryFiniteAutomaton::size(iLogTemplate* log) const {
	return states.size();
}

std::vector<MFAState> MemoryFiniteAutomaton::get_states() const {
	return states;
}

bool MemoryFiniteAutomaton::is_deterministic(iLogTemplate* log) const {
	if (log) {
		//		log->set_parameter("oldautomaton", *this);
	}
	bool result = true;
	for (const auto& state : states) {
		for (const auto& [symbol, symbol_transitions] : state.transitions) {
			if ((symbol.is_epsilon() || symbol.is_ref()) && state.transitions.size() > 1) {
				result = false;
				break;
			}
			if (symbol_transitions.size() > 1) {
				result = false;
				break;
			}
		}
	}
	if (log) {
		log->set_parameter("result", result ? "True" : "False");
	}
	return result;
}

MemoryFiniteAutomaton MemoryFiniteAutomaton::add_trap_state(iLogTemplate* log) const {
	if (!is_deterministic())
		throw std::logic_error("add_trap_state: mfa must be deterministic");

	vector<MFAState> new_states = states;
	bool add_trap = false;
	MetaInfo new_meta;
	int count = static_cast<int>(size());
	for (auto& state : new_states) {
		for (const Symbol& symb : language->get_alphabet()) {
			if (state.transitions.size() == 1 && (state.transitions.begin()->first.is_epsilon() ||
												  state.transitions.begin()->first.is_ref()))
				continue;
			if (!state.transitions.count(symb)) {
				state.add_transition(MFATransition(count), symb);
				new_meta.upd(EdgeMeta{state.index, count, symb, MetaInfo::trap_color});
				add_trap = true;
			}
		}
	}

	if (add_trap) {
		new_states.emplace_back(count, "", false);
		for (const Symbol& symb : language->get_alphabet()) {
			new_states[count].add_transition(MFATransition(count), symb);
			new_meta.upd(EdgeMeta{count, count, symb, MetaInfo::trap_color});
		}
	}

	MemoryFiniteAutomaton new_mfa(initial_state, new_states, language);
	if (log) {
		log->set_parameter("oldmfa", *this);
		log->set_parameter("result", new_mfa);
	}
	return new_mfa;
}

MemoryFiniteAutomaton MemoryFiniteAutomaton::get_just_one_total_trap(
	const std::shared_ptr<Language>& language) {
	vector<MFAState> states;
	states.emplace_back(0, "", false);
	for (const Symbol& symb : language->get_alphabet()) {
		states[0].add_transition(MFATransition(0), symb);
	}

	return {0, states, language};
}

void compose_actions(MFATransition::MemoryActions& composition, // NOLINT(runtime/references)
					 int cell_num, MFATransition::MemoryAction action) {
	auto cell_action = composition.find(cell_num);
	if (cell_action != composition.end()) {
		if (action == MFATransition::close) {
			switch (cell_action->second) {
			case MFATransition::open:
				composition[cell_num] = MFATransition::reset;
				break;
			case MFATransition::close:
				composition[cell_num] = MFATransition::close;
				break;
			case MFATransition::reset:
				break;
			}
		} else if (action == MFATransition::open) {
			switch (cell_action->second) {
			case MFATransition::open:
				break;
			case MFATransition::close:
			case MFATransition::reset:
				composition[cell_num] = MFATransition::open;
				break;
			}
		} else {
			composition[cell_num] = action;
		}
	} else {
		composition[cell_num] = action;
	}
}

void MemoryFiniteAutomaton::dfs_by_eps(
	int state_index, set<int>& reachable, const int& first, int& last,
	MFATransition::MemoryActions& memory_actions_composition) const {
	if (!reachable.count(state_index)) {
		reachable.insert(state_index);
		last = state_index;
		const auto& by_eps = states[state_index].transitions.find(Symbol::Epsilon);
		if (by_eps != states[state_index].transitions.end()) {
			if (states[state_index].transitions.size() > 1 && state_index != first)
				throw std::logic_error(
					"dfs_by_eps: eps-transitions have parallel symb-transitions");
			if (by_eps->second.size() > 1)
				throw std::logic_error(
					"dfs_by_eps: trying to make a composition of parallel eps-transitions");
			for (const auto& transition : by_eps->second) {
				for (auto [num, action] : transition.memory_actions) {
					compose_actions(memory_actions_composition, num, action);
				}
				dfs_by_eps(transition.to, reachable, first, last, memory_actions_composition);
			}
		}
	}
}

tuple<set<int>, unordered_set<int>, MFATransition::MemoryActions> MemoryFiniteAutomaton::
	get_eps_closure(const set<int>& indices) const {
	// оставил set, потому что важен порядок для идентификаторов состояний (тесты)
	set<int> reachable;
	unordered_set<int> last;
	MFATransition::MemoryActions memory_actions_composition;
	for (int index : indices) {
		int _last = index; // последнее состояние в цепочке eps-переходов
		dfs_by_eps(index, reachable, index, _last, memory_actions_composition);
		last.insert(_last);
	}
	return {reachable, last, memory_actions_composition};
}

MemoryFiniteAutomaton MemoryFiniteAutomaton::remove_eps(iLogTemplate* log) const {
	auto get_identifier = [](set<int>& s) { // NOLINT(runtime/references)
		stringstream ss;
		bool is_first = true;
		for (const auto& element : s) {
			if (!is_first) {
				ss << ", ";
			}
			ss << element;
			is_first = false;
		}
		return ss.str();
	};

	struct Hasher {
		std::size_t operator()(const std::set<int>& s) const {
			std::size_t seed = s.size();
			for (const int& i : s) {
				seed ^= std::hash<int>()(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}
			return seed;
		}
	};

	unordered_set<Symbol, Symbol::Hasher> unique_symbols;
	for (const auto& state : states)
		for (const auto& [symbol, symbol_transitions] : state.transitions)
			if (!symbol.is_epsilon())
				unique_symbols.insert(symbol);

	vector<MFAState> new_states;
	unordered_map<set<int>, int, Hasher> state_number_by_closure;
	auto [initial_closure, initial_closure_last, initial_memory_actions] =
		get_eps_closure({initial_state});
	state_number_by_closure[initial_closure] = 0;
	new_states.emplace_back(0, get_identifier(initial_closure), false);

	stack<tuple<set<int>, unordered_set<int>, MFATransition::MemoryActions>> s;
	s.emplace(initial_closure, initial_closure_last, initial_memory_actions);
	int states_counter = 1;
	while (!s.empty()) {
		auto [from_closure, from_closure_last, prev_memory_actions] = s.top();
		s.pop();
		for (const auto& symb : unique_symbols) {
			optional<MFATransition> transition;
			int from_state;
			for (int i : from_closure) {
				auto symbol_transitions = states[i].transitions.find(symb);
				if (symbol_transitions != states[i].transitions.end()) {
					if (symbol_transitions->second.size() > 1 || transition.has_value())
						throw std::logic_error("remove_eps: trying to make a composition of "
											   "parallel transitions by symb");
					if (symbol_transitions->second.size() == 1) {
						from_state = i;
						transition = *symbol_transitions->second.begin();
					}
				}
			}
			if (transition.has_value()) {
				auto [closure, closure_last, closure_memory_actions] =
					get_eps_closure({transition->to});
				if (!closure.empty()) {
					if (!state_number_by_closure.count(closure)) {
						MFAState new_state(states_counter, get_identifier(closure), false);
						new_states.push_back(new_state);
						state_number_by_closure[closure] = states_counter;
						s.emplace(closure, closure_last, closure_memory_actions);
						states_counter++;
					}
					if (from_closure_last.count(from_state) &&
						!from_closure.count(transition->to)) {
						auto temp_memory_actions = prev_memory_actions;
						for (auto [num, action] : transition->memory_actions)
							compose_actions(temp_memory_actions, num, action);
						new_states[state_number_by_closure[from_closure]].add_transition(
							MFATransition(state_number_by_closure[closure], temp_memory_actions),
							symb);
					} else {
						new_states[state_number_by_closure[from_closure]].add_transition(
							MFATransition(state_number_by_closure[closure],
										  transition->memory_actions),
							symb);
					}
				}
			}
		}
	}

	for (const auto& [formed_by_states, num] : state_number_by_closure) {
		for (int i : formed_by_states) {
			if (states[i].is_terminal)
				new_states[num].is_terminal = true;
		}
	}

	MemoryFiniteAutomaton res = {0, new_states, language};
	if (log) {
		log->set_parameter("oldmfa", *this);
		log->set_parameter("result", res);
	}
	return res;
}

MemoryFiniteAutomaton MemoryFiniteAutomaton::complement(iLogTemplate* log) const {
	MemoryFiniteAutomaton new_mfa(initial_state, states, language->get_alphabet());
	new_mfa = new_mfa.add_trap_state();
	int final_states_counter = 0;
	for (int i = 0; i < new_mfa.size(); i++) {
		new_mfa.states[i].is_terminal = !new_mfa.states[i].is_terminal;
		if (new_mfa.states[i].is_terminal)
			final_states_counter++;
	}
	if (!final_states_counter)
		new_mfa = MemoryFiniteAutomaton::get_just_one_total_trap(new_mfa.language);

	if (log) {
		log->set_parameter("oldmfa", *this);
		log->set_parameter("result", new_mfa);
	}
	return new_mfa;
}

ParingState::ParingState(int pos, const MFAState* state, const MemoryConfiguration& opened_cells,
						 const MemoryContents& memory)
	: pos(pos), state(state), opened_cells(opened_cells), memory(memory) {}

bool ParingState::operator==(const ParingState& other) const {
	return pos == other.pos && state == other.state && opened_cells == other.opened_cells &&
		   memory == other.memory;
}

Matcher::Matcher(const string& s) : s(&s) {}

void ParseTransition::do_memory_actions(int pos) {
	for (const auto [num, action] : memory_actions) {
		if (action == MFATransition::MemoryAction::open) {
			opened_cells.insert(num);
			memory[num].first = pos;
			memory[num].second = pos;
		} else if (action == MFATransition::MemoryAction::close) {
			if (!opened_cells.count(num))
				std::cerr << "ParseTransition::do_memory_actions: cell is already closed\n";
			opened_cells.erase(num);
		} else if (action == MFATransition::MemoryAction::reset) {
			// ячейка может сбрасываться, даже если она не открыта
			memory[num].first = pos;
			memory[num].second = pos;
			opened_cells.erase(num);
		}
	}
}

// для символа-буквы - 1, для символа-ссылки - длина содержимого памяти
int get_symbol_len(const MemoryContents& memory, const Symbol& symbol) {
	if (symbol.is_ref()) {
		pair<int, int> substr = memory.at(symbol.get_ref());
		return substr.second - substr.first;
	} else if (!symbol.is_epsilon()) {
		return 1;
	}
	return 0;
}

void ParseTransition::update_memory_contents(const Symbol& symbol) {
	for (auto num : opened_cells) {
		memory[num].second += get_symbol_len(memory, symbol);
	}
}

ParseTransition::ParseTransition(const MFATransition& transition, const ParingState& parsing_state)
	: MFATransition(transition), opened_cells(parsing_state.opened_cells),
	  memory(parsing_state.memory) {
	do_memory_actions(parsing_state.pos);
}

ParseTransition::ParseTransition(const MFATransition& transition,
								 const TraversalState& traversal_state)
	: MFATransition(transition), opened_cells(traversal_state.opened_cells),
	  memory(traversal_state.memory) {
	do_memory_actions(traversal_state.str.size());
}

void ParseTransitions::add_transitions(const Symbol& symbol,
									   const MFAState::SymbolTransitions& symbol_transitions,
									   const ParingState& parsing_state) {
	for (const auto& tr : symbol_transitions)
		transitions[symbol].insert(ParseTransition(tr, parsing_state));
}

void ParseTransitions::add_transitions(const Symbol& symbol,
									   const MFAState::SymbolTransitions& symbol_transitions,
									   const TraversalState& traversal_state) {
	for (const auto& tr : symbol_transitions)
		transitions[symbol].insert(ParseTransition(tr, traversal_state));
}

unordered_map<Symbol, unordered_set<ParseTransition, MFATransition::Hasher>,
			  Symbol::Hasher>::const_iterator
ParseTransitions::begin() const {
	return transitions.begin();
}

unordered_map<Symbol, unordered_set<ParseTransition, MFATransition::Hasher>,
			  Symbol::Hasher>::const_iterator
ParseTransitions::end() const {
	return transitions.end();
}

// пара {переходы по символам-буквам/непустым символам-ссылкам, переходы по эпсилон/пустым ссылкам}
pair<ParseTransitions, ParseTransitions> get_transitions(const string& s,
														 const ParingState& parsing_state,
														 Matcher* matcher) {
	ParseTransitions transitions, eps_transitions;
	// тройки {символ-ссылка, начало подстроки, конец подстроки}
	vector<tuple<Symbol, MFAState::SymbolTransitions, int, int>> refs_to_match;
	for (const auto& [symbol, symbol_transitions] : parsing_state.state->transitions) {
		if (symbol.is_ref()) {
			if (!parsing_state.memory.count(symbol.get_ref())) {
				// пустая ссылка добавляется к eps-переходам
				eps_transitions.add_transitions(symbol, symbol_transitions, parsing_state);
				continue;
			}
			const auto& [l, r] = parsing_state.memory.at(symbol.get_ref());
			if (l == r) {
				// пустая ссылка добавляется к eps-переходам
				eps_transitions.add_transitions(symbol, symbol_transitions, parsing_state);
				continue;
			}
			MFAState::SymbolTransitions non_empty_ref_transitions, empty_ref_transitions;
			for (const auto& tr : symbol_transitions) {
				auto ref_cell_memory_action = tr.memory_actions.find(symbol.get_ref());
				if (ref_cell_memory_action != tr.memory_actions.end() &&
					ref_cell_memory_action->second == MFATransition::reset) {
					empty_ref_transitions.insert(tr);
					continue;
				}
				non_empty_ref_transitions.insert(tr);
			}
			if (!empty_ref_transitions.empty()) {
				eps_transitions.add_transitions(symbol, empty_ref_transitions, parsing_state);
			}
			if (r - l <= s.size() - parsing_state.pos) {
				refs_to_match.emplace_back(symbol, non_empty_ref_transitions, l, r);
			}
		} else if (symbol == s[parsing_state.pos]) {
			transitions.add_transitions(symbol, symbol_transitions, parsing_state);
		} else if (symbol.is_epsilon()) {
			eps_transitions.add_transitions(symbol, symbol_transitions, parsing_state);
		}
	}

	if (!refs_to_match.empty())
		matcher->match(parsing_state, transitions, refs_to_match);

	return {transitions, eps_transitions};
}

pair<int, bool> MemoryFiniteAutomaton::_parse_slow(const string& s, Matcher* matcher) const {
	stack<ParingState> parsing_states_stack;
	// тройка (актуальный индекс элемента в строке, начало эпсилон-перехода, конец эпсилон-перехода)
	set<tuple<int, int, int>> visited_eps;
	int counter = 0;
	int parsed_len = 0;
	const MFAState* state = &states[initial_state];
	parsing_states_stack.emplace(parsed_len, state, MemoryConfiguration(), MemoryContents());
	while (!parsing_states_stack.empty()) {
		if (state->is_terminal && parsed_len == s.size()) {
			break;
		}
		counter++;
		ParingState parsing_state = parsing_states_stack.top();
		state = parsing_state.state;
		parsed_len = parsing_state.pos;
		parsing_states_stack.pop();
		// состояния достижимые по символам-буквам/символам-ссылкам
		// и состояния достижимые по эпсилон-переходам/пустым ссылкам
		auto [reach, reach_eps] = get_transitions(s, parsing_state, matcher);

		// переходы в новые состояния по букве/непустой ссылке
		for (const auto& [symbol, symbol_transitions] : reach) {
			for (auto tr : symbol_transitions) {
				if (parsed_len + get_symbol_len(tr.memory, symbol) > s.size())
					continue;
				tr.update_memory_contents(symbol);
				parsing_states_stack.emplace(parsed_len + get_symbol_len(tr.memory, symbol),
											 &states[tr.to],
											 tr.opened_cells,
											 tr.memory);
			}
		}

		// если произошёл откат по строке, то эпсилон-переходы из рассмотренных состояний больше не
		// считаются повторными
		if (!visited_eps.empty()) {
			set<tuple<int, int, int>> temp_eps;
			for (auto pos : visited_eps) {
				if (std::get<0>(pos) <= parsed_len)
					temp_eps.insert(pos);
			}
			visited_eps = temp_eps;
		}

		// добавление тех эпсилон-переходов, по которым ещё не было разбора от этой позиции и этого
		// состояния
		for (const auto& [_, symb_transitions] : reach_eps) {
			for (auto eps_tr : symb_transitions) {
				if (!visited_eps.count({parsed_len, state->index, eps_tr.to})) {
					eps_tr.update_memory_contents(Symbol::Epsilon);
					parsing_states_stack.emplace(
						parsed_len, &states[eps_tr.to], eps_tr.opened_cells, eps_tr.memory);
					visited_eps.insert({parsed_len, state->index, eps_tr.to});
				}
			}
		}
	}

	if (s.size() == parsed_len && state->is_terminal) {
		return {counter, true};
	}

	return {counter, false};
}

size_t ParingState::Hasher::operator()(const ParingState& s) const {
	std::size_t seed = 0;

	hash_combine(seed, s.pos);
	hash_combine(seed, s.state);

	for (const auto& cell : s.opened_cells) {
		hash_combine(seed, cell);
	}

	for (const auto& entry : s.memory) {
		hash_combine(seed, entry.first);
		hash_combine(seed, entry.second.first);
		hash_combine(seed, entry.second.second);
	}

	return seed;
}

pair<int, bool> MemoryFiniteAutomaton::_parse(const string& s, Matcher* matcher) const {
	unordered_set<ParingState, ParingState::Hasher> current_states;
	current_states.insert(
		ParingState(0, &states[initial_state], MemoryConfiguration(), MemoryContents()));

	unordered_set<ParingState, ParingState::Hasher> visited_states;

	int counter = 0;
	while (!current_states.empty()) {
		unordered_set<ParingState, ParingState::Hasher> following_states;
		for (const auto& cur_state : current_states) {
			if (visited_states.count(cur_state))
				continue;

			const MFAState* state = cur_state.state;
			int parsed_len = cur_state.pos;
			if (state->is_terminal && parsed_len == s.size()) {
				return {counter, true};
			}

			// состояния достижимые по символам-буквам/символам-ссылкам
			// и состояния достижимые по эпсилон-переходам/пустым ссылкам
			auto [reach, reach_eps] = get_transitions(s, cur_state, matcher);

			// переходы в новые состояния по букве/непустой ссылке
			for (const auto& [symbol, symbol_transitions] : reach) {
				for (auto tr : symbol_transitions) {
					if (parsed_len + get_symbol_len(tr.memory, symbol) > s.size())
						continue;
					tr.update_memory_contents(symbol);
					following_states.emplace(parsed_len + get_symbol_len(tr.memory, symbol),
											 &states[tr.to],
											 tr.opened_cells,
											 tr.memory);
				}
			}

			// эпсилон-переходы
			for (const auto& [_, symb_transitions] : reach_eps) {
				for (auto eps_tr : symb_transitions) {
					eps_tr.update_memory_contents(Symbol::Epsilon);
					following_states.emplace(
						parsed_len, &states[eps_tr.to], eps_tr.opened_cells, eps_tr.memory);
				}
			}

			visited_states.insert(cur_state);
		}
		current_states = following_states;
		counter++;
	}

	return {counter, false};
}

class BasicMatcher : public Matcher {
  public:
	explicit BasicMatcher(const string&);

	// сопоставление за линию для каждой ячейки
	void match(const ParingState&, ParseTransitions&, // NOLINT(runtime/references)
			   vector<tuple<Symbol, MFAState::SymbolTransitions, int, int>>&)
		override; // NOLINT(runtime/references)
};

BasicMatcher::BasicMatcher(const string& s) : Matcher(s) {}

void BasicMatcher::match(
	const ParingState& parsing_state, ParseTransitions& transitions,
	vector<tuple<Symbol, MFAState::SymbolTransitions, int, int>>& refs_to_match) {
	for (int i = parsing_state.pos; i <= s->size(); i++) {
		if (refs_to_match.empty())
			break;
		for (auto it = refs_to_match.begin(); it != refs_to_match.end();) {
			const auto& [symbol, symbol_transitions, l, r] = *it;
			if (l == parsing_state.pos)
				std::cerr << "Trying to match ref to current position\n";
			int index = l + i - parsing_state.pos;
			if (index == r) {
				transitions.add_transitions(symbol, symbol_transitions, parsing_state);
				it = refs_to_match.erase(it);
			} else if ((*s)[index] != (*s)[i]) {
				it = refs_to_match.erase(it);
			} else {
				++it;
			}
		}
	}
}

pair<int, bool> MemoryFiniteAutomaton::parse(const string& s) const {
	BasicMatcher matcher(s);
	return _parse(s, &matcher);
}

class FastMatcher : public Matcher {
  private:
	vector<vector<int>> sparse_table;
	vector<int> inv_sa;

	int query_sparse_table(int l, int r);

  public:
	explicit FastMatcher(const string&);

	// сопоставление за константу для каждой ячейки
	void match(const ParingState&, ParseTransitions&, // NOLINT(runtime/references)
			   vector<tuple<Symbol, MFAState::SymbolTransitions, int, int>>&)
		override; // NOLINT(runtime/references)
};

void counting_sort(vector<int>& p, const vector<int>& c) { // NOLINT(runtime/references)
	int n = p.size();
	vector<int> count(n, 0);
	vector<int> p_new(n, 0);

	for (int i = 0; i < n; i++) {
		count[c[p[i]]]++;
	}

	for (int i = 1; i < n; i++) {
		count[i] += count[i - 1];
	}

	for (int i = n - 1; i >= 0; i--) {
		p_new[--count[c[p[i]]]] = p[i];
	}

	p = p_new;
}

FastMatcher::FastMatcher(const string& s) : Matcher(s), inv_sa(s.size() + 1) {
	// построение суффиксного массива
	string temp_s = s + "$";
	int n = temp_s.size();
	vector<int> p(n, 0);
	vector<int> c(n, 0);

	// сортируем символы и строим начальные массивы p и c
	vector<pair<char, int>> a(n);
	for (int i = 0; i < n; i++) {
		a[i] = {temp_s[i], i};
	}
	sort(a.begin(), a.end());

	for (int i = 0; i < n; i++) {
		p[i] = a[i].second;
	}
	c[p[0]] = 0;
	for (int i = 1; i < n; i++) {
		if (a[i].first == a[i - 1].first) {
			c[p[i]] = c[p[i - 1]];
		} else {
			c[p[i]] = c[p[i - 1]] + 1;
		}
	}

	int k = 0;
	while ((1 << k) < n) {
		for (int i = 0; i < n; i++) {
			p[i] = (p[i] - (1 << k) + n) % n;
		}
		counting_sort(p, c);

		vector<int> c_new(n, 0);
		c_new[p[0]] = 0;
		for (int i = 1; i < n; i++) {
			pair<int, int> prev = {c[p[i - 1]], c[(p[i - 1] + (1 << k)) % n]};
			pair<int, int> cur = {c[p[i]], c[(p[i] + (1 << k)) % n]};
			if (prev == cur) {
				c_new[p[i]] = c_new[p[i - 1]];
			} else {
				c_new[p[i]] = c_new[p[i - 1]] + 1;
			}
		}
		c = c_new;

		k++;
	}

	// построение LCP
	vector<int> lcp(n);

	for (int i = 0; i < n; i++) {
		inv_sa[p[i]] = i;
	}

	int len = 0;
	for (int i = 0; i < n; i++) {
		if (inv_sa[i] == n - 1) {
			len = 0;
			continue;
		}

		int j = p[inv_sa[i] + 1];
		while (i + len < n && j + len < n && temp_s[i + len] == temp_s[j + len]) {
			len++;
		}

		lcp[inv_sa[i]] = len;
		if (len > 0)
			len--;
	}

	// построение sparse table
	int logN = log2(n) + 1;
	sparse_table.resize(n, vector<int>(logN));

	for (int i = 0; i < n; i++) {
		sparse_table[i][0] = lcp[i];
	}

	for (int j = 1; j < logN; j++) {
		for (int i = 0; i + (1 << j) <= n; i++) {
			sparse_table[i][j] =
				std::min(sparse_table[i][j - 1], sparse_table[i + (1 << (j - 1))][j - 1]);
		}
	}
}

int FastMatcher::query_sparse_table(int l, int r) {
	int k = log2(r - l + 1);
	return std::min(sparse_table[l][k], sparse_table[r - (1 << k) + 1][k]);
}

void FastMatcher::match(
	const ParingState& parsing_state, ParseTransitions& transitions,
	vector<tuple<Symbol, MFAState::SymbolTransitions, int, int>>& refs_to_match) {
	for (const auto& [symbol, symbol_transitions, l, r] : refs_to_match) {
		if (l == parsing_state.pos)
			std::cerr << "Trying to match ref to current position\n";
		if (query_sparse_table(std::min(inv_sa[l], inv_sa[parsing_state.pos]),
							   std::max(inv_sa[l], inv_sa[parsing_state.pos]) - 1) >= r - l)
			transitions.add_transitions(symbol, symbol_transitions, parsing_state);
	}
}

pair<int, bool> MemoryFiniteAutomaton::parse_additional(const string& s) const {
	FastMatcher matcher(s);
	return _parse(s, &matcher);
}

bool MemoryFiniteAutomaton::check_memory_correctness() {
	// Ячейки, которые могут быть открыты при попадании в состояние
	map<int, set<int>> open_cells;
	// Ячейки, переходы из которых надо проверить
	set<int> states_to_check;
	for (const auto& state : get_states()) {
		states_to_check.insert(state.index);
	}

	while (!states_to_check.empty()) {
		int cur_state = *states_to_check.begin();
		states_to_check.erase(cur_state);
		auto transitions = get_states()[cur_state].transitions;
		for (const auto& symbol_transitions : transitions) {
			for (const auto& transition : symbol_transitions.second) {
				for (auto cell : open_cells[cur_state]) {
					// Если в текущем состоянии ячейка открыта для записи и переход её не закрывает
					bool pass = !transition.memory_actions.count(cell);
					// Если раньше в open_cells для состояния transition.to ячейка была закрыта, то
					// это состояние надо проверить
					if (!open_cells[transition.to].count(cell) && pass) {
						open_cells[transition.to].insert(cell);
						states_to_check.insert(transition.to);
					}
				}
				for (auto action : transition.memory_actions) {
					// Если переход открывает ячейку
					bool open = (action.second == MFATransition::open);
					// Если раньше в open_cells для состояния transition.to ячейка была закрыта, то
					// это состояние надо проверить
					if (!open_cells[transition.to].count(action.first) && open) {
						open_cells[transition.to].insert(action.first);
						states_to_check.insert(transition.to);
					}
				}
			}
		}
	}

	for (const auto& state : get_states()) {
		auto transitions = get_states()[state.index].transitions;
		for (const auto& symbol_transitions : transitions) {
			// Ячейка может быть открыта и по ней существует переход
			if (symbol_transitions.first.is_ref() &&
				open_cells[state.index].count(symbol_transitions.first.get_ref())) {
				return false;
				// throw runtime_error("MFA ERROR(Reading from the open cell)");
			}
			for (const auto& transition : symbol_transitions.second) {
				for (auto action : transition.memory_actions) {
					if (action.second == MFATransition::open) {
						if (symbol_transitions.first.is_ref() &&
							symbol_transitions.first.get_ref() == action.first)
							// Чтение из открывающейся для записи ячейки
							return false;
						if (open_cells[state.index].count(action.first)) {
							return false;
							// throw runtime_error("MFA ERROR(Openning the open cell)");
						}
					}
				}
			}
		}
	}

	return true;
}
string random_mutation(const string& word, int l, int r, const Alphabet& alphabet) {
	string mutated_word = word.substr(l, r - l);

	std::random_device rd;
	std::mt19937 gen(rd());
	uniform_int_distribution<int> distribution(0, 1);
	int mutation_type = distribution(gen);
	if (mutation_type == 0 || mutated_word.size() == 1) {
		// вставка
		int insertionPoint = uniform_int_distribution<int>(0, mutated_word.size() - 1)(gen);
		int fragmentLength = uniform_int_distribution<int>(0, (mutated_word.size() + 1) / 2)(gen);
		string randomFragment;
		for (int i = 0; i < fragmentLength; ++i) {
			Symbol random_symb = *next(begin(alphabet), gen() % alphabet.size());
			randomFragment += random_symb;
		}
		mutated_word.insert(insertionPoint, randomFragment);
	} else {
		// удаление
		int numDeletions = uniform_int_distribution<int>(1, mutated_word.size() / 2)(gen);
		for (int i = 0; i < numDeletions; ++i) {
			int deletionPoint = uniform_int_distribution<int>(0, mutated_word.size() - 1)(gen);
			mutated_word.erase(deletionPoint, 1);
		}
	}

	auto s = word;
	s.insert(r, mutated_word);
	return s;
}

size_t MutationHasher::operator()(const std::tuple<int, int, int>& t) const {
	return std::hash<int>{}(std::get<0>(t)) ^ std::hash<int>{}(std::get<2>(t) - std::get<1>(t));
}

TraversalState::TraversalState(const MFAState* state) : state(state) {}

TraversalState::TraversalState(const string& str, const MFAState* state,
							   MemoryConfiguration opened_cells, MemoryContents memory,
							   const TraversalState& previous_state, bool memory_used)
	: str(str), state(state), opened_cells(std::move(opened_cells)), memory(std::move(memory)),
	  visited_path(previous_state.visited_path), visited_states(previous_state.visited_states),
	  last_memory_reading(previous_state.last_memory_reading),
	  substrs_to_mutate(previous_state.substrs_to_mutate) {
	if (memory_used)
		last_memory_reading = str.size();
}

bool TraversalState::operator==(const TraversalState& other) const {
	return str == other.str && state == other.state && opened_cells == other.opened_cells &&
		   memory == other.memory;
}

void TraversalState::process_mutations() {
	auto visited_state = visited_states.find(state->index);
	// нашли цикл для мутации
	if (visited_state != visited_states.end()) {
		auto [index_in_visited_path, prev_size] = visited_state->second;
		// если с момента последнего посещения не было чтения из памяти,
		// добавляем подстроку в список мутаций
		if (prev_size != str.size() && prev_size > last_memory_reading)
			substrs_to_mutate.insert({state->index, prev_size, str.size()});
		// удаляем все промежуточные посещенные состояния
		for (int i = index_in_visited_path; i < visited_path.size(); i++)
			visited_states.erase(visited_path[i]);
	}
	visited_path.push_back(state->index);
	// сохраняем позиции в автомате и строке на момент посещения
	visited_states[state->index] = {visited_path.size() - 1, str.size()};
}

size_t TraversalState::Hasher::operator()(const TraversalState& s) const {
	std::size_t seed = 0;

	hash_combine(seed, s.str);
	hash_combine(seed, s.state);

	for (const auto& cell : s.opened_cells) {
		hash_combine(seed, cell);
	}

	for (const auto& entry : s.memory) {
		hash_combine(seed, entry.first);
		// если память непустая
		if (entry.second.first != entry.second.second) {
			hash_combine(seed, entry.second.first);
			hash_combine(seed, entry.second.second);
		}
	}

	return seed;
}

pair<unordered_set<string>, unordered_set<string>> MemoryFiniteAutomaton::generate_test_set(
	int max_len) const {
	unordered_set<string> words_in_language;
	unordered_map<string, IntPairSet> words_to_mutate;

	unordered_set<TraversalState, TraversalState::Hasher> current_states;
	current_states.insert(TraversalState(&states[initial_state]));

	unordered_set<TraversalState, TraversalState::Hasher> visited_states;
	while (!current_states.empty()) {
		unordered_set<TraversalState, TraversalState::Hasher> following_states;
		for (const auto& state_to_process : current_states) {
			if (visited_states.count(state_to_process))
				continue;

			auto cur_state = state_to_process;
			cur_state.process_mutations();
			const MFAState* state = cur_state.state;
			if (state->is_terminal) {
				words_in_language.insert(cur_state.str);
				for (const auto& [_, l, r] : cur_state.substrs_to_mutate)
					words_to_mutate[cur_state.str].insert({l, r});
			}

			// выбор переходов
			ParseTransitions reach, reach_eps;
			for (const auto& [symbol, symbol_transitions] : cur_state.state->transitions) {
				if (symbol.is_ref()) {
					if (!cur_state.memory.count(symbol.get_ref())) {
						// пустая ссылка добавляется к eps-переходам
						reach_eps.add_transitions(symbol, symbol_transitions, cur_state);
						continue;
					}
					const auto& [l, r] = cur_state.memory.at(symbol.get_ref());
					if (l == r) {
						// пустая ссылка добавляется к eps-переходам
						reach_eps.add_transitions(symbol, symbol_transitions, cur_state);
						continue;
					}
					MFAState::SymbolTransitions non_empty_ref_transitions, empty_ref_transitions;
					for (const auto& tr : symbol_transitions) {
						auto ref_cell_memory_action = tr.memory_actions.find(symbol.get_ref());
						if (ref_cell_memory_action != tr.memory_actions.end() &&
							ref_cell_memory_action->second == MFATransition::reset) {
							empty_ref_transitions.insert(tr);
							continue;
						}
						non_empty_ref_transitions.insert(tr);
					}
					if (!empty_ref_transitions.empty()) {
						reach_eps.add_transitions(symbol, empty_ref_transitions, cur_state);
					}
					reach.add_transitions(symbol, non_empty_ref_transitions, cur_state);
				} else if (symbol.is_epsilon()) {
					reach_eps.add_transitions(symbol, symbol_transitions, cur_state);
				} else {
					reach.add_transitions(symbol, symbol_transitions, cur_state);
				}
			}

			// переходы в новые состояния по букве/непустой ссылке
			for (const auto& [symbol, symbol_transitions] : reach) {
				for (auto tr : symbol_transitions) {
					if (cur_state.str.size() + get_symbol_len(tr.memory, symbol) > max_len)
						continue;
					tr.update_memory_contents(symbol);
					if (symbol.is_ref()) {
						pair<int, int> substr = tr.memory.at(symbol.get_ref());
						following_states.emplace(
							cur_state.str +
								cur_state.str.substr(substr.first, substr.second - substr.first),
							&states[tr.to],
							tr.opened_cells,
							tr.memory,
							cur_state,
							true);
					} else {
						following_states.emplace(cur_state.str + string(symbol),
												 &states[tr.to],
												 tr.opened_cells,
												 tr.memory,
												 cur_state);
					}
				}
			}

			// эпсилон-переходы
			for (const auto& [_, symb_transitions] : reach_eps) {
				for (auto eps_tr : symb_transitions) {
					eps_tr.update_memory_contents(Symbol::Epsilon);
					following_states.emplace(cur_state.str,
											 &states[eps_tr.to],
											 eps_tr.opened_cells,
											 eps_tr.memory,
											 cur_state);
				}
			}

			visited_states.insert(cur_state);
		}
		current_states = following_states;
	}

	unordered_set<string> mutated_words;
	for (const auto& [word, mutations] : words_to_mutate)
		for (const auto& mutation_bounds : mutations) {
			string mutation = random_mutation(
				word, mutation_bounds.first, mutation_bounds.second, language->get_alphabet());
			if (!words_in_language.count(mutation))
				mutated_words.insert(mutation);
		}

	return {words_in_language, mutated_words};
}

FiniteAutomaton MemoryFiniteAutomaton::to_fa() const {
	vector<FAState> fa_states;
	Alphabet alphabet;
	fa_states.reserve(states.size());
	for (const auto& state : states)
		fa_states.emplace_back(state, alphabet);
	return {initial_state, fa_states, alphabet};
}

bool MemoryFiniteAutomaton::action_bisimilar(const MemoryFiniteAutomaton& mfa1,
											 const MemoryFiniteAutomaton& mfa2, iLogTemplate* log) {
	FiniteAutomaton fa1(mfa1.to_fa()), fa2(mfa2.to_fa());
	bool result = FiniteAutomaton::bisimilar(fa1, fa2);
	if (log) {
		log->set_parameter("mfa1", mfa1);
		log->set_parameter("mfa2", mfa2);
		log->set_parameter("result", result ? "True" : "False");
	}
	return result;
}

FiniteAutomaton MemoryFiniteAutomaton::to_fa_mem() const {
	int n = size();
	vector<FAState> fa_states(n);
	Alphabet alphabet;
	for (int i = 0; i < size(); i++) {
		const auto& state = states[i];
		fa_states[i] = FAState(i, state.identifier, state.is_terminal);
		for (const auto& [symbol, symbol_transitions] : state.transitions) {
			alphabet.insert(symbol);
			for (const auto& transition : symbol_transitions) {
				set<int> opens;
				set<int> closes;
				set<int> resets;
				for (const auto& [num, action] : transition.memory_actions) {
					switch (action) {
					case MFATransition::open:
						opens.insert(num);
						break;
					case MFATransition::close:
						closes.insert(num);
						break;
					case MFATransition::reset:
						resets.insert(num);
						break;
					}
				}
				int start = n;
				for (auto ind : closes)
					fa_states.emplace_back(n++, "C" + std::to_string(ind), false);
				for (auto ind : resets)
					fa_states.emplace_back(n++, "R" + std::to_string(ind), false);
				for (auto ind : opens)
					fa_states.emplace_back(n++, "O" + std::to_string(ind), false);

				if (n > start) {
					alphabet.insert(fa_states[start].identifier);
					fa_states[i].transitions[fa_states[start].identifier].insert(start);
					for (int j = start; j < n - 1; j++) {
						alphabet.insert(fa_states[j + 1].identifier);
						fa_states[j].transitions[fa_states[j + 1].identifier].insert(j + 1);
					}
					fa_states[fa_states.size() - 1].transitions[symbol].insert(transition.to);
				} else {
					fa_states[i].transitions[symbol].insert(transition.to);
				}
			}
		}
	}
	return {initial_state, fa_states, alphabet};
}

bool MemoryFiniteAutomaton::literally_bisimilar(const MemoryFiniteAutomaton& mfa1,
												const MemoryFiniteAutomaton& mfa2,
												iLogTemplate* log) {
	FiniteAutomaton fa1(mfa1.to_fa_mem()), fa2(mfa2.to_fa_mem());
	bool result = FiniteAutomaton::bisimilar(fa1, fa2);
	if (log) {
		log->set_parameter("mfa1", mfa1);
		log->set_parameter("mfa2", mfa2);
		log->set_parameter("result", result ? "True" : "False");
	}
	return result;
}

MemoryConfiguration update_memory_configuration(const MFATransition::MemoryActions& memory_actions,
												const MemoryConfiguration& opened_cells) {
	MemoryConfiguration updated(opened_cells);
	for (const auto [num, action] : memory_actions) {
		if (action == MFATransition::MemoryAction::open) {
			updated.insert(num);
		} else if (action == MFATransition::MemoryAction::close) {
			if (!opened_cells.count(num))
				std::cerr << "update_memory_configuration: cell is already closed\n";
			updated.erase(num);
		} else if (action == MFATransition::MemoryAction::reset) {
			updated.erase(num);
		}
	}
	return updated;
}

void MemoryFiniteAutomaton::color_mem_dfs(int state_index, vector<bool>& visited,
										  const MemoryConfiguration& opened_cells,
										  unordered_map<int, unordered_set<int>>& colors) const {
	visited[state_index] = true;
	colors[state_index] = opened_cells;
	for (const auto& [symbol, symbol_transitions] : states[state_index].transitions) {
		for (const auto& tr : symbol_transitions) {
			if (!visited[tr.to])
				color_mem_dfs(tr.to,
							  visited,
							  update_memory_configuration(tr.memory_actions, opened_cells),
							  colors);
		}
	}
}

vector<MFAState::Transitions> MemoryFiniteAutomaton::get_reversed_transitions() const {
	vector<MFAState::Transitions> res(size());

	for (int i = 0; i < size(); ++i)
		for (const auto& [symbol, symbol_transitions] : states[i].transitions)
			for (const auto& tr : symbol_transitions)
				res[tr.to][symbol].insert(MFATransition(i, tr.memory_actions));

	return res;
}

void find_opening_states_dfs(int state_index,
							 const vector<MFAState::Transitions>& reversed_transitions,
							 vector<bool>& visited,				 // NOLINT(runtime/references)
							 unordered_set<int>& opening_states, // NOLINT(runtime/references)
							 int cell) {
	visited[state_index] = true;

	for (const auto& [symbol, symbol_transitions] : reversed_transitions[state_index])
		for (const auto& tr : symbol_transitions) {
			optional<MFATransition::MemoryAction> action;
			if (tr.memory_actions.count(cell))
				action = tr.memory_actions.at(cell);
			if (action && (action == MFATransition::open || action == MFATransition::reset))
				opening_states.insert(tr.to);
			else if (!visited[tr.to])
				find_opening_states_dfs(tr.to, reversed_transitions, visited, opening_states, cell);
		}
}

vector<vector<int>> MemoryFiniteAutomaton::find_cg_traces(int state_index,
														  unordered_set<int> visited, int cell,
														  int opening_state) const {
	vector<vector<int>> res;
	visited.insert(state_index);

	for (const auto& [symbol, symbol_transitions] : states[state_index].transitions)
		for (const auto& tr : symbol_transitions) {
			optional<MFATransition::MemoryAction> action;
			if (tr.memory_actions.count(cell))
				action = tr.memory_actions.at(cell);
			if (action && (action == MFATransition::close || action == MFATransition::reset)) {
				res.push_back({state_index});
			} else if (!visited.count(tr.to) && !(state_index == opening_state &&
												  (!action || action != MFATransition::open))) {
				auto t = find_cg_traces(tr.to, visited, cell, opening_state);
				for (auto i : t) {
					i.insert(i.begin(), state_index);
					res.emplace_back(i);
				}
			}
		}

	return res;
}

vector<CaptureGroup> MemoryFiniteAutomaton::find_capture_groups_backward(
	int ref_incoming_state, int cell, const std::vector<int>& fa_classes) const {
	vector<MFAState::Transitions> reversed_transitions = get_reversed_transitions();
	unordered_set<int> opening_states;
	vector<bool> visited(size(), false);
	find_opening_states_dfs(
		ref_incoming_state, reversed_transitions, visited, opening_states, cell);

	vector<CaptureGroup> res;

	for (auto opening_st : opening_states) {
		auto traces = find_cg_traces(opening_st, {}, cell, opening_st);
		// отделяем ресеты
		for (auto it = traces.begin(); it != traces.end();) {
			if (it->size() == 1) {
				res.push_back(CaptureGroup(cell, {*it}, fa_classes));
				it = traces.erase(it);
			} else {
				++it;
			}
		}
		res.emplace_back(cell, traces, fa_classes);
	}
	return res;
}

bool MemoryFiniteAutomaton::find_path_decisions(int state_index, vector<int>& visited,
												const unordered_set<int>& path_states) const {
	visited[state_index] = 1;

	optional<MFATransition> single_tr;
	int count = 0;
	for (const auto& [symbol, symbol_transitions] : states[state_index].transitions)
		for (const auto& tr : symbol_transitions)
			if (path_states.count(tr.to)) {
				if (visited[tr.to] == 0) {
					if (++count > 1)
						return true;
					single_tr = tr;
				} else if (visited[tr.to] == 1) {
					return true;
				}
			}

	bool found = false;
	if (single_tr)
		found = find_path_decisions(single_tr->to, visited, path_states);

	visited[state_index] = 2;
	return found;
}

bool MemoryFiniteAutomaton::path_contains_decisions(const unordered_set<int>& path_states) const {
	vector<int> visited(size(), 0);
	for (auto start : path_states) {
		if (visited[start] != 0)
			continue;
		if (find_path_decisions(start, visited, path_states))
			return true;
	}
	return false;
}

optional<bool> MemoryFiniteAutomaton::bisimilarity_checker(const MemoryFiniteAutomaton& mfa1,
														   const MemoryFiniteAutomaton& mfa2) {
	const int N = 2;
	// раскрашиваем состояния
	vector<unordered_map<int, unordered_set<int>>> mfa_colors(N);
	vector<bool> visited(mfa1.size(), false);
	mfa1.color_mem_dfs(mfa1.get_initial(), visited, {}, mfa_colors[0]);
	visited.assign(mfa2.size(), false);
	mfa2.color_mem_dfs(mfa2.get_initial(), visited, {}, mfa_colors[1]);
	//	using std::cout;
	//	cout << mfa1.to_txt() << mfa2.to_txt();
	for (const auto& mfa_colors_i : mfa_colors)
		for (const auto& j : mfa_colors_i) {
			if (j.second.size() > 1)
				return std::nullopt;
			//			cout << j.first << ": ";
			//			cout << j.second;
		}
	// проверяем action bisimilarity
	vector<FiniteAutomaton> fas({mfa1.to_fa(), mfa2.to_fa()});
	auto [res, fa_classes] = FiniteAutomaton::bisimilarity_checker(fas[0], fas[1]);
	if (!res)
		return false;
	// проверяем совпадение раскраски эквивалентных состояний в КСС
	vector<vector<vector<int>>> SCCs({fas[0].get_SCCs(), fas[1].get_SCCs()});
	vector<multiset<multiset<pair<int, set<int>>>>> colored_SCCs(N);
	for (int i = 0; i < N; i++) {
		for (const auto& SCC : SCCs[i]) {
			multiset<pair<int, set<int>>> colored_SCC;
			for (auto j : SCC) {
				if (!mfa_colors[i].at(j).empty()) {
					auto j_colors = mfa_colors[i].at(j);
					colored_SCC.insert(
						{fa_classes[i][j], set<int>(j_colors.begin(), j_colors.end())});
				}
			}
			if (!colored_SCC.empty())
				colored_SCCs[i].insert(colored_SCC);
		}
	}
	//	for (int i = 0; i < N; i++) {
	//		for (const auto& j : colored_SCCs[i]) {
	//			cout << "(\n";
	//			for (auto [state_class, colors] : j)
	//				cout << state_class << ": " << colors;
	//			cout << ")\n";
	//		}
	//		cout << "----\n";
	//	}

	if (colored_SCCs[0] != colored_SCCs[1])
		return false;

	// ищем пары состояний, от которых будем делать обратный расчет
	vector<unordered_map<int, vector<int>>> class_to_states(N);
	for (int i = 0; i < N; i++)
		for (int st = 0; st < fa_classes[i].size(); st++)
			class_to_states[i][fa_classes[i][st]].emplace_back(st);

	vector<vector<FAState::Transitions>> reversed_transitions(
		{fas[0].get_reversed_transitions(), fas[1].get_reversed_transitions()});
	vector<unordered_map<int, unordered_set<int>>> incoming_refs(N);
	for (int i = 0; i < N; i++)
		for (int st = 0; st < reversed_transitions[i].size(); st++) {
			unordered_set<int> found_refs;
			for (const auto& [symbol, _] : reversed_transitions[i][st])
				if (symbol.is_ref())
					found_refs.insert(symbol.get_ref());
			if (!found_refs.empty())
				incoming_refs[i][st] = found_refs;
		}

	unordered_set<tuple<int, int, int>, TupleHasher>
		pairs_to_calc; // {номер ячейки, состояние первого автомата, состояние второго}
	for (const auto& [fa1_st, fa1_st_incoming_refs] : incoming_refs[0]) {
		int fa1_st_class = fa_classes[0][fa1_st];
		for (auto fa2_st : class_to_states[1].at(fa1_st_class))
			for (auto fa2_st_incoming_ref : incoming_refs[1].at(fa2_st))
				if (fa1_st_incoming_refs.count(fa2_st_incoming_ref))
					pairs_to_calc.insert({fa2_st_incoming_ref, fa1_st, fa2_st});
	}

	//	for (const auto& i : pairs_to_calc)
	//		cout << i;

	vector<pair<vector<CaptureGroup>, vector<CaptureGroup>>> capture_groups_to_cmp;
	capture_groups_to_cmp.reserve(pairs_to_calc.size());
	for (const auto& [cell, st1, st2] : pairs_to_calc) {
		capture_groups_to_cmp.emplace_back(
			mfa1.find_capture_groups_backward(st1, cell, fa_classes[0]),
			mfa2.find_capture_groups_backward(st1, cell, fa_classes[1]));
	}

	for (const auto& CGs : capture_groups_to_cmp) {
		//		cout << "---------\n";
		//		for (const auto& j : CGs.first)
		//			cout << j;
		//		cout << "<>\n";
		//		for (const auto& j : CGs.second)
		//			cout << j;
		//		cout << "---------\n";

		const auto& CGs1 = CGs.first;
		const auto& CGs2 = CGs.second;

		unordered_set<int> check_set1, check_set2;
		for (int i = 0; i < CGs1.size(); i++)
			for (int j = 0; j < CGs2.size(); j++) {
				const auto &cg1 = CGs1[i], cg2 = CGs2[j];
				unordered_set<int> states_to_check_1 = cg1.get_states_diff(cg2.state_classes),
								   states_to_check_2 = cg2.get_states_diff(cg1.state_classes);

				if (!mfa1.path_contains_decisions(states_to_check_1) &&
					!mfa2.path_contains_decisions(states_to_check_2)) {
					check_set1.insert(i);
					check_set2.insert(j);
				}
			}

		if (check_set1.size() != CGs1.size() || check_set2.size() != CGs2.size())
			return false;

		FiniteAutomaton CGs1_fa(fas[0].get_subautomaton(CGs1[0])),
			CGs2_fa(fas[1].get_subautomaton(CGs2[0]));
		for (int i = 1; i < CGs1.size(); i++)
			CGs1_fa = FiniteAutomaton::uunion(CGs1_fa, fas[0].get_subautomaton(CGs1[i]));
		for (int i = 1; i < CGs2.size(); i++)
			CGs2_fa = FiniteAutomaton::uunion(CGs2_fa, fas[1].get_subautomaton(CGs2[i]));
		if (!FiniteAutomaton::equivalent(CGs1_fa, CGs2_fa))
			return false;
	}

	return true;
}

optional<bool> MemoryFiniteAutomaton::bisimilar(const MemoryFiniteAutomaton& mfa1,
												const MemoryFiniteAutomaton& mfa2,
												iLogTemplate* log) {
	optional<bool> result = bisimilarity_checker(mfa1, mfa2);

	if (log) {
		if (result)
			log->set_parameter("result", *result ? "True" : "False");
		else
			log->set_parameter("result", "Unknown");

		log->set_parameter("mfa1", mfa1);
		log->set_parameter("mfa2", mfa2);
	}
	return result;
}
