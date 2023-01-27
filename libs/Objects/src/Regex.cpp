#include "Objects/Regex.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/Language.h"
#include "Objects/iLogTemplate.h"
#include <set>

Regex::Lexem::Lexem(Type type, const alphabet_symbol& symbol, int number)
	: type(type), symbol(symbol), number(number) {}

vector<Regex::Lexem> Regex::parse_string(string str) {
	vector<Regex::Lexem> lexems;
	lexems = {};
	bool flag_alt = false;
	bool regex_is_eps = true;
	auto is_symbol = [](char c) {
		return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
	};
	// int index = 0;
	// for (const char& c : str) {
	for (size_t index = 0; index < str.size(); index++) {
		char c = str[index];
		Regex::Lexem lexem;
		switch (c) {
		case '(':
			lexem.type = Regex::Lexem::parL;
			flag_alt = true;
			break;
		case ')':
			lexem.type = Regex::Lexem::parR;
			if ((index != 0) &&
				(flag_alt || lexems.back().type == Regex::Lexem::parL)) {
				lexem.type = Regex::Lexem::error;
				lexems = {};
				lexems.push_back(lexem);
				return lexems;
			} else if (index == 0) {
				lexem.type = Regex::Lexem::error;
				lexems = {};
				lexems.push_back(lexem);
				return lexems;
			}
			break;
		case '|':
			lexem.type = Regex::Lexem::alt;
			break;
		case '*':
			if ((index != 0) && (lexems.back().type == Regex::Lexem::star ||
								 lexems.back().type == Regex::Lexem::alt)) {
				lexem.type = Regex::Lexem::error;
				lexems = {};
				lexems.push_back(lexem);
				return lexems;
			} else if (index == 0) {
				lexem.type = Regex::Lexem::error;
				lexems = {};
				lexems.push_back(lexem);
				return lexems;
			}
			lexem.type = Regex::Lexem::star;
			break;
		default:
			if (is_symbol(c)) {
				regex_is_eps = false;
				lexem.type = Regex::Lexem::symb;
				lexem.symbol = c;
				for (size_t j = index + 1; j < str.size(); j++) {
					bool lin = false;
					bool annote = false;
					if (str[j] == alphabet_symbol::linearize_marker) {
						lin = true;
					}
					if (str[j] == alphabet_symbol::annote_marker) {
						annote = true;
					}
					if (!lin && !annote) {
						break;
					}
					string number;
					for (size_t i = j + 1; i < str.size(); i++) {
						if (str[i] >= '0' && str[i] <= '9') {
							number += str[i];
						} else {
							break;
						}
						index = i;
						j = i;
					}
					if (number.length() == 0) {
						lexem.type = Regex::Lexem::error;
						lexems = {};
						lexems.push_back(lexem);
						return lexems;
					}
					int numb = stoi(number);
					if (lin) {
						lexem.symbol.linearize(numb);
					}
					if (annote) {
						lexem.symbol.annote(numb);
					}
				}
				flag_alt = false;

			} else {
				lexem.type = Regex::Lexem::error;
				lexems = {};
				lexems.push_back(lexem);
				return lexems;
			}
			break;
		}

		if (lexems.size() &&
			(
				// Regex::Lexem left
				lexems.back().type == Regex::Lexem::symb ||
				lexems.back().type == Regex::Lexem::star ||
				lexems.back().type == Regex::Lexem::parR) &&
			(
				// Regex::Lexem right
				lexem.type == Regex::Lexem::symb ||
				lexem.type == Regex::Lexem::parL)) {

			// We place . between
			lexems.push_back({Regex::Lexem::conc});
		}

		if (lexems.size() && ((lexems.back().type == Regex::Lexem::parL &&
							   (lexem.type == Regex::Lexem::parR ||
								lexem.type == Regex::Lexem::alt)) ||
							  (lexems.back().type == Regex::Lexem::alt &&
							   lexem.type == Regex::Lexem::parR) ||
							  (lexems.back().type == Regex::Lexem::alt &&
							   lexem.type == Regex::Lexem::alt))) {
			//  We place eps between
			lexems.push_back({Regex::Lexem::eps});
		}

		lexems.push_back(lexem);
	}
	if (regex_is_eps) {
		lexems = {};
		lexems.push_back({Regex::Lexem::error});
		return lexems;
	}

	if (lexems.size() && lexems[0].type == Regex::Lexem::alt) {
		lexems.insert(lexems.begin(), {Regex::Lexem::eps});
	}

	if (lexems.back().type == Regex::Lexem::alt) {
		lexems.push_back({Regex::Lexem::eps});
	}

	int balance = 0;
	for (size_t i = 0; i < lexems.size(); i++) {
		if (lexems[i].type == Regex::Lexem::parL) {
			balance++;
		}
		if (lexems[i].type == Regex::Lexem::parR) {
			balance--;
		}
	}

	if (balance != 0) {
		lexems = {};
		lexems.push_back({Regex::Lexem::error});
		return lexems;
	}

	return lexems;
}

Regex* Regex::scan_conc(const vector<Regex::Lexem>& lexems, int index_start,
						int index_end) {
	Regex* p = nullptr;
	int balance = 0;
	for (int i = index_start; i < index_end; i++) {
		if (lexems[i].type == Regex::Lexem::parL) { // LEFT_BRACKET
			balance++;
		}
		if (lexems[i].type == Regex::Lexem::parR) { // RIGHT_BRACKET
			balance--;
		}
		if (lexems[i].type == Regex::Lexem::conc && balance == 0) {
			Regex* l = expr(lexems, index_start, i);
			Regex* r = expr(lexems, i + 1, index_end);

			if (l == nullptr || r == nullptr || r->type == Regex::eps ||
				l->type == Regex::eps) { // Проверка на адекватность)
				if (r != nullptr) {
					delete r;
				}
				if (l != nullptr) {
					delete l;
				}
				return p;
			}

			p = new Regex;
			p->term_l = l;
			p->term_r = r;
			p->value = lexems[i];
			p->type = Regex::conc;

			set<alphabet_symbol> s = l->alphabet;
			s.insert(r->alphabet.begin(), r->alphabet.end());
			p->alphabet = s;
			return p;
		}
	}
	return nullptr;
}

Regex* Regex::scan_star(const vector<Regex::Lexem>& lexems, int index_start,
						int index_end) {
	Regex* p = nullptr;
	int balance = 0;
	for (int i = index_start; i < index_end; i++) {
		if (lexems[i].type == Regex::Lexem::parL) { // LEFT_BRACKET
			balance++;
		}
		if (lexems[i].type == Regex::Lexem::parR) { // RIGHT_BRACKET
			balance--;
		}
		if (lexems[i].type == Regex::Lexem::star && balance == 0) {
			Regex* l = expr(lexems, index_start, i);

			if (l == nullptr || l->type == Regex::eps) {
				if (l != nullptr) {
					delete l;
				}
				return p;
			}

			p = new Regex;
			p->term_l = l;

			p->term_r = nullptr;
			p->value = lexems[i];
			p->type = Regex::star;

			set<alphabet_symbol> s = l->alphabet;
			p->alphabet = s;
			return p;
		}
	}
	return nullptr;
}

