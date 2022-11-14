#include "Regex.h"
#include "FiniteAutomaton.h"
#include "Language.h"
#include "Logger.h"
#include <set>

Lexem::Lexem(Type type, alphabet_symbol symbol, int number)
	: type(type), symbol(symbol), number(number) {}

vector<Lexem> Regex::parse_string(string str) {
	vector<Lexem> lexems;
	lexems = {};
	bool flag_alt = false;
	auto is_symbol = [](char c) {
		return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
	};
	// int index = 0;
	// for (const char& c : str) {
	for (size_t index = 0; index < str.size(); index++) {
		char c = str[index];
		Lexem lexem;
		switch (c) {
		case '(':
			lexem.type = Lexem::parL;
			flag_alt = true;
			break;
		case ')':
			lexem.type = Lexem::parR;
			if (lexems.back().type == Lexem::parL || flag_alt) {
				lexem.type = Lexem::error;
				lexems = {};
				lexems.push_back(lexem);
				return lexems;
			}
			break;
		case '|':
			lexem.type = Lexem::alt;
			break;
		case '*':
			lexem.type = Lexem::star;
			break;
		default:
			if (is_symbol(c)) {
				lexem.type = Lexem::symb;
				string number;
				for (size_t i = index + 1; i < str.size(); i++) {
					if (str[i] >= '0' && str[i] <= '9') {
						number += str[i];
					} else {
						break;
					}
					index = i;
				}

				lexem.symbol = c + number;
				flag_alt = false;

			} else {
				lexem.type = Lexem::error;
				lexems = {};
				lexems.push_back(lexem);
				return lexems;
			}
			break;
		}

		if (lexems.size() &&
			(
				// Lexem left
				lexems.back().type == Lexem::symb ||
				lexems.back().type == Lexem::star ||
				lexems.back().type == Lexem::parR) &&
			(
				// Lexem right
				lexem.type == Lexem::symb || lexem.type == Lexem::parL)) {

			// We place . between
			lexems.push_back({Lexem::conc});
		}

		if (lexems.size() &&
			((lexems.back().type == Lexem::parL &&
			  (lexem.type == Lexem::parR || lexem.type == Lexem::alt)) ||
			 (lexems.back().type == Lexem::alt && lexem.type == Lexem::parR) ||
			 (lexems.back().type == Lexem::alt && lexem.type == Lexem::alt))) {
			//  We place eps between
			lexems.push_back({Lexem::eps});
		}

		lexems.push_back(lexem);
	}

	if (lexems.size() && lexems[0].type == Lexem::alt) {
		lexems.insert(lexems.begin(), {Lexem::eps});
	}

	if (lexems.back().type == Lexem::alt) {
		lexems.push_back({Lexem::eps});
	}

	int balance = 0;
	for (size_t i = 0; i < lexems.size(); i++) {
		if (lexems[i].type == Lexem::parL) {
			balance++;
		}
		if (lexems[i].type == Lexem::parR) {
			balance--;
		}
	}

	if (balance != 0) {
		lexems = {};
		lexems.push_back({Lexem::error});
		return lexems;
	}

	return lexems;
}

