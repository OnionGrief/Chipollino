#include "Objects/FiniteAutomaton.h"
#include "Objects/Language.h"
#include "Objects/iLogTemplate.h"
#include <Objects/BackRefRegex.h>
#include <Objects/PushdownAutomaton.h>
#include <Objects/Symbol.h>
#include <sstream>
#include <string>

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
	hash_combine(hash, t.to);
	hash_combine(hash, string(t.input_symbol));
	hash_combine(hash, string(t.pop));
	for (const auto& push_symb : t.push) {
		hash_combine(hash, string(push_symb));
	}
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
	std::unordered_map<int, int> recover_index;
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

	return {new_initial_index, new_states, get_language()};
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
	std::unordered_map<std::pair<int, int>, int, PairHasher> recover_index;
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
	Alphabet result_alphabet;
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

vector<PDATransition> _flatten_transitions(PDAState state) {
	vector<PDATransition> result;

	for (const auto& [symbol, symbol_transitions] : state.transitions) {
		for (const auto& trans : symbol_transitions) {
			result.emplace_back(trans);
		}
	}

	return result;
}

pair<bool, EqualityCheckerState> PushdownAutomaton::_equality_dfs(
	const PushdownAutomaton& pda1, const PushdownAutomaton& pda2, EqualityCheckerState chst) const {

	if (!chst.can_map_states(chst.index1, chst.index2) ||
		pda1.states[chst.index1].is_terminal != pda2.states[chst.index2].is_terminal) {
		return {false, {}};
	} else if (chst.states_mapping.count(chst.index1) && chst.states_mapping[chst.index1] == chst.index2) {
		return {true, chst};
	}
	chst.map_states(chst.index1, chst.index2);

	const auto state1 = pda1.states[chst.index1];
	const auto state2 = pda2.states[chst.index2];

	if (state1.transitions.size() != state2.transitions.size()) {
		// Два состояния должны иметь переходы по равному количеству символов.
		return {false, {}};
	}

	auto transitions1 = _flatten_transitions(state1);
	auto transitions2 = _flatten_transitions(state2);
	if (transitions1.size() != transitions2.size()) {
		return {false, {}};
	}

	struct st {
		int rem1_index;
		vector<PDATransition> rem1, rem2;
		EqualityCheckerState chst;
	};

	std::queue<st> q;
	q.push({0, transitions1, transitions2, chst});
	while (!q.empty()) {
		auto data = q.front();
		q.pop();

		if (data.rem1_index >= data.rem1.size()) {
			return {true, data.chst};
		}

		auto trans1 = data.rem1[data.rem1_index];
		for (int i = 0; i<data.rem2.size(); i++) {
			auto chst_copy = data.chst;
			auto trans2 = data.rem2[i];

			if(trans1.input_symbol != trans2.input_symbol) {
				continue;
			}

			// Проверяем возможность сопоставить pop символы
			if (!chst_copy.can_map_stack(trans1.pop, trans2.pop))
				continue;
			chst_copy.map_stack(trans1.pop, trans2.pop);

			// Проверяем возможность сопоставить push символы
			if (trans1.push.size() != trans2.push.size())
				continue;

			bool is_push_mapped = true;
			for (int i = 0; i < trans1.push.size(); i++) {
				auto ss1 = trans1.push[i], ss2 = trans2.push[i];
				if (!chst_copy.can_map_stack(ss1, ss2)) {
					is_push_mapped = false;
					break;
				}
				chst_copy.map_stack(ss1, ss2);
			}
			if (!is_push_mapped)
				continue;

			auto [success, chst_new] = _equality_dfs(pda1, pda2, {trans1.to, trans2.to, chst_copy.stack_mapping, chst_copy.states_mapping});
			if (!success) {
				continue;
			}

			// Удалось сопоставить переход, пробуем раскрывать дальше
			data.rem2.erase(data.rem2.begin()+i);
			q.push({data.rem1_index+1, data.rem1, data.rem2, chst_new});
		}
	}

	return {false, {}};
}

pair<bool, unordered_map<Symbol, Symbol, Symbol::Hasher>> PushdownAutomaton::_equality_checker(PushdownAutomaton pda1, PushdownAutomaton pda2) {
	// Автоматы должны как минимум иметь равное количество состояний,
	// равное количество символов стэка и равные алфавиты.
	if (pda1.size() != pda2.size() ||
		pda1.get_language()->get_alphabet() != pda2.get_language()->get_alphabet() ||
		pda1._get_stack_symbols().size() != pda2._get_stack_symbols().size()) {
		return {false, {}};
	}

	const EqualityCheckerState chst(pda1.initial_state,
									pda2.initial_state,
									{{Symbol::StackTop, Symbol::StackTop}, {Symbol::Epsilon, Symbol::Epsilon}},
									{});
	const auto [success, chst_new] = pda1._equality_dfs(pda1, pda2, chst);
	if (!success) {
		return {false, {}};
	}

	return {true, chst_new.stack_mapping};
}