Regex* Regex::scan_alt(const vector<Regex::Lexem>& lexems, int index_start,
					   int index_end) {
	Regex* p = nullptr;
	int balance = 0;
	for (int i = index_start; i < index_end; i++) {
		if (lexems[i].type == Regex::Lexem::parL) { // LEFT_BRACKET
			balance++;
		}
		if (lexems[i].type == Regex::Lexem::parR) { // RIGHT_BRACKET
			balance--;
		}
		if (lexems[i].type == Regex::Lexem::alt && balance == 0) {
			Regex* l = expr(lexems, index_start, i);
			Regex* r = expr(lexems, i + 1, index_end);
			// cout << l->type << " " << r->type << "\n";
			if (((l == nullptr) || (r == nullptr))/* ||
				(l->type == Regex::eps &&
				 r->type == Regex::eps)*/) { // Проверка на адекватность)

				if (r != nullptr) delete r;
				if (l != nullptr) delete l;
				return nullptr;
			}

			p = new Regex;
			p->term_l = l;
			p->term_r = r;

			p->value = lexems[i];
			p->type = Regex::alt;

			set<alphabet_symbol> s = l->alphabet;
			s.insert(r->alphabet.begin(), r->alphabet.end());

			p->alphabet = s;
			return p;
		}
	}
	return nullptr;
}

Regex* Regex::scan_symb(const vector<Regex::Lexem>& lexems, int index_start,
						int index_end) {
	Regex* p = nullptr;
	if ((lexems.size() <= index_start) ||
		(lexems[index_start].type != Regex::Lexem::symb) ||
		(index_end - index_start > 1)) {
		return nullptr;
	}

	p = new Regex;
	p->value = lexems[index_start];
	p->type = Regex::symb;

	vector<alphabet_symbol> v = {lexems[index_start].symbol};
	// char_to_alphabet_symbol(lexems[index_start].symbol)};
	set<alphabet_symbol> s(v.begin(), v.end());

	p->alphabet = s;
	p->term_l = nullptr;
	p->term_r = nullptr;
	return p;
}

Regex* Regex::scan_eps(const vector<Regex::Lexem>& lexems, int index_start,
					   int index_end) {
	Regex* p = nullptr;
	// cout << lexems[index_start].type << "\n";
	if (lexems.size() <= (index_start) || (index_end - index_start != 1) ||
		lexems[index_start].type != Regex::Lexem::eps) {
		return nullptr;
	}
	p = new Regex;
	p->value = lexems[index_start];
	p->type = Regex::eps;

	set<alphabet_symbol> s;
	// l = new Language(s);

	p->alphabet = s;
	p->term_l = nullptr;
	p->term_r = nullptr;
	return p;
}

Regex* Regex::scan_par(const vector<Regex::Lexem>& lexems, int index_start,
					   int index_end) {
	Regex* p = nullptr;

	if (lexems.size() <= (index_end - 1) ||
		(lexems[index_start].type != Regex::Lexem::parL ||
		 lexems[index_end - 1].type != Regex::Lexem::parR)) {
		return nullptr;
	}
	p = expr(lexems, index_start + 1, index_end - 1);
	return p;
}
Regex* Regex::expr(const vector<Regex::Lexem>& lexems, int index_start,
				   int index_end) {
	Regex* p;
	p = scan_alt(lexems, index_start, index_end);
	if (!p) {
		p = scan_conc(lexems, index_start, index_end);
	}
	if (!p) {
		p = scan_star(lexems, index_start, index_end);
	}
	if (!p) {
		p = scan_symb(lexems, index_start, index_end);
	}
	if (!p) {
		p = scan_eps(lexems, index_start, index_end);
	}
	if (!p) {
		p = scan_par(lexems, index_start, index_end);
	}
	return p;
}
Regex::Regex() {
	type = Regex::eps;
	term_l = nullptr;
	term_r = nullptr;
}

Regex::Regex(const string& str) : Regex() {
	try {
		bool res = from_string(str);
		if (!res) {
			throw runtime_error("Regex::from_string() ERROR");
		}
	} catch (const runtime_error& re) {
		cout << re.what() << endl;
		exit(EXIT_FAILURE);
	}
}

Regex::Regex(const string& str, const shared_ptr<Language>& new_language)
	: Regex(str) {
	language = new_language;
}

Regex Regex::normalize_regex(const vector<pair<Regex, Regex>>& rules,
							 iLogTemplate* log) const {
	// Logger::init_step("Normalize");
	Regex regex = *this;
	// Logger::log("Регулярное выражение до нормализации", regex.to_txt());
	// regex.normalize_this_regex(rules);
	/*Logger::log("Регулярное выражение после нормализации", regex.to_txt());
	Logger::finish_step();*/
	if (log) {
		log->set_parameter("oldregex", *this);
	}
	return regex;
}
bool Regex::from_string(const string& str) {
	if (!str.size()) {
		value = Regex::Lexem::eps;
		type = Regex::eps;
		alphabet = {};
		language = make_shared<Language>(alphabet);
		return true;
	}

	vector<Regex::Lexem> l = parse_string(str);
	Regex* root = expr(l, 0, l.size());

	if (root == nullptr || root->type == eps) {
		if (root != nullptr) delete root;

		return false;
	}

	//*this = *(root->copy());
	value = root->value;
	type = root->type;
	alphabet = root->alphabet;
	language = make_shared<Language>(alphabet);
	if (root->term_l != nullptr) {
		term_l = root->term_l->copy();
	}
	if (root->term_r != nullptr) {
		term_r = root->term_r->copy();
	}
	delete root;
	return true;
}
void Regex::regex_union(Regex* a, Regex* b) {
	type = Type::conc;
	term_l = a->copy();
	term_r = b->copy();
}
void Regex::regex_alt(Regex* a, Regex* b) {
	type = Type::alt;
	term_l = a->copy();
	term_r = b->copy();
}
void Regex::regex_star(Regex* a) {
	type = Type::star;
	term_l = a->copy();
}
void Regex::regex_eps() {
	type = Type::eps;
}

void Regex::clear() {
	if (term_l != nullptr) {
		delete term_l;
		term_l = nullptr;
	}
	if (term_r != nullptr) {
		delete term_r;
		term_r = nullptr;
	}
}

Regex::~Regex() {
	clear();
}

Regex* Regex::copy() const {
	Regex* c = new Regex();
	c->alphabet = alphabet;
	c->type = type;
	c->value = value;
	c->language = language;
	if (type != Regex::eps && type != Regex::symb) {
		c->term_l = term_l->copy();
		if (type != Regex::star) c->term_r = term_r->copy();
	}
	return c;
}

Regex::Regex(const Regex& reg)
	: BaseObject(reg.language), type(reg.type), value(reg.value),
	  alphabet(reg.alphabet),
	  term_l(reg.term_l == nullptr ? nullptr : reg.term_l->copy()),
	  term_r(reg.term_r == nullptr ? nullptr : reg.term_r->copy()) {}

Regex& Regex::operator=(const Regex& reg) {
	clear();
	language = reg.language;
	type = reg.type;
	value = reg.value;
	alphabet = reg.alphabet;
	if (reg.term_l) term_l = reg.term_l->copy();
	if (reg.term_r) term_r = reg.term_r->copy();
	return *this;
}

void Regex::generate_alphabet(set<alphabet_symbol>& _alphabet) {
	if (term_l != nullptr) {
		term_l->generate_alphabet(alphabet);
	}
	if (term_r != nullptr) {
		term_r->generate_alphabet(alphabet);
	}
	for (auto sym : alphabet) {
		_alphabet.insert(sym);
	}
}

void Regex::make_language() {
	generate_alphabet(alphabet);
	language = make_shared<Language>(alphabet);
}

void Regex::set_language(const set<alphabet_symbol>& _alphabet) {
	alphabet = _alphabet;
	language = make_shared<Language>(alphabet);
}

void Regex::normalize_this_regex(const vector<pair<Regex, Regex>>& rules) {}

void Regex::pre_order_travers() const {
	if (type == Regex::symb /*&& value.symbol*/) {
		cout << value.symbol << " ";
	} else {
		cout << type << " ";
	}
	if (term_l) {
		term_l->pre_order_travers();
	}
	if (term_r) {
		term_r->pre_order_travers();
	}
}

