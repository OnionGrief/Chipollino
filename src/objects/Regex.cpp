#include "Regex.h"

Lexem::Lexem(Type type, char symbol)
	: type(type), symbol(symbol) {}

vector<Lexem> Regex::parse_string(string str) {
	vector<Lexem> lexems;
	lexems = {};

	auto is_symbol = [](char c) {
		return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
	};

	for (const char& c : str) {
		Lexem lexem;
		switch (c) {
		case '(':
			lexem.type = Lexem::parL;
			break;
		case ')':
			lexem.type = Lexem::parR;
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
				lexem.symbol = c;
			}
			else {
				lexem.type = Lexem::error;
				lexems = {};
				lexems.push_back(lexem);
				return lexems;
			}
			break;
		}

		if (lexems.size() && (
			// Lexem left
			lexems.back().type == Lexem::symb ||
			lexems.back().type == Lexem::star ||
			lexems.back().type == Lexem::parR) && (
				// Lexem right
				lexem.type == Lexem::symb ||
				lexem.type == Lexem::parL)) {

			// We place . between
			lexems.push_back({ Lexem::conc });
		}

		lexems.push_back(lexem);
	}
	return lexems;
}

Regex* Regex::scan_conc(vector<Lexem> lexems, int index_start, int index_end) {
	Regex* p = nullptr;
	int balance = 0;
	for (int i = index_start; i < index_end; i++)
	{
		if (lexems[i].type == Lexem::parL) { // LEFT_BRACKET
			balance++;
		}
		if (lexems[i].type == Lexem::parR) { // RIGHT_BRACKET
			balance--;
		}
		if (lexems[i].type == Lexem::conc && balance == 0) {
			Regex* l = expr(lexems, index_start, i);
			Regex* r = expr(lexems, i + 1, index_end);


			if (l == nullptr || r == nullptr) { // Проверка на адекватность)
				return p;
			}

			p = new Regex;
			l->term_p = p;
			r->term_p = p;
			p->term_l = l;
			p->term_r = r;
			p->value = lexems[i];
			p->type = Regex::conc;
			return p;
		}
	}
	return nullptr;
}

Regex* Regex::scan_star(vector<Lexem> lexems, int index_start, int index_end) {
	Regex* p = nullptr;
	int balance = 0;
	for (int i = index_start; i < index_end; i++)
	{
		if (lexems[i].type == Lexem::parL) { // LEFT_BRACKET
			balance++;
		}
		if (lexems[i].type == Lexem::parR) { // RIGHT_BRACKET
			balance--;
		}
		if (lexems[i].type == Lexem::star && balance == 0) {
			Regex* l = expr(lexems, index_start, i);

			if (l == nullptr) {
				return p;
			}

			p = new Regex;
			l->term_p = p;
			p->term_l = l;

			p->term_r = nullptr;
			p->value = lexems[i];
			p->type = Regex::star;
			return p;
		}
	}
	return nullptr;
}

Regex* Regex::scan_alt(vector<Lexem> lexems, int index_start, int index_end) {
	Regex* p = nullptr;
	int balance = 0;
	for (int i = index_start; i < index_end; i++)
	{
		if (lexems[i].type == Lexem::parL) { // LEFT_BRACKET
			balance++;
		}
		if (lexems[i].type == Lexem::parR) { // RIGHT_BRACKET
			balance--;
		}
		if (lexems[i].type == Lexem::alt && balance == 0) {
			Regex* l = expr(lexems, index_start, i);
			Regex* r = expr(lexems, i + 1, index_end);
			if ((l == nullptr) || (r == nullptr)) { // Проверка на адекватность)
				return nullptr;
			}

			p = new Regex;
			l->term_p = p;
			r->term_p = p;
			p->term_l = l;
			p->term_r = r;

			p->value = lexems[i];
			p->type = Regex::alt;
			return p;
		}
	}
	return nullptr;
}

Regex* Regex::scan_symb(vector<Lexem> lexems, int index_start, int index_end) {
	Regex* p = nullptr;
	if (lexems.size() <= (index_start) || lexems[index_start].type != Lexem::symb) {
		return nullptr;
	}
	p = new Regex;
	p->value = lexems[index_start];
	p->type = Regex::symb;
	return p;
}

