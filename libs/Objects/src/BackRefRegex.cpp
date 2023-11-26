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
	value = tmp->value;
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

string BackRefRegex::to_txt(bool eps_is_empty) const {
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
		symb = value.symbol;
		break;
	}

	return str1 + symb + str2;
}

string BackRefRegex::type_to_str() const {
	if (value.symbol != "")
		return value.symbol;
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
	p->value = lexemes[index_start];
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
	p->value = lexemes[index_start];
	p->type = AlgExpression::memoryWriter;
	p->cell_number = lexemes[index_start].number;
	p->alphabet = l->alphabet;
	return p;
}

vector<BackRefRegex*> BackRefRegex::pre_order_travers(unordered_set<int> cell_numbers) {
	vector<BackRefRegex*> res;
	if (type == AlgExpression::symb || type == AlgExpression::ref) {
		in_cells = cell_numbers;
		res.push_back(this);
		return res;
	}

	if (type == AlgExpression::memoryWriter)
		cell_numbers.insert(cell_number);

	if (term_l) {
		vector<BackRefRegex*> l = cast(term_l)->pre_order_travers(cell_numbers);
		res.insert(res.end(), l.begin(), l.end());
	}

	if (term_r) {
		vector<BackRefRegex*> r = cast(term_r)->pre_order_travers(cell_numbers);
		res.insert(res.end(), r.begin(), r.end());
	}

	return res;
}

pair<unordered_set<int>, unordered_set<int>> set_difference(const std::unordered_set<int>& set1,
															const std::unordered_set<int>& set2) {

	std::unordered_set<int> difference_set1;
	std::unordered_set<int> difference_set2;

	for (const auto& i : set1) {
		if (set2.find(i) == set2.end()) {
			difference_set1.insert(i);
		}
	}

	for (const auto& i : set2) {
		if (set1.find(i) == set1.end()) {
			difference_set2.insert(i);
		}
	}

	return {difference_set1, difference_set2};
}

MemoryFiniteAutomaton BackRefRegex::to_mfa(iLogTemplate* log) const {
	BackRefRegex temp_copy(*this);
	vector<BackRefRegex*> symbols = temp_copy.pre_order_travers({});
	for (size_t i = 0; i < symbols.size(); i++) {
		symbols[i]->value.number = i;
		symbols[i]->value.symbol.linearize(i);
	}
	// Множество начальных состояний
	vector<BackRefRegex*> first = cast(temp_copy.get_first_nodes());
	// Множество конечных состояний
	vector<BackRefRegex*> last = cast(temp_copy.get_last_nodes());
	// множество состояний, которым предшествует key-int
	unordered_map<int, vector<int>> following_states = temp_copy.pairs();
	int eps_in = this->contains_eps();
	vector<MemoryFiniteAutomaton::State> states; // Список состояний в автомате

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

	for (const auto& i : following_states) {
		for (auto& to : i.second) {
			str_pair = str_pair + "(" + string(symbols[i.first]->value.symbol) + "," +
					   string(symbols[to]->value.symbol) + ")" + "\\ ";
		}
	}

	vector<string> linearized_symbols;
	for (auto& i : symbols) {
		linearized_symbols.push_back(i->value.symbol);
		i->value.symbol.delinearize();
	}

	MemoryFiniteAutomaton::Transitions start_state_transitions;
	for (auto& i : first) {
		start_state_transitions[i->value.symbol].emplace_back(i->value.number + 1, i->in_cells);
	}

	if (eps_in) {
		states.emplace_back(0, "S", true, start_state_transitions);
	} else {
		states.emplace_back(0, "S", false, start_state_transitions);
	}

	unordered_set<int> last_lexemes;
	for (auto& i : last) {
		last_lexemes.insert(i->value.number);
	}

	for (size_t i = 0; i < symbols.size(); i++) {
		Lexeme lexeme = symbols[i]->value;
		MemoryFiniteAutomaton::Transitions transitions;

		for (int j : following_states[lexeme.number]) {
			transitions[symbols[j]->value.symbol].emplace_back(
				j + 1, set_difference(symbols[j]->in_cells, symbols[i]->in_cells));
		}
		string id_str = linearized_symbols[i];

		// В last_lexemes номера конечных лексем => last_lexemes.count проверяет есть ли
		// номер лексемы в списке конечных лексем (является ли состояние конечным)
		states.emplace_back(i + 1, id_str, last_lexemes.count(lexeme.number), transitions);
	}

	MemoryFiniteAutomaton mfa(0, states, language);
	if (log) {
	}

	return mfa;
}