bool PushdownAutomaton::equal(PushdownAutomaton pda1, PushdownAutomaton pda2, iLogTemplate* log) {
	auto [result, stack_mapping] = _equality_checker(pda1, pda2);

	if (log) {
		log->set_parameter("pda1", pda1);
		log->set_parameter("pda2", pda2);
		log->set_parameter("result", result);
		if (!result) {
			log->set_parameter("stack", "{}");
		} else {
			std::ostringstream oss;
			for (auto [first, second]: stack_mapping) {
				oss << first << " -> " << second << "; ";
			}
			log->set_parameter("stack", oss.str());
		}
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

std::unordered_set<Symbol, Symbol::Hasher> PushdownAutomaton::_get_stack_symbols() const {
	std::unordered_set<Symbol, Symbol::Hasher> result;
	for (const auto& state : states) {
		for (const auto& [_, symbol_transitions] : state.transitions) {
			for (const auto& trans : symbol_transitions) {
				if (!trans.pop.is_epsilon()) {
					result.emplace(trans.pop);
				}
			}
		}
	}
	return result;
}

void PushdownAutomaton::_dfs(int index, unordered_set<int>& reachable) const {
	if (reachable.count(index)) {
		return;
	}
	reachable.insert(index);

	const auto& by_eps = states[index].transitions.find(Symbol::Epsilon);
	if (by_eps == states[index].transitions.end()) {
		return;
	}

	for (auto& trans_to : by_eps->second) {
		_dfs(trans_to.to, reachable);
	}
}

unordered_map<PDATransition, unordered_set<int>, PDATransition::Hasher> PushdownAutomaton::closure(
	const int index) const {
	unordered_map<PDATransition, unordered_set<int>, PDATransition::Hasher> result;

	auto state = states[index];
	if (state.transitions.find(Symbol::Epsilon) == state.transitions.end()) {
		return result;
	}

	for (const auto& eps_trans : state.transitions.at(Symbol::Epsilon)) {
		unordered_set<int> reachable;
		_dfs(eps_trans.to, reachable);
		result[eps_trans] = reachable;
	}

	return result;
}

std::unordered_map<int, std::unordered_set<PDATransition, PDATransition::Hasher>>
PushdownAutomaton::_find_problematic_epsilon_transitions() const {
	std::unordered_map<int, std::unordered_set<PDATransition, PDATransition::Hasher>> result;

	std::unordered_set<int> states_with_problematic_trans;
	for (const auto& state : states) {
		// Ищем нефинальные состояния, из которых есть помимо eps-переходов есть иные переходы.
		if (state.is_terminal ||
			state.transitions.find(Symbol::Epsilon) == state.transitions.end()) {
			continue;
		}

		// Отмечаем все eps-переходы в финальные состояния как проблемные.
		for (const auto& trans : state.transitions.at(Symbol::Epsilon)) {
			if (states[trans.to].is_terminal) {
				result[state.index].emplace(trans);
				states_with_problematic_trans.emplace(state.index);
			}
		}
	}

	for (const auto& state : states) {
		if (state.is_terminal ||
			state.transitions.find(Symbol::Epsilon) == state.transitions.end()) {
			continue;
		}

		auto reachable = closure({state.index});
		// Ищем нефинальные состояния, из которых есть помимо eps-переходов есть иные переходы.
		for (const auto& [eps_trans, indices] : reachable) {
			for (const auto& index : indices) {
				if (states[index].is_terminal) {
					result[state.index].emplace(eps_trans);
				}
			}
		}
	}

	return result;
}

std::vector<std::pair<int, PDATransition>> PushdownAutomaton::_find_transitions_to(
	int index) const {
	std::vector<std::pair<int, PDATransition>> result;

	for (const auto& state : states) {
		for (const auto& [symbol, symbol_transitions] : state.transitions) {
			for (const auto& trans : symbol_transitions) {
				if (trans.to == index) {
					result.emplace_back(state.index, trans);
				}
			}
		}
	}

	return result;
}

PushdownAutomaton PushdownAutomaton::_add_trap_state() {
	PushdownAutomaton result(initial_state, states, language);
	auto stack_symbols = result._get_stack_symbols();
	int i_trap = static_cast<int>(result.size());
	bool already_has_trap = false;
	for (const auto& state : result.states) {
		if (state.identifier == "trap") {
			already_has_trap = true;
			i_trap = state.index;
		}
	}

	bool need_create_trap = false;
	for (auto& state : result.states) {
		std::set<std::pair<Symbol, Symbol>> stack_sym_in_sym_transitions;

		for (const auto& [symbol, symbol_transitions] : state.transitions) {
			for (const auto& trans : symbol_transitions) {
				stack_sym_in_sym_transitions.emplace(trans.pop, trans.input_symbol);
			}
		}

		std::set<Symbol> symbols = result.get_language()->get_alphabet();

		for (const auto& symb : symbols) {
			for (const auto& stack_symb : stack_symbols) {
				if (stack_sym_in_sym_transitions.count({stack_symb, symb}) ||
					stack_sym_in_sym_transitions.count({stack_symb, Symbol::Epsilon})) {
					continue;
				}
				need_create_trap = true;
				state.set_transition({i_trap, symb, stack_symb, std::vector<Symbol>{stack_symb}},
									 symb);
			}
		}
	}

	if (!need_create_trap || already_has_trap) {
		return result;
	}

	result.states.emplace_back(i_trap, "trap", false);
	for (const auto& symb : result.get_language()->get_alphabet()) {
		if (symb.is_epsilon())
			continue;

		for (const auto& stack_symb : stack_symbols) {
			result.states[i_trap].set_transition(
				{i_trap, symb, stack_symb, std::vector<Symbol>{stack_symb}}, symb);
		}
	}

	return result;
}

PushdownAutomaton PushdownAutomaton::complement(iLogTemplate* log) const {
	// PDA here is deterministic.
	if (!is_deterministic()) {
		throw std::logic_error("Complement is available only for deterministic PDA");
	}

	PushdownAutomaton result(initial_state, states, language->get_alphabet());
	result = result._add_trap_state();

	std::set<int> no_toggle_states;
	auto problematic_transitions = result._find_problematic_epsilon_transitions();
	for (const auto& [from_index, bad_transitions] : problematic_transitions) {
		for (const auto& bad_trans : bad_transitions) {
			auto bad_symbol = bad_trans.pop;
			auto final_state_index = bad_trans.to;
			auto problems_trap_index = static_cast<int>(result.size());
			no_toggle_states.emplace(
				problems_trap_index); // Состояние-ловушка проблемных переходов не
			// меняет финальность при обращении

			for (const auto& [from_from_index, trans] : result._find_transitions_to(from_index)) {
				if (!trans.push.empty() && trans.push.back() == bad_symbol) {
					// Если после перехода на вершине стэка точно окажется "проблемный символ", то
					// перенаправляем переход сразу в финальное состояние.
					result.states[from_from_index].set_transition(
						{final_state_index, trans.input_symbol, trans.pop, trans.push},
						trans.input_symbol);
					result.states[from_from_index].transitions[trans.input_symbol].erase(trans);
					continue;
				}

				// Иначе перенаправляем переход в ловушку проблемных переходов
				result.states[from_from_index].set_transition(
					{problems_trap_index, trans.input_symbol, trans.pop, trans.push},
					trans.input_symbol);
				result.states[from_from_index].transitions[trans.input_symbol].erase(trans);
			}

			// Добавляем состояние-ловушку проблемных переходов.
			result.states.emplace_back(problems_trap_index, "eps-trap", false);
			result.states[problems_trap_index].set_transition(
				{final_state_index, Symbol::Epsilon, bad_symbol, std::vector({bad_symbol})},
				Symbol::Epsilon);
			for (const auto& stack_symb : result._get_stack_symbols()) {
				if (stack_symb != bad_symbol) {
					result.states[problems_trap_index].set_transition(
						{from_index, Symbol::Epsilon, stack_symb, std::vector({stack_symb})},
						Symbol::Epsilon);
				}
			}
		}
	}

	result = result._add_trap_state();
	for (auto& state : result.states) {
		if (!no_toggle_states.count(state.index)) {
			state.is_terminal = !state.is_terminal;
		}
	}

	if (log) {
		log->set_parameter("oldpda", *this);
		log->set_parameter("result", result);
	}
	return result;
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
		if (!parsing_state.stack.empty() && parsing_state.stack.top() == trans.pop) {
			regular_transitions.emplace_back(trans);
		}
	}

	return regular_transitions;
}

std::vector<PDATransition> get_epsilon_transitions(const ParsingState& parsing_state) {
	std::vector<PDATransition> epsilon_transitions;
	const auto& transitions = parsing_state.state->transitions;

	if (transitions.find(Symbol::Epsilon) == transitions.end()) {
		return epsilon_transitions;
	}

	for (const auto& trans : transitions.at(Symbol::Epsilon)) {
		// переход по epsilon -> не потребляем символ
		if (!parsing_state.stack.empty() && parsing_state.stack.top() == trans.pop) {
			epsilon_transitions.emplace_back(trans);
		}
	}

	return epsilon_transitions;
}

std::stack<Symbol> perform_stack_actions(std::stack<Symbol> stack, const PDATransition& tr) {
	stack.pop();

	for (const auto& push_sym : tr.push) {
		if (!push_sym.is_epsilon()) {
			stack.push(push_sym);
		}
	}
	return stack;
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
		auto eps_transitions = get_epsilon_transitions(parsing_state);
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