Regex* Regex::scan_par(vector<Lexem> lexems, int index_start, int index_end) {
	Regex* p = nullptr;

	if (lexems.size() <= (index_end - 1) || (lexems[index_start].type != Lexem::parL ||
		lexems[index_end - 1].type != Lexem::parR)) {
		return nullptr;
	}
	p = expr(lexems, index_start + 1, index_end - 1);
	return p;
}
Regex* Regex::expr(vector<Lexem> lexems, int index_start, int index_end) {
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
		p = scan_par(lexems, index_start, index_end);
	}
	return p;
}
Regex::Regex() {}

bool Regex::from_string(string str) {
	vector<Lexem> l = parse_string(str);
	Regex* root = expr(l, 0, l.size());
	if (root == nullptr) {
		return true;
	}
	*this = *root;
	if (term_l != nullptr) {
		term_l->term_p = this;
	}
	if (term_r != nullptr) {
		term_r->term_p = this;
	}	
	delete root;
	return false;
}

Regex* Regex::copy() {
	Regex* c = new Regex();
	c->type = type;
	c->value = value;
	if (type != Type::eps && type != Regex::symb) {
		c->term_l = term_l->copy();
		if (type != Regex::conc)
			c->term_r = term_r->copy();
	}
	return c;
}

void Regex::clear() {
	if (term_l != nullptr) {
		term_l->clear();
		delete term_l;
	}
	if (term_r != nullptr) {
		term_r->clear();
		delete term_r;
	}
}

void Regex::pre_order_travers() {
	if (value.symbol) {
		cout << value.symbol << " ";
	}
	else {
		cout << type << " ";
	}
	if (term_l) {
		term_l->pre_order_travers();
	}
	if (term_r) {
		term_r->pre_order_travers();
	}
}

string Regex::to_txt() {
	return string();
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

void Regex::get_prefix(int len, std::set<std::string>* prefs) {
	std::set<std::string>* prefs1, * prefs2;
	switch (type)
	{
	case Type::eps:
		if (len == 0)
			prefs->insert("");
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

bool Regex::derevative_with_respect_to_sym(Regex* respected_sym, Regex* reg_e, Regex* result) {
	if (respected_sym->type != Type::eps && respected_sym->type != Type::symb) {
		std::cout << "Invalid input: unexpected regex instead of symbol\n";
		return false;
	}
	if (respected_sym->type == Type::eps) {
		result = reg_e->copy();
		return true;
	}
	Regex* subresult;
	bool answer = true;
	switch (reg_e->type) {
	case Type::eps:
		result = new Regex();
		return answer;
	case Type::symb:
		if (respected_sym->value.symbol != reg_e->value.symbol) {
			std::cout << "Invalid input: symbol is not a prefix of regex\n";
			return false;
		}
		result = new Regex();
		return answer;
	case Type::alt:
		result = new Regex();
		result->type = Type::alt;
		answer &= derevative_with_respect_to_sym(respected_sym, reg_e->term_l, result->term_l);
		answer &= derevative_with_respect_to_sym(respected_sym, reg_e->term_r, result->term_r);
		return answer;
	case Type::conc:
		subresult = new Regex();
		subresult->type = Type::conc;
		answer &= derevative_with_respect_to_sym(respected_sym, reg_e->term_l, subresult->term_l);
		subresult->term_r = reg_e->copy();
		if (reg_e->term_l->is_eps_possible()) {
			result = new Regex();
			result->type = Type::alt;
			result->term_l = subresult;
			answer &= derevative_with_respect_to_sym(respected_sym, reg_e->term_r, result->term_r);
		}
		else {
			result = subresult;
		}
		return answer;
	case Type::star:
		result = new Regex();
		result->type = Type::conc;
		bool answer = derevative_with_respect_to_sym(respected_sym, reg_e->term_l, result->term_l);
		result->term_r = reg_e->copy();
		return answer;
	}
}

bool Regex::derevative_with_respect_to_str(std::string str, Regex* reg_e, Regex* result) {
	bool success = true;
	auto cur = reg_e->copy();
	Regex* next;
	for (int i = 0; i < str.size(); i++) {
		auto sym = new Regex();
		sym->type = Type::symb;
		sym->value.symbol = str[i];
		success &= derevative_with_respect_to_sym(sym, cur, next);
		delete cur;
		delete sym;
		cur = next;
		if (!success) {
			break;
		}
	}
	result = next;
	return success;
}

void Regex::pump_lenght(Regex* reg_e) {
	for (int i = 0; i < 5; i++) {
		std::set<std::string> prefs;
		reg_e->get_prefix(i, &prefs);
		std::cout << i << "-prefix:\n";
		for (auto it = prefs.begin(); it != prefs.end(); it++) {
			std::cout << "   " << *it << "\n";
		}
	}
}