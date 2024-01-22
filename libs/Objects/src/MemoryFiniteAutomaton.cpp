#include <algorithm>
#include <sstream>
#include <stack>
#include <utility>

#include "Objects/FiniteAutomaton.h"
#include "Objects/Language.h"
#include "Objects/iLogTemplate.h"

using std::map;
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

MFATransition::MFATransition(int to) : to(to) {}

MFATransition::MFATransition(int to, MemoryActions memory_actions)
	: to(to), memory_actions(std::move(memory_actions)) {}

MFATransition::MFATransition(int to, const unordered_set<int>& opens,
							 const unordered_set<int>& closes)
	: MFATransition(to) {
	for (auto num : opens)
		memory_actions[num] = MFATransition::open;
	for (auto num : closes) {
		if (memory_actions.count(num))
			std::cerr << "!!! Конфликт действий с ячейкой памяти !!!";
		memory_actions[num] = MFATransition::close;
	}
}

MFATransition::MFATransition(int to, const unordered_set<int>& destination_first,
							 const unordered_set<int>& source_in_cells,
							 const unordered_set<int>& iteration_over_cells,
							 const unordered_set<int>& source_last,
							 const unordered_set<int>& destination_in_cells)
	: MFATransition(to) {
	for (auto num : destination_first) {
		if (source_in_cells.count(num) && !iteration_over_cells.count(num))
			continue;
		memory_actions[num] = MFATransition::open;
	}
	for (auto num : source_last) {
		if (destination_in_cells.count(num))
			continue;
		memory_actions[num] = MFATransition::close;
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

MFAState::MFAState(int index, std::string identifier, bool is_terminal)
	: State::State(index, std::move(identifier), is_terminal) {}

MFAState::MFAState(int index, std::string identifier, bool is_terminal,
				   MFAState::Transitions transitions)
	: State::State(index, std::move(identifier), is_terminal), transitions(std::move(transitions)) {
}

MFAState::MFAState(FAState state) : State::State(state.index, state.identifier, state.is_terminal) {
	for (const auto& [symbol, states_to] : state.transitions)
		for (auto to : states_to)
			transitions[symbol].insert(MFATransition(to));
}

void MFAState::set_transition(const MFATransition& to, const Symbol& symbol) {
	transitions[symbol].insert(to);
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

	for (const auto& [num, action] : memory_actions) {
		switch (action) {
		case MFATransition::open:
			opens.insert(num);
			break;
		case MFATransition::close:
			closes.insert(num);
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
		for (const auto& elem : state.transitions) {
			for (const auto& transition : elem.second) {
				ss << "\t" << state.index << " -> " << transition.to << " [label = \""
				   << string(elem.first) << transition.get_actions_str() << "\"]\n";
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
		for (const auto& [symbol, states_to] : state.transitions) {
			if ((symbol.is_epsilon() || symbol.is_ref()) && state.transitions.size() > 1) {
				result = false;
				break;
			}
			if (states_to.size() > 1) {
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
				state.set_transition(MFATransition(count), symb);
				new_meta.upd(EdgeMeta{state.index, count, symb, MetaInfo::trap_color});
				add_trap = true;
			}
		}
	}

	if (add_trap) {
		new_states.emplace_back(count, "", false);
		for (const Symbol& symb : language->get_alphabet()) {
			new_states[count].set_transition(MFATransition(count), symb);
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
		states[0].set_transition(MFATransition(0), symb);
	}

	return {0, states, language};
}

void MemoryFiniteAutomaton::dfs_by_eps(
	int index, set<int>& reachable,
	MFATransition::MemoryActions& memory_actions_composition) const {
	if (!reachable.count(index)) {
		reachable.insert(index);
		const auto& by_eps = states[index].transitions.find(Symbol::Epsilon);
		if (by_eps != states[index].transitions.end()) {
			if (by_eps->second.size() > 1)
				throw std::logic_error(
					"dfs_by_eps: trying to make a composition of parallel eps-transitions");
			for (const auto& transition : by_eps->second) {
				for (auto [num, action] : transition.memory_actions)
					memory_actions_composition[num] = action;
				dfs_by_eps(transition.to, reachable, memory_actions_composition);
			}
		}
	}
}

tuple<set<int>, MFATransition::MemoryActions> MemoryFiniteAutomaton::get_eps_closure(
	const set<int>& indices) const {
	set<int> reachable;
	MFATransition::MemoryActions memory_actions_composition;
	for (int index : indices)
		dfs_by_eps(index, reachable, memory_actions_composition);
	return {reachable, memory_actions_composition};
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
		for (const auto& [symbol, states_to] : state.transitions)
			if (!symbol.is_epsilon())
				unique_symbols.insert(symbol);

	vector<MFAState> new_states;
	unordered_map<set<int>, int, Hasher> state_number_by_closure;
	auto [initial_closure, initial_memory_actions] = get_eps_closure({initial_state});
	state_number_by_closure[initial_closure] = 0;
	new_states.emplace_back(0, get_identifier(initial_closure), false);

	stack<pair<set<int>, MFATransition::MemoryActions>> s;
	s.emplace(initial_closure, initial_memory_actions);
	int states_counter = 1;
	while (!s.empty()) {
		auto [from_closure, prev_memory_actions] = s.top();
		s.pop();
		for (const auto& symb : unique_symbols) {
			optional<MFATransition> transition;
			for (int i : from_closure) {
				auto transitions_by_symbol = states[i].transitions.find(symb);
				if (transitions_by_symbol != states[i].transitions.end()) {
					if (transitions_by_symbol->second.size() > 1 || transition.has_value())
						throw std::logic_error("remove_eps: trying to make a composition of "
											   "parallel transitions by symb");
					if (transitions_by_symbol->second.size() == 1) {
						transition = *transitions_by_symbol->second.begin();
					}
				}
			}
			if (transition.has_value()) {
				auto [cur_closure, closure_memory_actions] = get_eps_closure({transition->to});
				if (!cur_closure.empty()) {
					if (!state_number_by_closure.count(cur_closure)) {
						MFAState new_state(states_counter, get_identifier(cur_closure), false);
						new_states.push_back(new_state);
						state_number_by_closure[cur_closure] = states_counter;
						s.emplace(cur_closure, closure_memory_actions);
						states_counter++;
					}
					for (auto [num, action] : prev_memory_actions)
						transition->memory_actions[num] = action;
					new_states[state_number_by_closure[from_closure]].set_transition(
						MFATransition(state_number_by_closure[cur_closure],
									  transition->memory_actions),
						symb);
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
	new_mfa = new_mfa.remove_eps().add_trap_state();
	int final_states_counter = 0;
	for (int i = 0; i < new_mfa.size(); i++) {
		new_mfa.states[i].is_terminal = !new_mfa.states[i].is_terminal;
		if (new_mfa.states[i].is_terminal)
			final_states_counter++;
	}
	if (!final_states_counter)
		new_mfa = MemoryFiniteAutomaton::get_just_one_total_trap(new_mfa.language);

	if (log) {
		//		log->set_parameter("oldautomaton", *this);
		//		log->set_parameter("result", new_mfa);
	}
	return new_mfa;
}

ParingState::ParingState(int pos, const MFAState* state,
						 const std::unordered_set<int>& opened_cells,
						 const std::unordered_map<int, std::pair<int, int>>& memory)
	: pos(pos), state(state), opened_cells(opened_cells), memory(memory) {}

Matcher::Matcher(const string& s) : s(&s) {}

// пара {переходы по символам-буквам/непустым символам-ссылкам, переходы по эпсилон/пустым ссылкам}
pair<MFAState::Transitions, MFAState::Transitions> get_transitions(const string& s,
																   const ParingState& parsing_state,
																   Matcher* matcher) {
	MFAState::Transitions transitions, eps_transitions;
	// тройки {символ-ссылка, начало подстроки, конец подстроки}
	vector<tuple<Symbol, int, int>> refs_to_match;
	for (const auto& [symb, states_to] : parsing_state.state->transitions) {
		if (symb.is_ref()) {
			if (!parsing_state.memory.count(symb.get_ref())) {
				// пустая ссылка добавляется к eps-переходам
				eps_transitions[symb] = states_to;
				continue;
			}
			const auto& [l, r] = parsing_state.memory.at(symb.get_ref());
			if (l == r) {
				// пустая ссылка добавляется к eps-переходам
				eps_transitions[symb] = states_to;
				continue;
			}
			if (r - l <= s.size() - parsing_state.pos) {
				refs_to_match.emplace_back(symb, l, r);
			}
		} else if (symb == s[parsing_state.pos]) {
			transitions[symb] = states_to;
		} else if (symb.is_epsilon()) {
			eps_transitions[symb] = states_to;
		}
	}

	if (!refs_to_match.empty())
		matcher->match(parsing_state, transitions, refs_to_match);

	return {transitions, eps_transitions};
}

// для символа-буквы - 1, для символа-ссылки - длина содержимого памяти
int get_symbol_len(const unordered_map<int, pair<int, int>>& memory, const Symbol& symbol) {
	if (symbol.is_ref()) {
		pair<int, int> substr = memory.at(symbol.get_ref());
		return substr.second - substr.first;
	} else if (!symbol.is_epsilon()) {
		return 1;
	}
	return 0;
}

pair<unordered_set<int>, unordered_map<int, pair<int, int>>> update_memory(
	unordered_set<int> opened_cells, unordered_map<int, pair<int, int>> memory,
	const Symbol& symbol, const MFATransition::MemoryActions& memory_actions, int pos) {
	for (const auto [num, action] : memory_actions) {
		if (action == MFATransition::MemoryAction::open) {
			opened_cells.insert(num);
			memory[num].first = pos;
			memory[num].second = pos;
		} else {
			if (!opened_cells.count(num))
				std::cerr << "Cell is already closed\n";
			opened_cells.erase(num);
		}
	}

	for (auto num : opened_cells) {
		memory[num].second += get_symbol_len(memory, symbol);
	}

	return {opened_cells, memory};
}

std::pair<int, bool> MemoryFiniteAutomaton::_parse(const string& s, Matcher* matcher) const {
	stack<ParingState> parsing_states_stack;
	// тройка (актуальный индекс элемента в строке, начало эпсилон-перехода, конец эпсилон-перехода)
	set<tuple<int, int, int>> visited_eps;
	int counter = 0;
	int parsed_len = 0;
	const MFAState* state = &states[initial_state];
	parsing_states_stack.emplace(
		parsed_len, state, unordered_set<int>({}), unordered_map<int, pair<int, int>>({}));
	while (!parsing_states_stack.empty()) {
		if (state->is_terminal && parsed_len == s.size()) {
			break;
		}
		ParingState parsing_state = parsing_states_stack.top();
		state = parsing_state.state;
		parsed_len = parsing_state.pos;
		parsing_states_stack.pop();
		counter++;
		// состояния достижимые по символам-буквам/символам-ссылкам
		// и состояния достижимые по эпсилон-переходам/пустым ссылкам
		auto [reach, reach_eps] = get_transitions(s, parsing_state, matcher);

		// переходы в новые состояния по букве/непустой ссылке
		if (parsed_len + 1 <= s.size()) {
			for (const auto& [symbol, states_to] : reach) {
				for (const auto& to : states_to) {
					auto [new_opened_cells, new_memory] = update_memory(parsing_state.opened_cells,
																		parsing_state.memory,
																		symbol,
																		to.memory_actions,
																		parsed_len);
					parsing_states_stack.emplace(parsed_len + get_symbol_len(new_memory, symbol),
												 &states[to.to],
												 new_opened_cells,
												 new_memory);
				}
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
		for (const auto& [symb, states_to] : reach_eps) {
			for (const auto& eps_tr : states_to) {
				if (!visited_eps.count({parsed_len, state->index, eps_tr.to})) {
					auto [new_opened_cells, new_memory] = update_memory(parsing_state.opened_cells,
																		parsing_state.memory,
																		Symbol::Epsilon,
																		eps_tr.memory_actions,
																		parsed_len);
					parsing_states_stack.emplace(
						parsed_len, &states[eps_tr.to], new_opened_cells, new_memory);
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

class BasicMatcher : public Matcher {
  public:
	explicit BasicMatcher(const string&);

	// сопоставление за линию для каждой ячейки
	void match(const ParingState&, MFAState::Transitions&, // NOLINT(runtime/references)
			   vector<tuple<Symbol, int, int>>&) override; // NOLINT(runtime/references)
};

BasicMatcher::BasicMatcher(const string& s) : Matcher(s) {}

void BasicMatcher::match(const ParingState& parsing_state, MFAState::Transitions& transitions,
						 vector<tuple<Symbol, int, int>>& refs_to_match) {
	for (int i = parsing_state.pos; i <= s->size(); i++) {
		if (refs_to_match.empty())
			break;
		for (auto it = refs_to_match.begin(); it != refs_to_match.end();) {
			const auto& [symbol, l, r] = *it;
			if (l == parsing_state.pos)
				std::cerr << "Trying to match ref to current position\n";
			int index = l + i - parsing_state.pos;
			if (index == r) {
				transitions[symbol] = parsing_state.state->transitions.at(symbol);
				it = refs_to_match.erase(it);
			} else if ((*s)[index] != (*s)[i]) {
				it = refs_to_match.erase(it);
			} else {
				++it;
			}
		}
	}
}

std::pair<int, bool> MemoryFiniteAutomaton::parse(const string& s) const {
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
	void match(const ParingState&, MFAState::Transitions&, // NOLINT(runtime/references)
			   vector<tuple<Symbol, int, int>>&) override; // NOLINT(runtime/references)
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

void FastMatcher::match(const ParingState& parsing_state, MFAState::Transitions& transitions,
						vector<tuple<Symbol, int, int>>& refs_to_match) {
	for (const auto& [symbol, l, r] : refs_to_match) {
		if (l == parsing_state.pos)
			std::cerr << "Trying to match ref to current position\n";
		if (query_sparse_table(std::min(inv_sa[l], inv_sa[parsing_state.pos]),
							   std::max(inv_sa[l], inv_sa[parsing_state.pos]) - 1) >= r - l)
			transitions[symbol] = parsing_state.state->transitions.at(symbol);
	}
}

std::pair<int, bool> MemoryFiniteAutomaton::parse_additional(const string& s) const {
	FastMatcher matcher(s);
	return _parse(s, &matcher);
}
