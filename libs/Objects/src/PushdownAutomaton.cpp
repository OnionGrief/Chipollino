#include "Objects/FiniteAutomaton.h"
#include "Objects/Language.h"
#include "Objects/iLogTemplate.h"
#include <Objects/PushdownAutomaton.h>
#include <Objects/Symbol.h>
#include <sstream>

#include <utility>

using std::optional;
using std::pair;
using std::set;
using std::stack;
using std::string;
using std::stringstream;
using std::tuple;
using std::unordered_map;
using std::unordered_set;
using std::vector;

template <typename Range, typename Value = typename Range::value_type>
std::string Join(Range const& elements, const char *const delimiter) {
	std::ostringstream os;
	auto b = begin(elements), e = end(elements);

	if (b != e) {
		std::copy(b, prev(e), std::ostream_iterator<Value>(os, delimiter));
		b = prev(e);
	}
	if (b != e) {
		os << *b;
	}

	return os.str();
}

template <typename T> void hash_combine(std::size_t& seed, const T& v) {
	seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

PDATransition::PDATransition(const int to, const Symbol& input, const Symbol& pop, const std::vector<Symbol>& push)
	: to(to), input_symbol(input), push(push), pop(pop) {}

bool PDATransition::operator==(const PDATransition& other) const {
	return to == other.to && input_symbol == other.input_symbol && push == other.push && pop == other.pop;
}

std::size_t PDATransition::Hasher::operator()(const PDATransition& t) const {
	std::size_t hash = 0;
	hash += 123;
	return hash;
}

PDAState::PDAState(int index, bool is_terminal) : State(index, {}, is_terminal) {}

PDAState::PDAState(int index, string identifier, bool is_terminal)
	: State(index, std::move(identifier), is_terminal) {}

PDAState::PDAState(int index, std::string identifier, bool is_terminal, Transitions transitions)
	: State(index, std::move(identifier), is_terminal), transitions(std::move(transitions)) {}

std::string PDAState::to_txt() const {
	return {};
}

void PDAState::set_transition(const PDATransition& to, const Symbol& input_symbol) {
	transitions[input_symbol].insert(to);
}

PushdownAutomaton::PushdownAutomaton() : AbstractMachine() {}

PushdownAutomaton::PushdownAutomaton(int initial_state, std::vector<PDAState> states,
									 Alphabet alphabet)
	: AbstractMachine(initial_state, std::move(alphabet)), states(std::move(states)) {
	for (int i = 0; i < this->states.size(); i++) {
		if (this->states[i].index != i)
			throw std::logic_error(
				"State.index must correspond to its ordinal number in the states vector");
	}
}

PushdownAutomaton::PushdownAutomaton(int initial_state, vector<PDAState> states,
									 std::shared_ptr<Language> language)
	: AbstractMachine(initial_state, std::move(language)), states(std::move(states)) {
	for (int i = 0; i < this->states.size(); i++) {
		if (this->states[i].index != i)
			throw std::logic_error(
				"State.index must correspond to its ordinal number in the states vector");
	}
}

std::string PushdownAutomaton::to_txt() const {
	stringstream ss;
	ss << "digraph {\n\trankdir = LR\n\tdummy [label = \"\", shape = none]\n\t";
	for (const auto & state : states) {
		ss << state.index << " [label = \"" << state.identifier << "\", shape = ";
		ss << (state.is_terminal ? "doublecircle]\n\t" : "circle]\n\t");
	}
	if (states.size() > initial_state)
		ss << "dummy -> " << states[initial_state].index << "\n";

	for (const auto& state : states) {
		for (const auto& elem : state.transitions) {
			for (const auto& transition : elem.second) {
				ss << "\t" << state.index << " -> " << transition.to << " [label = \""
				   << string(elem.first) << ", " << transition.pop << "/" << Join(transition.push, ",") << "\"]\n";
			}
		}
	}

	ss << "}\n";
	return ss.str();
}

std::vector<PDAState> PushdownAutomaton::get_states() const {
	return states;
}

size_t PushdownAutomaton::size(iLogTemplate* log) const {
	return states.size();
}

bool PushdownAutomaton::is_deterministic(iLogTemplate* log) const {
	bool result = true;
	for (const auto& state : states) {
		// Отображение символов стэка в символы алфавита
		std::unordered_map<Symbol, std::unordered_set<Symbol, Symbol::Hasher>, Symbol::Hasher> stack_sym_to_sym;
		for (const auto& [symb, symbol_transitions] : state.transitions) {
			for (const auto& tr : symbol_transitions) {
				if (symb.is_epsilon() && !stack_sym_to_sym[tr.pop].empty()) {
					// Переход по эпсилону с pop некоторого символа стэка.
					// С этим символом стэка не должно быть иных переходов.
					result = false;
					break;
				}

				if (stack_sym_to_sym[tr.pop].count(symb) || stack_sym_to_sym[tr.pop].count(Symbol::Epsilon)) {
					// Перехода по одной и той же паре (symb, stack_symb) не должно быть.
					// Так же не может быть перехода по символу стэка, для которого ранее
					// зафиксировали наличие эпсилон-перехода.
					result = false;
					break;
				}

				stack_sym_to_sym[tr.pop].emplace(symb);
			}
		}
	}
	if (log) {
		log->set_parameter("result", result ? "True" : "False");
	}
	return result;
}

PushdownAutomaton PushdownAutomaton::complement(iLogTemplate* log) const {
	return PushdownAutomaton();
}

std::vector<PDATransition> get_regular_transitions(const string& s, const ParsingState& parsing_state) {
	std::vector<PDATransition> transitions;
	const Symbol symb(parsing_state.pos < s.size() ? s[parsing_state.pos] : char());
	if (parsing_state.state->transitions.find(symb) == parsing_state.state->transitions.end()) {
		return transitions;
	}

	auto symbol_transitions = parsing_state.state->transitions.at(symb);
	for (auto tr: symbol_transitions) {
		if (!parsing_state.stack.empty() && parsing_state.stack.top() == tr.pop) {
			transitions.emplace_back(tr);
		}
	}

	return transitions;
}

std::vector<std::pair<bool, PDATransition>> get_epsilon_transitions(const string& s, const ParsingState& parsing_state) {
	std::vector<std::pair<bool, PDATransition>> epsilon_transitions;

	if (parsing_state.state->transitions.find(Symbol::Epsilon) != parsing_state.state->transitions.end()) {
		for (auto tr : parsing_state.state->transitions.at(Symbol::Epsilon)) {
			// переход по epsilon -> не потребляем символ
			if (tr.pop.is_epsilon() || (!parsing_state.stack.empty() && parsing_state.stack.top() == tr.pop)) {
				epsilon_transitions.emplace_back(false, tr);
			}
		}
	}

	const Symbol symb(parsing_state.pos < s.size() ? s[parsing_state.pos] : char());
	if (parsing_state.state->transitions.find(symb) == parsing_state.state->transitions.end()) {
		return epsilon_transitions;
	}

	auto symbol_transitions = parsing_state.state->transitions.at(symb);
	for (auto tr: symbol_transitions) {
		// переход по не-epsilon, epsilon на стэке -> потребляем символ + эпсилон-переход
		if (tr.pop.is_epsilon()) {
			epsilon_transitions.emplace_back(true, tr);
		}
	}

	return epsilon_transitions;
}

std::stack<Symbol> perform_stack_actions(std::stack<Symbol> stack, const PDATransition& tr) {
	auto result = stack;
	if (!tr.pop.is_epsilon()) {
		result.pop();
	}
	for (const auto& push_sym:tr.push) {
		if (!push_sym.is_epsilon()) {
			result.push(push_sym);
		}
	}
	return result;
}

std::pair<int, bool> PushdownAutomaton::parse(const std::string& s) const {
	int counter = 0, parsed_len = 0;
	const PDAState* state = &states[initial_state];
	set<tuple<int, int, int>> visited_eps;
	std::stack<Symbol> pda_stack;
	std::stack<ParsingState> parsing_stack;
	parsing_stack.emplace(parsed_len, state, pda_stack);

	while (!parsing_stack.empty()) {
		if (state->is_terminal && parsed_len == s.size()) {
			break;
		}

		auto parsing_state = parsing_stack.top();
		parsing_stack.pop();

		state = parsing_state.state;
		parsed_len = parsing_state.pos;
		pda_stack = parsing_state.stack;
		counter++;

		auto transitions = get_regular_transitions(s, parsing_state);
		if (parsed_len + 1 <= s.size()) {
			for (auto tr: transitions) {
				parsing_stack.emplace(parsed_len+1, &states[tr.to], perform_stack_actions(parsing_state.stack, tr));
			}
		}

		// если произошёл откат по строке, то эпсилон-переходы из рассмотренных состояний больше не
		// считаются повторными
		if (!visited_eps.empty()) {
			set<tuple<int, int, int>> temp_eps;
			for (auto pos : visited_eps) {
				if (std::get<0>(pos) < parsed_len)
					temp_eps.insert(pos);
			}
			visited_eps = temp_eps;
		}

		// добавление тех эпсилон-переходов, по которым ещё не было разбора от этой позиции и этого
		// состояния и этого стэкового символа
		auto eps_transitions = get_epsilon_transitions(s, parsing_state);
		for (const auto& [consume, tr] : eps_transitions) {
			if (!visited_eps.count({parsed_len, state->index, tr.to})) {
				parsing_stack.emplace(parsed_len+consume, &states[tr.to], perform_stack_actions(parsing_state.stack, tr));
				visited_eps.insert({parsed_len+consume, state->index, tr.to});
			}
		}
	}

	if (s.size() == parsed_len && state->is_terminal) {
		return {counter, true};
	}

	return {counter, false};
}