string Regex::to_txt() const {
	string str1 = "", str2 = "";
	if (term_l) {
		str1 = term_l->to_txt();
	}
	if (term_r) {
		str2 = term_r->to_txt();
	}
	string symb;
	if (type == Type::conc) {
		if (term_l && term_l->type == Type::alt) {
			str1 = "(" + str1 + ")";
		}
		if (term_r && term_r->type == Type::alt) {
			str2 = "(" + str2 + ")";
		}
	}
	if (type == Type::symb /*value.symbol*/) symb = value.symbol;
	if (type == Type::eps) symb = "";
	if (type == Type::alt) symb = '|';
	if (type == Type::star) {
		symb = '*';
		if (term_l->type != Type::symb)
			str1 = "(" + str1 +
				   ")"; // ставим скобки при итерации, если символов > 1
	}

	return str1 + symb + str2;
}

// для метода test
string Regex::get_iterated_word(int n) const {
	string str = "";
	/*if (type == Type::alt) {
		cout << "ERROR: regex with '|' is passed to the method Test\n";
		return "";
	}*/
	if (term_l) {
		if (type == Type::star) {
			for (int i = 0; i < n; i++)
				str += term_l->get_iterated_word(n);
		} else
			str += term_l->get_iterated_word(n);
	}
	if (term_r && type != Type::alt) {
		str += term_r->get_iterated_word(n);
	}
	if (value.symbol != "") {
		str += value.symbol;
	}
	return str;
}

// возвращает пару <вектор сотсояний, max_index>
pair<vector<State>, int> Regex::get_thompson(int max_index) const {
	string str;			  // идентификатор состояния
	vector<State> s = {}; // вектор состояний нового автомата
	map<alphabet_symbol, set<int>> m, p, map_l, map_r; // словари автоматов
	set<int> trans; // новые транзишены
	int offset; // сдвиг для старых индексов состояний в новом автомате
	pair<vector<State>, int> al; // для левого автомата относительно операции
	pair<vector<State>, int> ar; // для правого автомата относительно операции
	Language* alp;				 // Новый язык для автомата
	switch (type) {
	case Regex::alt: // |

		al = term_l->get_thompson(max_index);
		ar = term_r->get_thompson(al.second);
		max_index = ar.second;

		str = "q" + to_string(max_index + 1);
		m[alphabet_symbol::epsilon()] = {1, int(al.first.size()) + 1};
		s.push_back(State(0, {}, str, false, m));

		for (size_t i = 0; i < al.first.size(); i++) {
			State test = al.first[i];
			for (auto el : test.transitions) {
				alphabet_symbol elem = el.first; // al->alphabet[i];
				trans = {};
				for (int transition_to : test.transitions[elem]) {
					trans.insert(transition_to + 1);
				}
				map_l[elem] = trans;
			}

			if (test.is_terminal) {
				map_l[alphabet_symbol::epsilon()] = {
					int(al.first.size() + ar.first.size()) + 1};
			}
			s.push_back(State(al.first[i].index + 1, {}, al.first[i].identifier,
							  false, map_l));
			map_l = {};
		}
		offset = s.size();
		for (size_t i = 0; i < ar.first.size(); i++) {
			State test = ar.first[i];
			for (auto el : test.transitions) {
				alphabet_symbol elem = el.first;
				trans = {};
				for (int transition_to : test.transitions[elem]) {
					trans.insert(transition_to + offset);
				}
				map_r[elem] = trans;
			}
			if (test.is_terminal) {
				map_r[alphabet_symbol::epsilon()] = {offset +
													 int(ar.first.size())};
			}

			s.push_back(State(ar.first[i].index + offset, {},
							  ar.first[i].identifier, false, map_r));
			map_r = {};
		}

		str = "q" + to_string(max_index + 2);
		s.push_back(State(int(al.first.size() + ar.first.size()) + 1, {}, str,
						  true, p));

		return {s, max_index + 2};
	case Regex::conc: // .
		al = term_l->get_thompson(max_index);
		ar = term_r->get_thompson(al.second);
		max_index = ar.second;

		for (size_t i = 0; i < al.first.size(); i++) {
			State test = al.first[i];
			for (auto el : test.transitions) {
				alphabet_symbol elem = el.first; // al->alphabet[i];
				trans = {};
				for (int transition_to : test.transitions[elem]) {
					trans.insert(transition_to);
				}
				map_l[elem] = trans;
			}

			if (test.is_terminal) {
				State test_r = ar.first[0];
				for (auto el : test_r.transitions) {
					alphabet_symbol elem = el.first; // al->alphabet[i];
					for (int transition_to : test_r.transitions[elem]) {
						// trans.push_back(test.transitions[elem][j] + 1);
						map_l[elem].insert(transition_to + al.first.size() - 1);
					}
					// map_l[elem] = trans;
				}
			}
			s.push_back(State(al.first[i].index, {}, al.first[i].identifier,
							  false, map_l));
			map_l = {};
		}
		offset = s.size();
		for (size_t i = 1; i < ar.first.size(); i++) {
			State test = ar.first[i];
			for (auto el : test.transitions) {
				alphabet_symbol elem = el.first; // al->alphabet[i];
				trans = {};
				// alfa.push_back(elem);
				for (int transition_to : test.transitions[elem]) {
					trans.insert(transition_to + offset - 1);
				}
				map_r[elem] = trans;
			}

			s.push_back(State(ar.first[i].index + offset - 1, {},
							  ar.first[i].identifier, test.is_terminal, map_r));
			map_r = {};
		}

		return {s, max_index};
	case Regex::star: // *
		al = term_l->get_thompson(max_index);
		max_index = al.second;

		str = "q" + to_string(max_index + 1);
		m[alphabet_symbol::epsilon()] = {1, int(al.first.size()) + 1};
		s.push_back(State(0, {}, str, false, m));

		for (size_t i = 0; i < al.first.size(); i++) {
			State test;
			test = al.first[i];
			for (auto el : test.transitions) {
				alphabet_symbol elem = el.first; // al->alphabet[i];
				trans = {};
				for (int transition_to : test.transitions[elem]) {
					trans.insert(transition_to + 1);
				}
				map_l[elem] = trans;
			}

			if (test.is_terminal) {
				map_l[alphabet_symbol::epsilon()] = {1,
													 int(al.first.size()) + 1};
			}
			s.push_back(State(al.first[i].index + 1, {}, al.first[i].identifier,
							  false, map_l));
			map_l = {};
		}
		offset = s.size();

		str = "q" + to_string(max_index + 2);
		s.push_back(State(int(al.first.size()) + 1, {}, str, true, p));

		return {s, max_index + 2};
	case Regex::eps:
		str = "q" + to_string(max_index + 1);

		m[alphabet_symbol::epsilon()] = {1};
		s.push_back(State(0, {}, str, false, m));
		str = "q" + to_string(max_index + 2);
		s.push_back(State(1, {}, str, true, p));

		return {s, max_index + 2};
	default:

		str = "q" + to_string(max_index + 1);
		// m[char_to_alphabet_symbol(value.symbol)] = {1};
		m[value.symbol] = {1};
		s.push_back(State(0, {}, str, false, m));
		str = "q" + to_string(max_index + 2);
		s.push_back(State(1, {}, str, true, p));

		return {s, max_index + 2};
	}
	return {};
}

FiniteAutomaton Regex::to_thompson(iLogTemplate* log) const {
	// Logger::init_step("Автомат Томпсона");
	// Logger::log("Регулярное выражение", to_txt());
	FiniteAutomaton fa(0, get_thompson(-1).first, language);
	if (log) {
		log->set_parameter("oldregex", *this);
		log->set_parameter("result", fa);
	}
	// Logger::log("Автомат", fa);
	// Logger::finish_step();
	return fa;
}

