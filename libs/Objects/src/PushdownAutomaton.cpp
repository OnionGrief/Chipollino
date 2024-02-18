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
std::string Join(Range const& elements, const char* const delimiter) {
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

struct pairhash {
  public:
	template <typename T, typename U> std::size_t operator()(const std::pair<T, U>& x) const {
		return std::hash<T>()(x.first) ^ std::hash<U>()(x.second);
	}
};

template <typename T> void hash_combine(std::size_t& seed, const T& v) {
	seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

PDATransition::PDATransition(const int to, const Symbol& input, const Symbol& pop,
							 const std::vector<Symbol>& push)
	: to(to), input_symbol(input), push(push), pop(pop) {}

bool PDATransition::operator==(const PDATransition& other) const {
	return to == other.to && input_symbol == other.input_symbol && push == other.push &&
		   pop == other.pop;
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
	for (const auto& state : states) {
		ss << state.index << " [label = \"" << state.identifier << "\", shape = ";
		ss << (state.is_terminal ? "doublecircle]\n\t" : "circle]\n\t");
	}
	if (states.size() > initial_state)
		ss << "dummy -> " << states[initial_state].index << "\n";

	for (const auto& state : states) {
		for (const auto& elem : state.transitions) {
			for (const auto& transition : elem.second) {
				ss << "\t" << state.index << " -> " << transition.to << " [label = \""
				   << string(elem.first) << ", " << transition.pop << "/"
				   << Join(transition.push, ",") << "\"]\n";
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

PushdownAutomaton PushdownAutomaton::_remove_unreachable_states(iLogTemplate* log) {
	if (states.size() == 1) {
		return *this;
	}

	std::unordered_set<int> reachable_states = {initial_state};
	std::queue<int> states_to_visit;
	states_to_visit.push(initial_state);

	// Perform BFS to find reachable states
	while (!states_to_visit.empty()) {
		int current_state_index = states_to_visit.front();
		states_to_visit.pop();
		const PDAState& current_state = states[current_state_index];

		// Visit transitions from the current state
		for (const auto& [symbol, symbol_transitions] : current_state.transitions) {
			for (const auto& trans : symbol_transitions) {
				int index = trans.to;
				// If the next state is not already marked as reachable
				if (!reachable_states.count(index)) {
					reachable_states.insert(index);
					states_to_visit.push(index);
				}
			}
		}
	}

	// Remove unreachable states from the states vector
	std::vector<PDAState> new_states;
	int i = 0, new_initial_index;
	std::map<int, int> recover_index;
	for (auto state : states) {
		if (!reachable_states.count(state.index)) {
			continue;
		}

		if (state.index == initial_state) {
			new_initial_index = i;
		}
		recover_index[state.index] = i;
		state.index = i;
		new_states.emplace_back(state);
		i++;
	}

	// Remap transitions
	for (auto& state : new_states) {
		PDAState::Transitions new_transitions;
		for (const auto& [symbol, symbol_transitions] : state.transitions) {
			for (const auto& trans : symbol_transitions) {
				new_transitions[symbol].insert(PDATransition(
					recover_index[trans.to], trans.input_symbol, trans.pop, trans.push));
			}
		}
		state.transitions = new_transitions;
	}

	return {new_initial_index, new_states, get_language()->get_alphabet()};
}

PushdownAutomaton PushdownAutomaton::regular_intersect(const Regex& re, iLogTemplate* log) {
	auto dfa = re.to_thompson(log).determinize(log).minimize(log);

	// Flatten PDA transitions
	using PDA_from = int;
	std::vector<std::pair<PDA_from, PDATransition>> pda_transitions_by_from;
	for (const auto& state : states) {
		for (const auto& [symbol, symbol_transitions] : state.transitions) {
			for (const auto& trans : symbol_transitions) {
				pda_transitions_by_from.emplace_back(state.index, trans);
			}
		}
	}

	// Initialize states.
	// recover_index maps (pda_index, dfa_index) to result_index.
	std::vector<PDAState> result_states;
	std::unordered_map<std::pair<int, int>, int, pairhash> recover_index;
	int i = 0;
	for (const auto& pda_state : states) {
		for (const auto& dfa_state : dfa.get_states()) {
			std::ostringstream oss;
			oss << "(" << pda_state.identifier << ";" << dfa_state.identifier << ")";
			result_states.emplace_back(
				i, oss.str(), pda_state.is_terminal && dfa_state.is_terminal);
			recover_index[{pda_state.index, dfa_state.index}] = i;
			i++;
		}
	}

	// Calculate transitions
	for (const auto& [pda_from, pda_trans] : pda_transitions_by_from) {
		for (const auto& dfa_state : dfa.get_states()) {
			// Process epsilon transitions separately
			if (pda_trans.input_symbol.is_epsilon()) {
				auto from_index = recover_index[{pda_from, dfa_state.index}];
				auto to_index = recover_index[{pda_trans.to, dfa_state.index}];
				result_states[from_index].set_transition(
					PDATransition(to_index, pda_trans.input_symbol, pda_trans.pop, pda_trans.push),
					pda_trans.input_symbol);
				continue;
			}

			// Process regular transitions separately
			auto matching_transitions = dfa_state.transitions.find(pda_trans.input_symbol);
			if (matching_transitions == dfa_state.transitions.end()) {
				continue;
			}

			for (const auto& dfa_to : matching_transitions->second) {
				auto from_index = recover_index[{pda_from, dfa_state.index}];
				auto to_index = recover_index[{pda_trans.to, dfa_to}];
				result_states[from_index].set_transition(
					PDATransition(to_index, pda_trans.input_symbol, pda_trans.pop, pda_trans.push),
					pda_trans.input_symbol);
			}
		}
	}

	auto result_initial_state = recover_index[{initial_state, dfa.get_initial()}];

	// Calculate resulting alphabet
	auto pda_alphabet = get_language()->get_alphabet();
	auto dfa_alphabet = dfa.get_language()->get_alphabet();
	set<Symbol> result_alphabet;
	std::set_intersection(pda_alphabet.begin(),
						  pda_alphabet.end(),
						  dfa_alphabet.begin(),
						  dfa_alphabet.end(),
						  std::inserter(result_alphabet, result_alphabet.begin()));

	auto result = PushdownAutomaton(result_initial_state, result_states, result_alphabet);
	result = result._remove_unreachable_states(log);

	if (log) {
		log->set_parameter("pda", *this);
		log->set_parameter("regex", dfa);
		log->set_parameter("result", result);
	}

	return result;
}

bool PushdownAutomaton::is_deterministic(iLogTemplate* log) const {
	bool result = true;
	std::unordered_set<int> nondeterministic_states;
	for (const auto& state : states) {
		// Отображение символов стэка в символы алфавита
		std::unordered_map<Symbol, std::unordered_set<Symbol, Symbol::Hasher>, Symbol::Hasher>
			stack_sym_to_sym;
		for (const auto& [symb, symbol_transitions] : state.transitions) {
			for (const auto& tr : symbol_transitions) {
				if (symb.is_epsilon() && !stack_sym_to_sym[tr.pop].empty()) {
					// Переход по эпсилону с pop некоторого символа стэка.
					// С этим символом стэка не должно быть иных переходов.
					result = false;
					nondeterministic_states.emplace(state.index);
					break;
				}

				if (stack_sym_to_sym[tr.pop].count(symb) ||
					stack_sym_to_sym[tr.pop].count(Symbol::Epsilon)) {
					// Перехода по одной и той же паре (symb, stack_symb) не должно быть.
					// Так же не может быть перехода по символу стэка, для которого ранее
					// зафиксировали наличие эпсилон-перехода.
					result = false;
					nondeterministic_states.emplace(state.index);
					break;
				}

				stack_sym_to_sym[tr.pop].emplace(symb);
			}
		}
	}

	if (log) {
		MetaInfo meta;
		for (const auto& state : states) {
			if (nondeterministic_states.count(state.index)) {
				meta.upd(NodeMeta{state.index, MetaInfo::trap_color});
			}
		}
		log->set_parameter("pda", *this, meta);
		log->set_parameter("result", result ? "True" : "False");
	}

	return result;
}

std::pair<MetaInfo, PushdownAutomaton> PushdownAutomaton::_add_trap_state(iLogTemplate* log) {
	std::vector<PDAState> new_states = states;
	bool has_trap = false;
	MetaInfo meta;
	int count = static_cast<int>(size());
	for (auto& state : new_states) {
		for (const Symbol& symb : language->get_alphabet()) {
			if (!state.transitions.count(symb) || state.transitions.at(symb).empty()) {
				state.set_transition(
					{count, symb, Symbol::Epsilon, std::vector<Symbol>{Symbol::Epsilon}}, symb);
				has_trap = true;
			}
		}
	}


	if (has_trap) {
		new_states.emplace_back(count, "", false);
		// meta.upd(NodeMeta{count, MetaInfo::trap_color});
		for (const Symbol& symb : language->get_alphabet()) {
			new_states[count].set_transition(
				{count, symb, Symbol::Epsilon, std::vector<Symbol>{Symbol::Epsilon}}, symb);
		}
	}

	PushdownAutomaton result(initial_state, new_states, language);
	return {meta, result};
}

void invert_all_states(std::vector<PDAState>& states) {
	for (auto& state : states) {
		state.is_terminal = !state.is_terminal;
	}
}

bool has_final_state(const std::vector<PDAState>& states) {
	for (const auto& state : states) {
		if (state.is_terminal) {
			return true;
		}
	}
	return false;
}

PushdownAutomaton PushdownAutomaton::complement(iLogTemplate* log) const {
	PushdownAutomaton new_pda(initial_state, states, language);
	auto [meta, result] = new_pda._add_trap_state(log);
	for (auto&state : result.states) {
		state.is_terminal = !state.is_terminal;
	}

	if(log) {
		log->set_parameter("oldpda", *this);
		log->set_parameter("result", result, meta);
	}

	return result;
	// if (!has_final_state(result.states)) {
	// 	invert_all_states(result.states);
	// 	return result;
	// }
	//
	// std::set<int> reading_states;
	// std::map<int, std::vector<PDATransition>> new_transitions;
	// for (auto& state : result.states) {
	// 	bool has_input = false, has_input_eps = false;
	// 	for (const auto& [symbol, symbol_transitions] : state.transitions) {
	// 		for (const auto& trans : symbol_transitions) {
	// 			if (trans.input_symbol.is_epsilon()) {
	// 				has_input_eps = true;
	// 			} else {
	// 				has_input = true;
	// 			}
	// 		}
	// 	}
	//
	// 	if (!has_input_eps) {
	// 		reading_states.insert(state.index);
	// 	}
	// 	if (!has_input) {
	// 		continue;
	// 	}
	//
	// 	std::map<Symbol, int> stack_sym_to_state;
	// 	for (const auto& [symbol, symbol_transitions] : state.transitions) {
	// 		for (const auto& trans : symbol_transitions) {
	// 			if (!trans.input_symbol.is_epsilon()) {
	// 				auto it = stack_sym_to_state.find(trans.pop);
	// 				if (it == stack_sym_to_state.end()) {
	// 					it = stack_sym_to_state.try_emplace(trans.pop).first;
	// 					state.set_transition({it->second, Symbol::Epsilon, trans.pop, std::vector<Symbol>({Symbol::Epsilon})}, Symbol::Epsilon);
	// 					reading_states.insert(it->second);
	// 					if(state.is_terminal) {
	// 						result.states[it->second].is_terminal = true;
	// 					}
	// 				}
	//
	// 				new_transitions[it->second].emplace_back(trans.to, trans.input_symbol, Symbol::Epsilon, trans.push);
	// 			}
	// 		}
	// 	}
	// }
	//
	// for (auto&[state_index, transitions_to_add]: new_transitions) {
	// 	for (auto trans: transitions_to_add) {
	// 		result.states[state_index].set_transition(trans, trans.input_symbol);
	// 	}
	// }
	//
	// for(auto& state: result.states) {
	// 	if (reading_states.count(state.index) && !state.is_terminal) {
	// 		state.is_terminal = true;
	// 	} else {
	// 		state.is_terminal = false;
	// 	}
	// }

	return new_pda;
}

bool PushdownAutomaton::equal(PushdownAutomaton pda1, PushdownAutomaton pda2) {
	if(pda1.size() != pda2.size()) {
		return false;
	}


}

std::vector<PDATransition> get_regular_transitions(const string& s,
												   const ParsingState& parsing_state) {
	std::vector<PDATransition> regular_transitions;
	const auto& transitions = parsing_state.state->transitions;

	const Symbol symb(parsing_state.pos < s.size() ? s[parsing_state.pos] : char());
	if (transitions.find(symb) == transitions.end()) {
		return regular_transitions;
	}

	auto symbol_transitions = transitions.at(symb);
	for (const auto& trans : symbol_transitions) {
		if (trans.pop.is_epsilon() || (!parsing_state.stack.empty() && parsing_state.stack.top() == trans.pop)) {
			regular_transitions.emplace_back(trans);
		}
	}

	return regular_transitions;
}

std::vector<PDATransition> get_epsilon_transitions(const string& s,
												   const ParsingState& parsing_state) {
	std::vector<PDATransition> epsilon_transitions;
	const auto& transitions = parsing_state.state->transitions;

	if (transitions.find(Symbol::Epsilon) == transitions.end()) {
		return epsilon_transitions;
	}

	for (const auto& trans : transitions.at(Symbol::Epsilon)) {
		// переход по epsilon -> не потребляем символ
		if (trans.pop.is_epsilon() ||(!parsing_state.stack.empty() && parsing_state.stack.top() == trans.pop)) {
			epsilon_transitions.emplace_back(trans);
		}
	}

	return epsilon_transitions;
}

std::stack<Symbol> perform_stack_actions(std::stack<Symbol> stack, const PDATransition& tr) {
	auto result = stack;

	if(!tr.pop.is_epsilon()) {
		result.pop();
	}

	for (const auto& push_sym : tr.push) {
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
	pda_stack.emplace(Symbol::StackTop);
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
			for (const auto& trans : transitions) {
				parsing_stack.emplace(parsed_len + 1,
									  &states[trans.to],
									  perform_stack_actions(parsing_state.stack, trans));
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
		for (const auto& trans : eps_transitions) {
			if (!visited_eps.count({parsed_len, state->index, trans.to})) {
				parsing_stack.emplace(parsed_len, &states[trans.to], perform_stack_actions(parsing_state.stack, trans));
				visited_eps.insert({parsed_len, state->index, trans.to});
			}
		}
	}

	if (s.size() == parsed_len && state->is_terminal) {
		return {counter, true};
	}

	return {counter, false};
}