Regex* Regex::scan_conc(const vector<Lexem>& lexems, int index_start,
						int index_end) {
	Regex* p = nullptr;
	int balance = 0;
	for (int i = index_start; i < index_end; i++) {
		if (lexems[i].type == Lexem::parL) { // LEFT_BRACKET
			balance++;
		}
		if (lexems[i].type == Lexem::parR) { // RIGHT_BRACKET
			balance--;
		}
		if (lexems[i].type == Lexem::conc && balance == 0) {
			Regex* l = expr(lexems, index_start, i);
			Regex* r = expr(lexems, i + 1, index_end);

			if (l == nullptr || r == nullptr || r->type == Regex::eps ||
				l->type == Regex::eps) { // Проверка на адекватность)
				cout << "Test\n";
				return p;
			}

			p = new Regex;
			l->term_p = p;
			r->term_p = p;
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

Regex* Regex::scan_star(const vector<Lexem>& lexems, int index_start,
						int index_end) {
	Regex* p = nullptr;
	int balance = 0;
	for (int i = index_start; i < index_end; i++) {
		if (lexems[i].type == Lexem::parL) { // LEFT_BRACKET
			balance++;
		}
		if (lexems[i].type == Lexem::parR) { // RIGHT_BRACKET
			balance--;
		}
		if (lexems[i].type == Lexem::star && balance == 0) {
			Regex* l = expr(lexems, index_start, i);

			if (l == nullptr || l->type == Regex::eps) {
				return p;
			}

			p = new Regex;
			l->term_p = p;
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

Regex* Regex::scan_alt(const vector<Lexem>& lexems, int index_start,
					   int index_end) {
	Regex* p = nullptr;
	int balance = 0;
	for (int i = index_start; i < index_end; i++) {
		if (lexems[i].type == Lexem::parL) { // LEFT_BRACKET
			balance++;
		}
		if (lexems[i].type == Lexem::parR) { // RIGHT_BRACKET
			balance--;
		}
		if (lexems[i].type == Lexem::alt && balance == 0) {
			Regex* l = expr(lexems, index_start, i);
			Regex* r = expr(lexems, i + 1, index_end);
			// cout << l->type << " " << r->type << "\n";
			if (((l == nullptr) || (r == nullptr)) ||
				(l->type == Regex::eps &&
				 r->type == Regex::eps)) { // Проверка на адекватность)
				return nullptr;
			}

			p = new Regex;
			l->term_p = p;
			r->term_p = p;
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

Regex* Regex::scan_symb(const vector<Lexem>& lexems, int index_start,
						int index_end) {
	Regex* p = nullptr;
	if ((lexems.size() <= index_start) ||
		(lexems[index_start].type != Lexem::symb) ||
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

Regex* Regex::scan_eps(const vector<Lexem>& lexems, int index_start,
					   int index_end) {
	Regex* p = nullptr;
	// cout << lexems[index_start].type << "\n";
	if (lexems.size() <= (index_start) ||
		lexems[index_start].type != Lexem::eps) {
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

Regex* Regex::scan_par(const vector<Lexem>& lexems, int index_start,
					   int index_end) {
	Regex* p = nullptr;

	if (lexems.size() <= (index_end - 1) ||
		(lexems[index_start].type != Lexem::parL ||
		 lexems[index_end - 1].type != Lexem::parR)) {
		return nullptr;
	}
	p = expr(lexems, index_start + 1, index_end - 1);
	return p;
}
Regex* Regex::expr(const vector<Lexem>& lexems, int index_start,
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

Regex::Regex(string str) : Regex() {
	try {
		bool res = from_string(str);
		if (!res) {
			throw runtime_error("from_string ERROR");
		}
	} catch (const runtime_error& re) {
		cout << re.what() << endl;
		exit(EXIT_FAILURE);
	}
}

Regex Regex::normalize_regex(const string& file) const {
	Logger::init_step("Normalize");
	Regex regex = *this;
	Logger::log("Регулярное выражение до нормализации", regex.to_txt());
	regex.normalize_this_regex(file);
	Logger::log("Регулярное выражение после нормализации", regex.to_txt());
	Logger::finish_step();
	return regex;
}

bool Regex::from_string(string str) {
	if (!str.size()) {
		return false;
	}

	vector<Lexem> l = parse_string(str);
	Regex* root = expr(l, 0, l.size());

	if (root == nullptr || root->type == eps) {
		return false;
	}

	//*this = *(root->copy());
	value = root->value;
	type = root->type;
	alphabet = root->alphabet;
	language = make_shared<Language>(alphabet);
	if (root->term_l != nullptr) {
		term_l = root->term_l->copy();
		term_l->term_p = this;
	}
	if (root->term_r != nullptr) {
		term_r = root->term_r->copy();
		term_r->term_p = this;
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
		c->term_l->term_p = c;
		if (type != Regex::star) {
			c->term_r = term_r->copy();
			c->term_r->term_p = c;
		}
	}
	return c;
}

Regex::Regex(const Regex& reg)
	: BaseObject(reg.language), type(reg.type), value(reg.value),
	  term_p(nullptr), alphabet(reg.alphabet),
	  term_l(reg.term_l == nullptr ? nullptr : reg.term_l->copy()),
	  term_r(reg.term_r == nullptr ? nullptr : reg.term_r->copy()) {
	if (term_l) term_l->term_p = this;
	if (term_r) term_r->term_p = this;
}

Regex& Regex::operator=(const Regex& reg) {
	clear();
	language = reg.language;
	type = reg.type;
	value = reg.value;
	alphabet = reg.alphabet;
	if (reg.term_l) {
		term_l = reg.term_l->copy();
		term_l->term_p = this;
	}
	if (reg.term_r) {
		term_r = reg.term_r->copy();
		term_r->term_p = this;
	}
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

int Regex::search_replace_rec(const Regex& replacing, const Regex& replaced_by,
							  Regex* original) {
	int cond = 0;
	if (equal(replacing, *original)) {
		Regex* temp = new Regex(replaced_by);
		cond++;
		if (original->term_p && original->term_p->term_l &&
			original->term_p->term_l == original) {
			temp->term_p = original->term_p;
			original->term_p->term_l = temp;
		} else {
			if (original->term_p && original->term_p->term_r &&
				original->term_p->term_r == original) {
				temp->term_p = original->term_p;
				original->term_p->term_r = temp;
			}
		}
		delete original;
	} else {
		if (original->term_l) {
			cond +=
				search_replace_rec(replacing, replaced_by, original->term_l);
		}
		if (original->term_r) {
			cond +=
				search_replace_rec(replacing, replaced_by, original->term_r);
		}
	}
	return cond;
	//Привычка зарубать себе на носу довела Буратино до самоампутации органа
	//обоняния.
}
void Regex::normalize_this_regex(const string& file) {
	struct Rules {
		Regex from;
		Regex to;
	};
	vector<Rules> all_rules;
	string line;
	std::ifstream in(file);
	if (in.is_open()) {
		while (getline(in, line)) {
			string v1, v2;
			int ind = -1;
			for (char c : line) {
				if (c == '=') {
					ind = v1.size();
					continue;
				}
				if (c != ' ') {
					if (ind == -1) {
						v1 += c;
					} else {
						v2 += c;
					}
				}
			}
			if (v1 == "" || v2 == "") {
				cout << "error rewriting rules read from file";
				return;
			}
			Regex a(v1);
			Regex b(v2);

			Rules temp = {a, b};
			all_rules.push_back(temp);
		}
	}
	in.close();
	for (int i = 0; i < all_rules.size(); i++) {
		int cond = 0;
		cond += search_replace_rec(all_rules[i].from, all_rules[i].to, this);
		if (cond != 0) {
			i--;
		}
	}
}

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
	if (type == Type::symb /*value.symbol*/) symb = value.symbol;
	if (type == Type::eps) symb = "";
	if (type == Type::alt) {
		symb = '|';
		if (term_p != nullptr && term_p->type == Type::conc) {
			str1 = "(" + str1;
			str2 = str2 + ")"; // ставим скобки при альтернативах внутри
							   // конкатенации a(a|b)a
		}
	}
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
pair<vector<State>, int> Regex::get_tompson(int max_index) const {
	string str;			  //идентификатор состояния
	vector<State> s = {}; //вектор состояний нового автомата
	map<alphabet_symbol, set<int>> m, p, map_l, map_r; // словари автоматов
	set<int> trans; // новые транзишены
	int offset; // сдвиг для старых индексов состояний в новом автомате
	pair<vector<State>, int> al; // для левого автомата относительно операции
	pair<vector<State>, int> ar; // для правого автомата относительно операции
	Language* alp;				 // Новый язык для автомата
	switch (type) {
	case Regex::alt: // |

		al = term_l->get_tompson(max_index);
		ar = term_r->get_tompson(al.second);
		max_index = ar.second;

		str = "q" + to_string(max_index + 1);
		m[epsilon()] = {1, int(al.first.size()) + 1};
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
				map_l[epsilon()] = {int(al.first.size() + ar.first.size()) + 1};
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
				map_r[epsilon()] = {offset + int(ar.first.size())};
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
		al = term_l->get_tompson(max_index);
		ar = term_r->get_tompson(al.second);
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
		al = term_l->get_tompson(max_index);
		max_index = al.second;

		str = "q" + to_string(max_index + 1);
		m[epsilon()] = {1, int(al.first.size()) + 1};
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
				map_l[epsilon()] = {1, int(al.first.size()) + 1};
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

		m[epsilon()] = {1};
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

FiniteAutomaton Regex::to_tompson() const {
	Logger::init_step("Автомат Томпсона");
	FiniteAutomaton a(0, get_tompson(-1).first, language);
	Logger::log("", a);
	Logger::finish_step();
	return a;
}

int Regex::L() const {
	int l;
	int r;
	switch (type) {
	case Regex::alt:
		l = term_l->L();
		r = term_r->L();
		return l + r;
	case Regex::conc:
		l = term_l->L();
		r = term_r->L();
		if (l != 0 && r != 0) {
			return 1;
		}
		return 0;
	case Regex::star:
		return 1;
	case Regex::eps:
		return 1;
	default:
		return 0;
	}
}
vector<Lexem>* Regex::first_state() const {
	vector<Lexem>* l;
	vector<Lexem>* r;
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
		if (term_l->L() != 0) {
			r = term_r->first_state();
			l->insert(l->end(), r->begin(), r->end());
			delete r;
		}
		//
		return l;
	case Regex::eps:
		l = new vector<Lexem>;
		return l;
	default:
		l = new vector<Lexem>;
		l->push_back(value);
		return l;
	}
}

vector<Lexem>* Regex::end_state() const {
	vector<Lexem>* l;
	vector<Lexem>* r;
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
		if (term_r->L() != 0) {

			r = term_l->end_state();
			l->insert(l->end(), r->begin(), r->end());
			delete r;
		}
		return l;
	case Regex::eps:
		l = new vector<Lexem>;
		return l;
	default:
		l = new vector<Lexem>;
		l->push_back(value);
		return l;
	}
}

map<int, vector<int>> Regex::pairs() const {
	map<int, vector<int>> l;
	map<int, vector<int>> r;
	map<int, vector<int>> p;
	vector<Lexem>* rs;
	vector<Lexem>* ps;
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
bool Regex::is_term(int number, const vector<Lexem>& list) const {
	for (size_t i = 0; i < list.size(); i++) {
		if (list[i].number == number) {
			return true;
		}
	}
	return false;
}
Regex Regex::linearize() const {
	Logger::init_step("Linearise");
	Regex test(*this);
	vector<Regex*> list = test.pre_order_travers_vect();
	for (size_t i = 0; i < list.size(); i++) {
		list[i]->value.number = i;
	}
	Logger::log(test.to_txt());
	Logger::finish_step();
	return test;
}

Regex Regex::delinearize() const {
	Logger::init_step("DeLinearise");
	Regex test(*this);
	vector<Regex*> list = test.pre_order_travers_vect();
	for (size_t i = 0; i < list.size(); i++) {
		list[i]->value.number = 0;
	}
	Logger::log(test.to_txt());
	Logger::finish_step();
	return test;
}

FiniteAutomaton Regex::to_glushkov() const {

	Logger::init_step("Автомат Глушкова");

	Regex test(*this);
	vector<Regex*> list = test.pre_order_travers_vect();
	for (size_t i = 0; i < list.size(); i++) {
		list[i]->value.number = i;
	}
	vector<Lexem>* first = test.first_state(); // Множество начальных состояний
	vector<Lexem>* end = test.end_state(); // Множество конечных состояний
	int eps_in = test.L();
	map<int, vector<int>> p = test.pairs(); // Множество возможных пар состояний
	vector<State> st; // Список состояний в автомате
	map<alphabet_symbol, set<int>> tr; // мап для переходов в каждом состоянии

	string str_firs = "";
	string str_end = "";
	string str_pair = "";
	for (size_t i = 0; i < first->size(); i++) {
		str_firs = str_firs + (*first)[i].symbol +
				   to_string((*first)[i].number + 1) + " ";
	}

	for (size_t i = 0; i < end->size(); i++) {
		str_end =
			str_end + (*end)[i].symbol + to_string((*end)[i].number + 1) + " ";
	}
	for (auto& it1 : p) {
		for (size_t i = 0; i < it1.second.size(); i++) {
			str_pair = str_pair + list[it1.first]->value.symbol +
					   to_string(list[it1.first]->value.number + 1) +
					   list[it1.second[i]]->value.symbol +
					   to_string(list[it1.second[i]]->value.number + 1) + " ";
		}
	}

	if (eps) {
		str_end = "eps" + str_end;
	}

	// cout << test.to_str_log() << endl;
	// cout << "First " << str_firs << endl;
	// cout << "End " << str_end << endl;
	// cout << "Pairs " << str_pair << endl;

	Logger::log("Регулярка", test.to_str_log());
	Logger::log("First", str_firs);
	Logger::log("End", str_end);
	Logger::log("Pairs", str_pair);

	for (size_t i = 0; i < first->size(); i++) {
		tr[(*first)[i].symbol].insert((*first)[i].number + 1);
	}

	if (eps_in) {
		st.push_back(State(0, {}, "S", true, tr));
	} else {
		st.push_back(State(0, {}, "S", false, tr));
	}

	for (size_t i = 0; i < list.size(); i++) {
		Lexem elem = list[i]->value;
		tr = {};

		for (size_t j = 0; j < p[elem.number].size(); j++) {
			tr[list[p[elem.number][j]]->value.symbol].insert(p[elem.number][j] +
															 1);
		}
		string s = elem.symbol + to_string(i + 1);
		st.push_back(State(i + 1, {}, s, is_term(elem.number, (*end)), tr));
	}
	delete first;
	delete end;
	FiniteAutomaton a(0, st, language);
	Logger::log("", a);
	Logger::finish_step();
	return FiniteAutomaton(0, st, language);
}

FiniteAutomaton Regex::to_ilieyu() const {
	Logger::init_step("Автомат Илия–Ю");
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
		str_follow = str_follow + states[state_ind].identifier + ": ";
		for (auto j = states[state_ind].label.begin();
			 j != states[state_ind].label.end(); j++) {
			str_follow = str_follow + states[*j].identifier + " ";
		}
		str_follow = str_follow + ";\n";
	}

	// cout << str_follow;
	Logger::log("Автомат Глушкова", glushkov);
	Logger::log("Follow-отношения", str_follow);

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
	FiniteAutomaton a(0, new_states, glushkov.language);
	Logger::log("Итог", a);
	Logger::finish_step();
	return a;
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
					prefs->insert(*i + *j);
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
					prefs->insert(*i + *j);
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

bool Regex::derevative_with_respect_to_sym(Regex* respected_sym,
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
		answer1 = derevative_with_respect_to_sym(respected_sym, reg_e->term_l,
												 subresult);
		answer2 = derevative_with_respect_to_sym(respected_sym, reg_e->term_r,
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
		answer1 = derevative_with_respect_to_sym(respected_sym, reg_e->term_l,
												 *subresult.term_l);
		subresult.term_r = reg_e->term_r->copy();
		if (reg_e->term_l->is_eps_possible()) {
			answer2 = derevative_with_respect_to_sym(respected_sym,
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
		bool answer = derevative_with_respect_to_sym(
			respected_sym, reg_e->term_l, *result.term_l);
		result.term_r = reg_e->copy();
		return answer;
	}
}

bool Regex::partial_derevative_with_respect_to_sym(
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
		answer1 = partial_derevative_with_respect_to_sym(
			respected_sym, reg_e->term_l, subresult);
		answer2 = partial_derevative_with_respect_to_sym(
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
		answer1 = partial_derevative_with_respect_to_sym(
			respected_sym, reg_e->term_l, subresult);
		cur_subresult.term_r = reg_e->term_r->copy();
		for (int i = 0; i < subresult.size(); i++) {
			cur_subresult.term_l = subresult[i].copy();
			result.push_back(cur_subresult);
			delete cur_subresult.term_l;
			cur_subresult.term_l = nullptr;
		}
		if (reg_e->term_l->is_eps_possible()) {
			answer2 = partial_derevative_with_respect_to_sym(
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
		bool answer = partial_derevative_with_respect_to_sym(
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

bool Regex::derevative_with_respect_to_str(std::string str, const Regex* reg_e,
										   Regex& result) const {
	bool success = true;
	Regex cur = *reg_e;
	Regex next = *reg_e;
	// cout << "start getting derevative for prefix " << str << " in "
	//	 << reg_e->to_txt() << "\n";
	for (int i = 0; i < str.size(); i++) {
		auto sym = new Regex();
		sym->type = Type::symb;
		sym->value.symbol = str[i];
		next.clear();
		success &= derevative_with_respect_to_sym(sym, &cur, next);
		// cout << "derevative for prefix " << sym->to_txt() << " in "
		//	 << cur.to_txt() << " is " << next.to_txt() << "\n";
		delete sym;
		cur = next;
		if (!success) {
			break;
		}
	}
	result = next;
	// cout << " answer is " << result.to_txt();
	return success;
}

// Производная по символу
std::optional<Regex> Regex::symbol_derevative(
	const Regex& respected_sym) const {
	auto rs = respected_sym.copy();
	Regex result;
	std::optional<Regex> ans;
	if (derevative_with_respect_to_sym(rs, this, result))
		ans = result;
	else
		ans = nullopt;
	delete rs;
	return ans;
}
// Частичная производная по символу
void Regex::partial_symbol_derevative(const Regex& respected_sym,
									  vector<Regex>& result) const {
	auto rs = respected_sym.copy();
	partial_derevative_with_respect_to_sym(rs, this, result);
	delete rs;
	return;
}
// Производная по префиксу
std::optional<Regex> Regex::prefix_derevative(std::string respected_str) const {
	Regex result;
	std::optional<Regex> ans;
	if (derevative_with_respect_to_str(respected_str, this, result))
		ans = result;
	else
		ans = nullopt;
	return ans;
}
// Длина накачки
int Regex::pump_length() const {
	Logger::init_step("PumpLength");
	if (language->get_pump_length()) {
		Logger::log("Длина накачки",
					to_string(language->get_pump_length().value()));
		Logger::finish_step();
		return language->get_pump_length().value();
	}
	std::map<std::string, bool> checked_prefixes;
	for (int i = 1;; i++) {
		std::set<std::string> prefs;
		get_prefix(i, &prefs);
		if (prefs.empty()) {
			Logger::log(
				"Длина накачки совпадает с длиной регулярного выражения");
			Logger::finish_step();
			language->set_pump_length(-1); // TODO нужно оформить получение
										   // значения в отдельную фунцию
			// тогда и результат кэшировать проще, и логи делать
			return -1;
		}
		for (auto it = prefs.begin(); it != prefs.end(); it++) {
			bool was = false;
			for (int j = 0; j < it->size(); j++) {
				if (checked_prefixes[it->substr(0, j)]) {
					was = true;
					break;
				}
			}
			if (was) continue;
			for (int j = 0; j < it->size(); j++) {
				for (int k = j + 1; k <= it->size(); k++) {
					Regex pumping;
					std::string pumped_prefix;
					pumped_prefix += it->substr(0, j);
					pumped_prefix += "(" + it->substr(j, k - j) + ")*";
					pumped_prefix += it->substr(k, it->size() - k + j);
					pumping.type = Type::conc;
					pumping.term_l = new Regex(pumped_prefix);
					pumping.term_r = new Regex;
					if (!derevative_with_respect_to_str(*it, this,
														*pumping.term_r))
						continue;
					pumping.generate_alphabet(pumping.alphabet);
					pumping.language = make_shared<Language>(pumping.alphabet);
					// cout << pumped_prefix << " " << pumping.term_r->to_txt();
					if (subset(pumping)) {
						checked_prefixes[*it] = true;
						language->set_pump_length(i);
						/*cout << *it << "\n";
						cout << pumped_prefix << " " << pumping.term_r->to_txt()
							 << "\n";
						cout << to_txt() << "\n";*/
						Logger::log("Длина накачки", to_string(i));
						Logger::finish_step();
						return i;
					}
				}
			}
		}
	}
	Logger::log("Длина накачки совпадает с длиной регулярного выражения");
	Logger::finish_step();
	language->set_pump_length(-1);
	return -1;
}

bool Regex::equality_checker(const Regex* r1, const Regex* r2) {
	if (r1 == nullptr && r2 == nullptr) return true;
	if (r1 == nullptr || r2 == nullptr) return true;
	alphabet_symbol r1_value, r2_value;
	if (r1->value.symbol != "")
		r1_value = r1->value.symbol;
	else
		r1_value = to_string(r1->type);
	if (r2->value.symbol != "")
		r2_value = r2->value.symbol;
	else
		r2_value = to_string(r2->type);

	if (r1_value != r2_value) return false;

	return equality_checker(r1->term_l, r2->term_l) &&
			   equality_checker(r1->term_r, r2->term_r) ||
		   equality_checker(r1->term_r, r2->term_l) &&
			   equality_checker(r1->term_l, r2->term_r);
}

bool Regex::equal(const Regex& r1, const Regex& r2) {
	Logger::init_step("Equal");
	Logger::log("Первое регулярное выражение", r1.to_txt());
	Logger::log("Второе регулярное выражение", r2.to_txt());
	bool result = equality_checker(&r1, &r2);
	if (result)
		Logger::log("Результат Equal", "true");
	else
		Logger::log("Результат Equal", "false");
	Logger::finish_step();
	return result;
}

bool Regex::equivalent(const Regex& r1, const Regex& r2) {
	Logger::init_step("Equiv");
	Logger::log("Первое регулярное выражение", r1.to_txt());
	Logger::log("Второе регулярное выражение", r2.to_txt());
	FiniteAutomaton fa1 = r1.to_ilieyu();
	FiniteAutomaton fa2 = r2.to_ilieyu();
	bool result = FiniteAutomaton::equivalent(fa1, fa2);
	if (result)
		Logger::log("Результат Equiv", "true");
	else
		Logger::log("Результат Equiv", "false");
	Logger::finish_step();
	return result;
}

bool Regex::subset(const Regex& r) const {
	Logger::init_step("Subset");
	Logger::log("Первое регулярное выражение", to_txt());
	Logger::log("Второе регулярное выражение", r.to_txt());
	FiniteAutomaton dfa1(to_ilieyu().determinize());
	FiniteAutomaton dfa2(r.to_ilieyu().determinize());
	FiniteAutomaton dfa_instersection(
		FiniteAutomaton::intersection(dfa1, dfa2));
	bool result = FiniteAutomaton::equivalent(dfa_instersection, dfa2);
	if (result)
		Logger::log("Результат Subset", "true");
	else
		Logger::log("Результат Subset", "false");
	Logger::finish_step();
	return result;
}

FiniteAutomaton Regex::to_antimirov() const {
	Logger::init_step("Автомат Антимирова");
	map<set<string>, set<string>> trans;
	vector<Regex> states;
	vector<Regex> alph_regex;
	vector<vector<Regex>> out;
	set<string> check;
	for (set<alphabet_symbol>::iterator i = alphabet.begin();
		 i != alphabet.end(); i++) {
		string symbol = *i;
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
			state.partial_symbol_derevative(*j, regs);
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

	string derev_log = "";

	for (size_t i = 0; i < name_states.size(); i++) {
		string state = name_states[i];
		map<alphabet_symbol, set<int>> transit;
		for (size_t j = 0; j < out.size(); j++) {
			// cout << out[j][0].to_txt() << " ";
			// cout << out[j][1].to_txt() << " ";
			// cout << out[j][2].to_txt() << endl;
			derev_log = out[j][2].to_txt() + "(" + out[j][0].to_txt() + ")" +
						" = " + out[j][1].to_txt() + "\\";
			Logger::log(derev_log);
			if (out[j][0].to_txt() == state) {
				auto n = find(name_states.begin(), name_states.end(),
							  out[j][1].to_txt());
				alphabet_symbol s = out[j][2].to_txt();
				transit[s].insert(n - name_states.begin());
			}
		}

		if ((state.size() == 0) || (states[i].L())) {
			automat_state.push_back({int(i), {}, state, true, transit});
		} else {
			automat_state.push_back({int(i), {}, state, false, transit});
		}
	}
	string str_state = "State: ";
	for (size_t i = 0; i < automat_state.size(); i++) {
		str_state += automat_state[i].identifier + " ";
	}

	// cout << derev_log;
	// cout << str_state << endl;
	// Logger::log(derev_log, str_state);
	Logger::log(str_state);

	FiniteAutomaton a(0, automat_state, language);
	Logger::log("Итог", a);
	Logger::finish_step();
	return a;
}

string Regex::to_str_log() const {
	string str1 = "", str2 = "";
	if (term_l) {
		str1 = term_l->to_str_log();
	}
	if (term_r) {
		str2 = term_r->to_str_log();
	}
	string symb;
	if (type == Type::symb /*value.symbol*/)
		symb = value.symbol + to_string(value.number + 1);
	if (type == Type::eps) symb = "";
	if (type == Type::alt) {
		symb = '|';
		if (term_p != nullptr && term_p->type == Type::conc) {
			str1 = "(" + str1;
			str2 = str2 + ")"; // ставим скобки при альтернативах внутри
							   // конкатенации a(a|b)a
		}
	}
	if (type == Type::star) {
		symb = '*';
		if (term_l->type != Type::symb)
			str1 = "(" + str1 +
				   ")"; // ставим скобки при итерации, если символов > 1
	}

	return str1 + symb + str2;
}

Regex Regex::deannote() const {
	Logger::init_step("DeAnnote");
	Regex old_regex(*this);
	Logger::log("Регулярное выражение до преобразования", old_regex.to_txt());
	string with_number = old_regex.to_txt();
	string new_string;
	for (size_t i = 0; i < with_number.size(); i++) {
		if (!('0' <= with_number[i] && with_number[i] <= '9')) {
			new_string += with_number[i];
		}
	}
	Regex new_regex(new_string);
	Logger::log("Регулярное выражение после преобразования",
				new_regex.to_txt());
	Logger::finish_step();
	return new_regex;
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