bool Regex::contains_eps() const {
	int l;
	int r;
	switch (type) {
	case Regex::alt:
		return term_l->contains_eps() || term_r->contains_eps();
	case Regex::conc:
		return term_l->contains_eps() && term_r->contains_eps();
	case Regex::star:
		return 1;
	case Regex::eps:
		return 1;
	default:
		return 0;
	}
}
vector<Regex::Lexem>* Regex::first_state() const {
	vector<Regex::Lexem>* l;
	vector<Regex::Lexem>* r;
	switch (type) {
	case Regex::alt:
		l = term_l->first_state();
		r = term_r->first_state();
		l->insert(l->end(), r->begin(), r->end());
		delete r;
		return l;
	case Regex::star:
		l = term_l->first_state();
		return l;
	case Regex::conc:
		l = term_l->first_state();
		if (term_l->contains_eps()) {
			r = term_r->first_state();
			l->insert(l->end(), r->begin(), r->end());
			delete r;
		}
		//
		return l;
	case Regex::eps:
		l = new vector<Regex::Lexem>;
		return l;
	default:
		l = new vector<Regex::Lexem>;
		l->push_back(value);
		return l;
	}
}

vector<Regex::Lexem>* Regex::end_state() const {
	vector<Regex::Lexem>* l;
	vector<Regex::Lexem>* r;
	switch (type) {
	case Regex::alt:
		l = term_l->end_state();
		r = term_r->end_state();
		l->insert(l->end(), r->begin(), r->end());
		delete r;
		return l;
	case Regex::star:
		l = term_l->end_state();
		return l;
	case Regex::conc:
		l = term_r->end_state();
		if (term_r->contains_eps()) {

			r = term_l->end_state();
			l->insert(l->end(), r->begin(), r->end());
			delete r;
		}
		return l;
	case Regex::eps:
		l = new vector<Regex::Lexem>;
		return l;
	default:
		l = new vector<Regex::Lexem>;
		l->push_back(value);
		return l;
	}
}

map<int, vector<int>> Regex::pairs() const {
	map<int, vector<int>> l;
	map<int, vector<int>> r;
	map<int, vector<int>> p;
	vector<Regex::Lexem>* rs;
	vector<Regex::Lexem>* ps;
	switch (type) {
	case Regex::alt:
		l = term_l->pairs();
		r = term_r->pairs();
		for (auto& it : r) {
			l[it.first].insert(l[it.first].end(), it.second.begin(),
							   it.second.end());
		}
		return l;
	case Regex::star:
		l = term_l->pairs();
		rs = term_l->end_state();
		ps = term_l->first_state();
		for (size_t i = 0; i < rs->size(); i++) {
			for (size_t j = 0; j < ps->size(); j++) {
				r[(*rs)[i].number].push_back((*ps)[j].number);
			}
		}
		for (auto& it : r) {
			l[it.first].insert(l[it.first].end(), it.second.begin(),
							   it.second.end());
		}
		delete rs;
		delete ps;
		return l;
	case Regex::conc:
		l = term_l->pairs();
		r = term_r->pairs();
		for (auto& it : r) {
			l[it.first].insert(l[it.first].end(), it.second.begin(),
							   it.second.end());
		}
		r = {};
		rs = term_l->end_state();
		ps = term_r->first_state();

		for (size_t i = 0; i < rs->size(); i++) {
			for (size_t j = 0; j < ps->size(); j++) {
				r[(*rs)[i].number].push_back((*ps)[j].number);
			}
		}
		for (auto& it : r) {
			l[it.first].insert(l[it.first].end(), it.second.begin(),
							   it.second.end());
		}
		delete rs;
		delete ps;
		return l;
	default:
		break;
	}
	return {};
}

vector<Regex*> Regex::pre_order_travers_vect() {
	vector<Regex*> r;
	vector<Regex*> ret;
	if (Regex::symb == type /*value.symbol*/) {
		r = {};
		r.push_back(this);
		return r;
	}
	r = {};
	if (term_l) {
		ret = term_l->pre_order_travers_vect();
		r.insert(r.end(), ret.begin(), ret.end());
	}
	if (term_r) {
		ret = term_r->pre_order_travers_vect();
		r.insert(r.end(), ret.begin(), ret.end());
	}
	return r;
}
bool Regex::is_term(int number, const vector<Regex::Lexem>& list) const {
	for (size_t i = 0; i < list.size(); i++) {
		if (list[i].number == number) {
			return true;
		}
	}
	return false;
}
Regex Regex::linearize(iLogTemplate* log) const {
	// Logger::init_step("Linearise");
	Regex test(*this);
	vector<Regex*> list = test.pre_order_travers_vect();
	set<alphabet_symbol> lang_l;
	for (size_t i = 0; i < list.size(); i++) {
		list[i]->value.symbol.linearize(i);
		lang_l.insert(list[i]->value.symbol);
	}
	test.set_language(lang_l);
	if (log) {
		log->set_parameter("oldregex", *this);
		log->set_parameter("linearised regex", test);
	}
	// Logger::log(test.to_txt());
	// Logger::finish_step();
	return test;
}

Regex Regex::delinearize(iLogTemplate* log) const {
	// Logger::init_step("DeLinearise");
	Regex test(*this);
	vector<Regex*> list = test.pre_order_travers_vect();
	set<alphabet_symbol> lang_del;
	for (size_t i = 0; i < list.size(); i++) {
		list[i]->value.symbol.delinearize();
		lang_del.insert(list[i]->value.symbol);
	}
	test.set_language(lang_del);
	if (log) {
		log->set_parameter("oldregex", *this);
		log->set_parameter("result", test);
	}
	// Logger::log(test.to_txt());
	return test;
}

FiniteAutomaton Regex::to_glushkov(iLogTemplate* log) const {

	// Logger::init_step("Автомат Глушкова");

	Regex test(*this);
	vector<Regex*> list = test.pre_order_travers_vect();
	for (size_t i = 0; i < list.size(); i++) {
		list[i]->value.number = i;
		list[i]->value.symbol.linearize(i);
	}
	vector<Lexem>* first = test.first_state(); // Множество начальных состояний
	vector<Lexem>* end = test.end_state(); // Множество конечных состояний
	int eps_in = test.contains_eps();
	map<int, vector<int>> p = test.pairs(); // Множество возможных пар состояний
	vector<State> st; // Список состояний в автомате
	map<alphabet_symbol, set<int>> tr; // мап для переходов в каждом состоянии

	string str_first = "";
	string str_end = "";
	string str_pair = "";
	for (size_t i = 0; i < first->size(); i++) {
		str_first += string((*first)[i].symbol) + "\\ ";
	}

	set<string> end_set;

	for (size_t i = 0; i < end->size(); i++) {
		// str_end = str_end + string((*end)[i].symbol) +
		//		  to_string((*end)[i].number + 1) + " ";
		end_set.insert(string((*end)[i].symbol));
	}

	for (auto& elem : end_set) {
		str_end = str_end + elem + "\\ ";
	}

	for (auto& it1 : p) {
		for (size_t i = 0; i < it1.second.size(); i++) {
			str_pair = str_pair + "(" + string(list[it1.first]->value.symbol) +
					   "," + string(list[it1.second[i]]->value.symbol) + ")" +
					   "\\ ";
		}
	}

	if (eps) {
		str_end = "eps" + str_end;
	}

	// cout << test.to_str_log() << endl;
	// cout << "First " << str_first << endl;
	// cout << "End " << str_end << endl;
	// cout << "Pairs " << str_pair << endl;

	/*Logger::log("Регулярка", test.to_txt());
	Logger::log("First", str_first);
	Logger::log("End", str_end);
	Logger::log("Pairs", str_pair);*/
	vector<Regex> list_annote;
	for (size_t i = 0; i < list.size(); i++) {
		list_annote.push_back(*list[i]);
		list[i]->value.symbol.delinearize();
	}
	for (size_t i = 0; i < first->size(); i++) {
		(*first)[i].symbol.delinearize();
		tr[(*first)[i].symbol].insert((*first)[i].number + 1);
	}

	if (eps_in) {
		st.push_back(State(0, {}, "S", true, tr));
	} else {
		st.push_back(State(0, {}, "S", false, tr));
	}

	for (size_t i = 0; i < list.size(); i++) {
		Regex::Lexem elem = list[i]->value;
		tr = {};

		for (size_t j = 0; j < p[elem.number].size(); j++) {
			tr[list[p[elem.number][j]]->value.symbol].insert(p[elem.number][j] +
															 1);
		}
		string s = list_annote[i].value.symbol;
		st.push_back(State(i + 1, {}, s, is_term(elem.number, (*end)), tr));
	}
	delete first;
	delete end;
	FiniteAutomaton fa(0, st, language);
	if (log) {
		log->set_parameter("oldregex", test);
		log->set_parameter("linearised regex", test.linearize());
		log->set_parameter("first", str_first);
		log->set_parameter("end", str_end);
		log->set_parameter("pairs", str_pair);
		log->set_parameter("result", fa);
	}
	// Logger::log("Автомат", fa);
	// Logger::finish_step();
	return fa;
}

FiniteAutomaton Regex::to_ilieyu(iLogTemplate* log) const {
	// Logger::init_step("Автомат Илия–Ю");
	FiniteAutomaton glushkov = this->to_glushkov();
	vector<State> states = glushkov.states;
	vector<int> follow;
	for (size_t i = 0; i < states.size(); i++) {
		State st1 = states[i];
		map<alphabet_symbol, set<int>> map1 = st1.transitions;
		for (size_t j = i + 1; j < states.size(); j++) {
			State st2 = states[j];
			map<alphabet_symbol, set<int>> map2 = st2.transitions;
			bool flag = true;
			if (i == j || map2.size() != map1.size()) {
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

	vector<State> new_states;
	for (size_t i = 0; i < states.size(); i++) {
		if (find(follow.begin(), follow.end(), states[i].index) ==
			follow.end()) {
			new_states.push_back(states[i]);
		}
	}

	string str_follow;
	for (size_t i = 0; i < new_states.size(); i++) {
		int state_ind = new_states[i].index;
		str_follow = str_follow + states[state_ind].identifier + ":\\ ";
		for (auto j = states[state_ind].label.begin();
			 j != states[state_ind].label.end(); j++) {
			str_follow = str_follow + states[*j].identifier + "\\ ";
		}
		str_follow = str_follow + ";\\\\";
	}

	// cout << str_follow;
	// Logger::log("Регулярное выражение", to_txt());
	// Logger::log("Автомат Глушкова", glushkov);
	// Logger::log("Follow-отношения", str_follow);

	for (size_t i = 0; i < new_states.size(); i++) {
		State v1 = new_states[i];
		map<alphabet_symbol, set<int>> old_map = v1.transitions;
		map<alphabet_symbol, set<int>> new_map;
		for (auto& it1 : old_map) {
			set<int> v1 = it1.second;
			for (int transition_to : v1) {
				for (size_t k = 0; k < new_states.size(); k++) {
					if (new_states[k].label.find(transition_to) !=
							new_states[k].label.end() ||
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
	// Logger::log("Автомат", fa);
	// Logger::finish_step();
	return fa;
}
bool Regex::is_eps_possible() {
	switch (type) {
	case Type::eps:
		return true;
	case Type::symb:
		return false;
	case Type::alt:
		return term_l->is_eps_possible() || term_r->is_eps_possible();
	case Type::conc:
		return term_l->is_eps_possible() && term_r->is_eps_possible();
	case Type::star:
		return true;
	default:
		return false;
	}
}

void Regex::get_prefix(int len, std::set<std::string>* prefs) const {
	std::set<std::string>*prefs1, *prefs2;
	if (len == 0) {
		prefs->insert("");
		return;
	}
	switch (type) {
	case Type::eps:
		if (len == 0) prefs->insert("");
		return;
	case Type::symb:
		if (len == 1) {
			std::string res = "";
			res += value.symbol;
			prefs->insert(res);
		}
		return;
	case Type::alt:
		prefs1 = new std::set<std::string>();
		prefs2 = new std::set<std::string>();
		term_l->get_prefix(len, prefs1);
		term_r->get_prefix(len, prefs2);
		for (auto i = prefs1->begin(); i != prefs1->end(); i++) {
			prefs->insert(*i);
		}
		for (auto i = prefs2->begin(); i != prefs2->end(); i++) {
			prefs->insert(*i);
		}
		delete prefs1;
		delete prefs2;
		return;
	case Type::conc:
		prefs1 = new std::set<std::string>();
		prefs2 = new std::set<std::string>();
		for (int k = 0; k <= len; k++) {
			term_l->get_prefix(k, prefs1);
			term_r->get_prefix(len - k, prefs2);
			for (auto i = prefs1->begin(); i != prefs1->end(); i++) {
				for (auto j = prefs2->begin(); j != prefs2->end(); j++) {
					if (prefix_derivative(*i + *j)) prefs->insert(*i + *j);
				}
			}
			prefs1->clear();
			prefs2->clear();
		}
		delete prefs1;
		delete prefs2;
		return;
	case Type::star:
		if (len == 0) {
			prefs->insert("");
			return;
		}
		prefs1 = new std::set<std::string>();
		prefs2 = new std::set<std::string>();
		for (int k = 1; k <= len; k++) {
			term_l->get_prefix(k, prefs1);
			get_prefix(len - k, prefs2);
			for (auto i = prefs1->begin(); i != prefs1->end(); i++) {
				for (auto j = prefs2->begin(); j != prefs2->end(); j++) {
					if (prefix_derivative(*i + *j)) prefs->insert(*i + *j);
				}
			}
			prefs1->clear();
			prefs2->clear();
		}
		delete prefs1;
		delete prefs2;
		return;
	}
}

bool Regex::derivative_with_respect_to_sym(Regex* respected_sym,
										   const Regex* reg_e,
										   Regex& result) const {
	if (respected_sym->type != Type::eps && respected_sym->type != Type::symb) {
		std::cout << "Invalid input: unexpected regex instead of symbol\n";
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
		if (respected_sym->type != Type::eps) return false;
		result.type = Type::eps;
		return answer;
	case Type::symb:
		if (respected_sym->value.symbol != reg_e->value.symbol) {
			return false;
		}
		result.type = Type::eps;
		return answer;
	case Type::alt:
		answer1 = derivative_with_respect_to_sym(respected_sym, reg_e->term_l,
												 subresult);
		answer2 = derivative_with_respect_to_sym(respected_sym, reg_e->term_r,
												 subresult1);
		// cout << "alt of " << reg_e->term_l->to_txt() << " and "
		//	 << reg_e->term_r->to_txt() << "\n";
		// cout << answer1 << " " << answer2 << "\n";
		if (answer1 && answer2) {
			result.type = Type::alt;
			result.term_l = subresult.copy();
			result.term_r = subresult1.copy();
			// cout << result.to_txt() << "\n";
			return true;
		}
		if (!answer1 && !answer2) {
			// cout << result.to_txt() << "\n";
			return false;
		}
		if (answer1) {
			result = subresult;
			// cout << result.to_txt() << "\n";
			return true;
		}
		if (answer2 && !answer1) {
			result = subresult1;
			// cout << result.to_txt() << "\n";
			return true;
		}
		return answer;
	case Type::conc:
		subresult.type = Type::conc;
		if (subresult.term_l == nullptr) subresult.term_l = new Regex();
		answer1 = derivative_with_respect_to_sym(respected_sym, reg_e->term_l,
												 *subresult.term_l);
		subresult.term_r = reg_e->term_r->copy();
		if (reg_e->term_l->is_eps_possible()) {
			answer2 = derivative_with_respect_to_sym(respected_sym,
													 reg_e->term_r, subresult1);
			if (answer1 && answer2) {
				result.type = Type::alt;
				result.term_l = subresult.copy();
				result.term_r = subresult1.copy();
			}
			if (answer1 && !answer2) {
				result.type = subresult.type;
				if (subresult.term_l != nullptr)
					result.term_l = subresult.term_l->copy();
				if (subresult.term_r != nullptr)
					result.term_r = subresult.term_r->copy();
			}
			if (answer2 && !answer1) {
				result.type = subresult1.type;
				if (subresult1.term_l != nullptr)
					result.term_l = subresult1.term_l->copy();
				if (subresult1.term_r != nullptr)
					result.term_r = subresult1.term_r->copy();
			}
			// cout << "conc of " << reg_e->term_l->to_txt() << " and "
			//	 << reg_e->term_r->to_txt() << "\n";
			// cout << answer1 << " " << answer2 << " " << result.to_txt()
			//	 << "\n";
			answer = answer1 | answer2;
		} else {
			// cout << "conc of " << reg_e->term_l->to_txt() << " and "
			//	 << reg_e->term_r->to_txt() << "\n";
			// cout << answer1 << "\n";
			answer = answer1;
			result.type = subresult.type;
			if (subresult.term_l != nullptr)
				result.term_l = subresult.term_l->copy();
			if (subresult.term_r != nullptr)
				result.term_r = subresult.term_r->copy();
		}
		return answer;
	case Type::star:
		result.type = Type::conc;
		if (result.term_l == nullptr) result.term_l = new Regex();
		bool answer = derivative_with_respect_to_sym(
			respected_sym, reg_e->term_l, *result.term_l);
		result.term_r = reg_e->copy();
		return answer;
	}
}

bool Regex::partial_derivative_with_respect_to_sym(
	Regex* respected_sym, const Regex* reg_e, vector<Regex>& result) const {
	Regex cur_result;
	if (respected_sym->type != Type::eps && respected_sym->type != Type::symb) {
		std::cout << "Invalid input: unexpected regex instead of symbol\n";
		return false;
	}
	if (respected_sym->type == Type::eps) {
		cur_result.type = reg_e->type;
		if (reg_e->term_l != nullptr) cur_result.term_l = reg_e->term_l->copy();
		if (reg_e->term_l) cur_result.term_r = reg_e->term_l->copy();
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
		if (respected_sym->value.symbol != reg_e->value.symbol) {
			return false;
		}
		cur_result.type = Type::eps;
		result.push_back(cur_result);
		return answer;
	case Type::alt:
		answer1 = partial_derivative_with_respect_to_sym(
			respected_sym, reg_e->term_l, subresult);
		answer2 = partial_derivative_with_respect_to_sym(
			respected_sym, reg_e->term_r, subresult1);
		for (int i = 0; i < subresult.size(); i++) {
			result.push_back(subresult[i]);
		}
		for (int i = 0; i < subresult1.size(); i++) {
			result.push_back(subresult1[i]);
		}
		answer = answer1 | answer2;
		return answer;
	case Type::conc:
		cur_subresult.type = Type::conc;
		answer1 = partial_derivative_with_respect_to_sym(
			respected_sym, reg_e->term_l, subresult);
		cur_subresult.term_r = reg_e->term_r->copy();
		for (int i = 0; i < subresult.size(); i++) {
			cur_subresult.term_l = subresult[i].copy();
			result.push_back(cur_subresult);
			delete cur_subresult.term_l;
			cur_subresult.term_l = nullptr;
		}
		if (reg_e->term_l->is_eps_possible()) {
			answer2 = partial_derivative_with_respect_to_sym(
				respected_sym, reg_e->term_r, subresult1);
			for (int i = 0; i < subresult1.size(); i++) {
				result.push_back(subresult1[i]);
			}
			answer = answer1 | answer2;
		} else {
			answer = answer1;
		}
		return answer;
	case Type::star:
		cur_result.type = Type::conc;
		bool answer = partial_derivative_with_respect_to_sym(
			respected_sym, reg_e->term_l, subresult);
		cur_result.term_r = reg_e->copy();
		for (int i = 0; i < subresult.size(); i++) {
			cur_result.term_l = subresult[i].copy();
			result.push_back(cur_result);
			delete cur_result.term_l;
			cur_result.term_l = nullptr;
		}
		return answer;
	}
}

bool Regex::derivative_with_respect_to_str(std::string str, const Regex* reg_e,
										   Regex& result) const {
	bool success = true;
	Regex cur = *reg_e;
	Regex next = *reg_e;
	// cout << "start getting derivative for prefix " << str << " in "
	//	 << reg_e->to_txt() << "\n";
	for (int i = 0; i < str.size(); i++) {
		Regex sym;
		sym.type = Type::symb;
		sym.value.symbol = str[i];
		next.clear();
		success &= derivative_with_respect_to_sym(&sym, &cur, next);
		// cout << "derivative for prefix " << sym->to_txt() << " in "
		//	 << cur.to_txt() << " is " << next.to_txt() << "\n";
		if (!success) {
			return false;
		}
		cur = next;
	}
	result = next;
	// cout << " answer is " << result.to_txt();
	return success;
}

// Производная по символу
std::optional<Regex> Regex::symbol_derivative(
	const Regex& respected_sym) const {
	auto rs = respected_sym.copy();
	Regex result;
	std::optional<Regex> ans;
	if (derivative_with_respect_to_sym(rs, this, result))
		ans = result;
	else
		ans = nullopt;
	delete rs;
	return ans;
}
// Частичная производная по символу
void Regex::partial_symbol_derivative(const Regex& respected_sym,
									  vector<Regex>& result) const {
	auto rs = respected_sym.copy();
	partial_derivative_with_respect_to_sym(rs, this, result);
	delete rs;
	return;
}
// Производная по префиксу
std::optional<Regex> Regex::prefix_derivative(std::string respected_str) const {
	Regex result;
	std::optional<Regex> ans;
	if (derivative_with_respect_to_str(respected_str, this, result))
		ans = result;
	else
		ans = nullopt;
	return ans;
}
// Длина накачки
// Длина накачки
int Regex::pump_length(iLogTemplate* log) const {
	if (log) log->set_parameter("oldregex", *this);
	if (language->pump_length_cached()) {
		if (log) {
			log->set_parameter("pumplength", language->get_pump_length());
			log->set_parameter("cach", "(!) результат получен из кэша");
		}
		return language->get_pump_length();
	}
	std::map<std::string, bool> checked_prefixes;
	for (int i = 1;; i++) {
		std::set<std::string> prefs;
		get_prefix(i, &prefs);
		if (prefs.empty()) {
			language->set_pump_length(i);
			if (log) {
				log->set_parameter("pumplength", i);
			}
			return i;
		}
		bool pumped = true;
		for (auto it = prefs.begin(); it != prefs.end(); it++) {
			bool was = false;
			for (int j = 0; j < it->size(); j++) {
				if (checked_prefixes[it->substr(0, j)]) {
					was = true;
					break;
				}
			}
			if (was) continue;
			bool infix_pumped = false;
			for (int j = 0; j < it->size(); j++) {
				std::string pumped_prefix;
				pumped_prefix += it->substr(0, j);
				pumped_prefix += "(" + it->substr(j, it->size()) + ")*";
				Regex a;
				if (!derivative_with_respect_to_str(*it, this, a)) {
					continue;
				}
				pumped_prefix += "(" + a.to_txt() + ")";
				Regex pumping(pumped_prefix);
				if (subset(pumping)) {
					checked_prefixes[*it] = true;
					infix_pumped = true;
					break;
				}
			}
			pumped &= infix_pumped;
		}
		std::string ch_prefixes;
		for (auto it = checked_prefixes.begin(); it != checked_prefixes.end();
			 it++) {
			if (it->second) ch_prefixes += it->first + "\n";
		}
		if (pumped) {
			language->set_pump_length(i);
			if (log) {
				log->set_parameter("pumplength1", i);
				log->set_parameter("pumplength2", ch_prefixes);
			}
			return i;
		}
	}
}

bool Regex::equality_checker(const Regex* r1, const Regex* r2) {
	if (r1 == nullptr && r2 == nullptr) return true;
	if (r1 == nullptr || r2 == nullptr) return false;
	if (r1->value.type != r2->value.type) return false;

	if (r1->value.type == Regex::Lexem::Type::symb) {
		alphabet_symbol r1_symb, r2_symb;
		r1_symb = r1->value.symbol;
		r2_symb = r2->value.symbol;
		if (r1_symb != r2_symb) return false;
	}

	if (equality_checker(r1->term_l, r2->term_l) &&
		equality_checker(r1->term_r, r2->term_r))
		return true;
	if (equality_checker(r1->term_r, r2->term_l) &&
		equality_checker(r1->term_l, r2->term_r))
		return true;
	return false;
}

bool Regex::equal(const Regex& r1, const Regex& r2, iLogTemplate* log) {
	// Logger::init_step("Equal");
	// Logger::log("Первое регулярное выражение", r1.to_txt());
	// Logger::log("Второе регулярное выражение", r2.to_txt());
	bool result = equality_checker(&r1, &r2);
	/*if (result)
		Logger::log("Результат Equal", "true");
	else
		Logger::log("Результат Equal", "false");
	Logger::finish_step();*/
	if (log) {
		log->set_parameter("regex1", r1);
		log->set_parameter("regex2", r2);
		log->set_parameter("result", result);
	}
	return result;
}

bool Regex::equivalent(const Regex& r1, const Regex& r2, iLogTemplate* log) {
	// Logger::init_step("Equiv");
	// Logger::log("Первое регулярное выражение", r1.to_txt());
	// Logger::log("Второе регулярное выражение", r2.to_txt());
	bool result = true;
	if (r1.language == r2.language) {
		if (log)
			log->set_parameter(
				"samelanguage",
				"(!) регулярные выражения изначально принадлежат одному языку");
		// Logger::log(
		//"(!) регулярные выражения изначально принадлежат одному языку");
	} else {
		FiniteAutomaton fa1 = r1.to_ilieyu();
		FiniteAutomaton fa2 = r2.to_ilieyu();
		result = FiniteAutomaton::equivalent(fa1, fa2);
	}
	/*if (result)
		Logger::log("Результат Equiv", "true");
	else
		Logger::log("Результат Equiv", "false");
	Logger::finish_step();*/
	if (log) {
		log->set_parameter("regex1", r1);
		log->set_parameter("regex2", r2);
		log->set_parameter("result", result);
	}
	return result;
}

bool Regex::subset(const Regex& r, iLogTemplate* log) const {
	// Logger::init_step("Subset");
	// Logger::log("Первое регулярное выражение", to_txt());
	// Logger::log("Второе регулярное выражение", r.to_txt());
	auto il = to_ilieyu();
	auto ril = r.to_ilieyu();
	bool result = il.subset(ril);
	/*if (result)
		Logger::log("Результат Subset", "true");
	else
		Logger::log("Результат Subset", "false");
	Logger::finish_step();*/
	if (log) {
		log->set_parameter("regex1", *this);
		log->set_parameter("regex2", r);
		log->set_parameter("result", result);
	}
	return result;
}

FiniteAutomaton Regex::to_antimirov(iLogTemplate* log) const {
	// Logger::init_step("Автомат Антимирова");
	// Logger::log("Регулярное выражение", to_txt());
	map<set<string>, set<string>> trans;
	vector<Regex> states;
	vector<Regex> alph_regex;
	vector<vector<Regex>> out;
	set<string> check;
	for (const alphabet_symbol& as : language->get_alphabet()) {
		string symbol = as;
		Regex r(symbol);
		alph_regex.push_back(r);
	}

	states.push_back(*this);
	check.insert(to_txt());
	for (size_t i = 0; i < states.size(); i++) {
		Regex state = states[i];
		for (vector<Regex>::iterator j = alph_regex.begin();
			 j != alph_regex.end(); j++) {
			vector<Regex> regs;
			state.partial_symbol_derivative(*j, regs);
			for (vector<Regex>::iterator k = regs.begin(); k != regs.end();
				 k++) {
				out.push_back({state, *k, *j});
				if (check.find(k->to_txt()) == check.end()) {
					states.push_back(*k);
					check.insert(k->to_txt());
				}
			}
		}
	}

	vector<string> name_states;

	for (size_t i = 0; i < states.size(); i++) {
		name_states.push_back(states[i].to_txt());
	}

	vector<State> automat_state;

	string deriv_log = "";

	for (size_t i = 0; i < name_states.size(); i++) {
		string state = name_states[i];
		map<alphabet_symbol, set<int>> transit;
		for (size_t j = 0; j < out.size(); j++) {
			// cout << out[j][0].to_txt() << " ";
			// cout << out[j][1].to_txt() << " ";
			// cout << out[j][2].to_txt() << endl;
			deriv_log +=
				out[j][2].to_txt() + "(" + out[j][0].to_txt() + ")" + "\\ =\\ ";
			if (out[j][1].to_txt() == "") {
				deriv_log += "eps\\\\";
			} else {
				deriv_log += out[j][1].to_txt() + "\\\\";
			}
			// Logger::log(deriv_log);
			if (out[j][0].to_txt() == state) {
				auto n = find(name_states.begin(), name_states.end(),
							  out[j][1].to_txt());
				alphabet_symbol s = out[j][2].to_txt();
				transit[s].insert(n - name_states.begin());
			}
		}

		if ((state.size() == 0) || (states[i].contains_eps())) {
			if (state == "") {
				state = alphabet_symbol::epsilon();
			}
			automat_state.push_back({int(i), {}, state, true, transit});
		} else {
			automat_state.push_back({int(i), {}, state, false, transit});
		}
	}
	string str_state = "";
	for (size_t i = 0; i < automat_state.size(); i++) {
		str_state += automat_state[i].identifier + "\\\\ ";
	}

	// cout << deriv_log;
	// cout << str_state << endl;
	// //Logger::log(deriv_log, str_state);
	// Logger::log(str_state);

	FiniteAutomaton fa(0, automat_state, language);
	if (log) {
		log->set_parameter("oldregex", *this);
		log->set_parameter("derivative", deriv_log);
		log->set_parameter("state", str_state);
		log->set_parameter("result", fa);
	}
	// Logger::log("Автомат", fa);
	// Logger::finish_step();
	return fa;
}

Regex Regex::deannote(iLogTemplate* log) const {
	// Logger::init_step("DeAnnote");
	Regex test(*this);
	// Logger::log("Регулярное выражение до преобразования", test.to_txt());
	vector<Regex*> list = test.pre_order_travers_vect();
	set<alphabet_symbol> lang_deann;
	for (size_t i = 0; i < list.size(); i++) {
		list[i]->value.symbol.deannote();
		lang_deann.insert(list[i]->value.symbol);
	}
	test.set_language(lang_deann);
	if (log) {
		log->set_parameter("oldregex", *this);
		log->set_parameter("result", test);
	}
	/*Logger::log("Регулярное выражение после преобразования", test.to_txt());
	Logger::finish_step();*/
	return test;
}

// для дебага
void Regex::print_subtree(Regex* r, int level) {
	if (r) {
		print_subtree(r->term_l, level + 1);
		for (int i = 0; i < level; i++)
			cout << "   ";
		alphabet_symbol r_v;
		if (r->value.symbol != "")
			r_v = r->value.symbol;
		else
			r_v = to_string(r->type);
		cout << r_v << endl;
		print_subtree(r->term_r, level + 1);
	}
}

void Regex::print_tree() {
	print_subtree(term_l, 1);
	for (int i = 0; i < 0; i++)
		cout << "   ";
	alphabet_symbol r_v;
	if (value.symbol != "")
		r_v = value.symbol;
	else
		r_v = to_string(type);
	cout << r_v << endl;
	print_subtree(term_r, 1);
}

bool Regex::is_one_unambiguous(iLogTemplate* log) const {
	if (log) {
		log->set_parameter("oldregex", *this);
	}
	// Logger::init_step("OneUnambiguity");
	FiniteAutomaton fa = to_glushkov();
	bool res = fa.is_deterministic();
	// Logger::log(res ? "True" : "False");
	// Logger::finish_step();
	if (log) {
		log->set_parameter("result", res ? "True" : "False");
	}
	return fa.is_deterministic();
}

Regex Regex::get_one_unambiguous_regex(iLogTemplate* log) const {
	// Logger::init_step("OneUnambiguityRegex");
	if (log) {
		log->set_parameter("oldregex", *this);
	}
	FiniteAutomaton fa = to_glushkov();
	if (fa.language->is_one_unambiguous_regex_cached()) {
		// Logger::log("1-однозначное регулярное выражение, описывающее язык",
		// 			fa.language->get_one_unambiguous_regex().to_txt());
		// Logger::finish_step();
		if (log) {
			log->set_parameter("result",
							   fa.language->get_one_unambiguous_regex());
			log->set_parameter("cach",
							   "(!) результат OneUnambiguous получен из кэша");
		}
		return fa.language->get_one_unambiguous_regex();
	}
	if (!fa.language->is_one_unambiguous_flag_cached()) fa.is_one_unambiguous();
	if (!fa.language->get_one_unambiguous_flag()) {
		// Logger::log("Язык не является 1-однозначным");
		// Logger::finish_step();
		if (log) {
			log->set_parameter("result", "Язык не является 1-однозначным");
		}
		return *this;
	}
	string regl;
	FiniteAutomaton min_fa;
	if (!fa.language->min_dfa_cached() && log) {
		log->set_parameter("cachedMINDFA",
						   "Минимальный автомат сохранен в кэше");
	}
	if (fa.states.size() == 1)
		min_fa = fa.minimize();
	else
		min_fa = fa.minimize().remove_trap_states();

	

	set<map<alphabet_symbol, set<int>>> final_states_transitions;
	for (int i = 0; i < min_fa.states.size(); i++) {
		if (min_fa.states[i].is_terminal) {
			final_states_transitions.insert(min_fa.states[i].transitions);
		}
	}

	set<alphabet_symbol> min_fa_consistent;
	// calculate a set of min_fa_consistent symbols
	for (alphabet_symbol symb : min_fa.language->get_alphabet()) {
		set<int> reachable_by_symb;
		bool is_symb_min_fa_consistent = true;
		for (int i = 0; i < min_fa.states.size(); i++) {
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
			for (auto final_state_transitions : final_states_transitions) {
				if (find(final_state_transitions[symb].begin(),
						 final_state_transitions[symb].end(),
						 elem) == final_state_transitions[symb].end()) {
					is_symb_min_fa_consistent = false;
					break;
				}
			}
			if (is_symb_min_fa_consistent) min_fa_consistent.insert(symb);
		}
	}

	FiniteAutomaton min_fa_cut =
		FiniteAutomaton(min_fa.initial_state, min_fa.states, min_fa.language);

	for (int i = 0; i < min_fa.states.size(); i++) {
		if (min_fa.states[i].is_terminal) {
			map<alphabet_symbol, set<int>> new_transitions;
			for (const auto& transition : min_fa.states[i].transitions) {
				if (find(min_fa_consistent.begin(), min_fa_consistent.end(),
						 transition.first) == min_fa_consistent.end()) {
					new_transitions[transition.first] = transition.second;
				}
			}
			min_fa_cut.states[i].transitions = new_transitions;
		}
	}

	min_fa_cut = min_fa_cut.remove_unreachable_states();
	regl = min_fa_cut.to_regex().to_txt();

	int counter = 0;
	for (alphabet_symbol consistent_symb : min_fa_consistent) {
		bool alternate_flag = 0;
		// TODO
		// сборка регулярок из строк будет ошибочной, если символы размечены
		if (!counter)
			regl += "(" + (string)consistent_symb;
		else {
			regl += "|" + (string)consistent_symb;
			alternate_flag = true;
		}
		set<int> reachable_by_consistent_symb;
		for (int i = 0; i < min_fa.states.size(); i++) {
			for (int consistent_symb_transition :
				 min_fa.states[i].transitions[consistent_symb]) {
				reachable_by_consistent_symb.insert(consistent_symb_transition);
			}
		}
		for (int elem : reachable_by_consistent_symb) {
			FiniteAutomaton consistent_symb_automaton(0, {},
													  make_shared<Language>());
			set<int> reachable_states = min_fa.closure({elem}, false);
			vector<int> inserted_states_indices;
			int consistent_symb_automaton_initial_state = 0;
			for (int reachable_state : reachable_states) {
				if (reachable_state == elem)
					consistent_symb_automaton.initial_state =
						consistent_symb_automaton_initial_state;
				consistent_symb_automaton.states.push_back(
					min_fa.states[reachable_state]);
				inserted_states_indices.push_back(reachable_state);
				consistent_symb_automaton_initial_state++;
			}
			set<alphabet_symbol> consistent_symb_automaton_alphabet;
			for (int j = 0; j < consistent_symb_automaton.states.size(); j++) {
				consistent_symb_automaton.states[j].index = j;
				map<alphabet_symbol, set<int>>
					consistent_symb_automaton_state_transitions;
				for (const auto& symb_transition :
					 consistent_symb_automaton.states[j].transitions) {
					for (int transition : symb_transition.second) {
						for (int k = 0; k < inserted_states_indices.size();
							 k++) {
							if (inserted_states_indices[k] == transition) {
								consistent_symb_automaton_state_transitions
									[symb_transition.first]
										.insert(k);
								consistent_symb_automaton_alphabet.insert(
									symb_transition.first);
							}
						}
					}
				}
				if (consistent_symb_automaton_state_transitions.size()) {
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
			for (int j = 0; j < consistent_symb_automaton.states.size(); j++) {
				if (consistent_symb_automaton.states[j].is_terminal) {
					map<alphabet_symbol, set<int>> new_transitions;
					for (const auto& transition :
						 consistent_symb_automaton.states[j].transitions) {
						if (find(min_fa_consistent.begin(),
								 min_fa_consistent.end(),
								 transition.first) == min_fa_consistent.end()) {
							new_transitions[transition.first] =
								transition.second;
						}
					}
					consistent_symb_automaton_cut.states[j].transitions =
						new_transitions;
				}
			}
			consistent_symb_automaton_cut =
				consistent_symb_automaton_cut.remove_unreachable_states();
			string consistent_symb_automaton_cut_to_regex =
				consistent_symb_automaton_cut.to_regex().to_txt();
			if (!consistent_symb_automaton_cut_to_regex.empty()) {
				if (alternate_flag) regl += "(";
				regl += consistent_symb_automaton_cut_to_regex;
				if (alternate_flag) regl += ")";
			}
		}
		counter++;
	}
	if (counter) regl += ")*";
	// Logger::log("1-однозначное регулярное выражение, описывающее язык",
	// regl); Logger::finish_step();
	language->set_one_unambiguous_regex(regl, fa.language);
	Regex res = language->get_one_unambiguous_regex();
	if (log) {
		log->set_parameter("result", res);
	}
	return res;
}