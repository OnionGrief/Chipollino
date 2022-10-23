#include "Regex.h"
#include <set>

Lexem::Lexem(Type type, char symbol) : type(type), symbol(symbol) {}

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
			 (lexems.back().type == Lexem::alt && lexem.type == Lexem::parR))) {
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
			return p;
		}
	}
	return nullptr;
}

Regex* Regex::scan_symb(const vector<Lexem>& lexems, int index_start,
						int index_end) {
	Regex* p = nullptr;
	if (lexems.size() <= (index_start) ||
		lexems[index_start].type != Lexem::symb) {
		return nullptr;
	}
	p = new Regex;
	p->value = lexems[index_start];
	p->type = Regex::symb;
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
Regex::Regex() {}

bool Regex::from_string(string str) {
	vector<Lexem> l = parse_string(str);
	Regex* root = expr(l, 0, l.size());
	if (root == nullptr || root->type == eps) {
		return false;
	}
	*this = *root;
	if (term_l != nullptr) {
		term_l->term_p = this;
	}
	if (term_r != nullptr) {
		term_r->term_p = this;
	}
	delete root;
	return true;
}

Regex* Regex::copy() const {
	Regex* c = new Regex();
	c->type = type;
	c->value = value;
	if (type != Regex::eps && type != Regex::symb) {
		c->term_l = term_l->copy();
		if (type != Regex::star) c->term_r = term_r->copy();
	}
	return c;
}

Regex::Regex(const Regex& reg)
	: type(reg.type), value(reg.value),
	  term_l(reg.term_l == nullptr ? nullptr : reg.term_l->copy()),
	  term_r(reg.term_r == nullptr ? nullptr : reg.term_r->copy()) {}

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

Regex::~Regex() {
	clear();
}

void Regex::pre_order_travers() {
	if (value.symbol) {
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

void Regex::get_prefix(int len, std::set<std::string>* prefs) const {
	std::set<std::string>*prefs1, *prefs2;
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
										   Regex* result) const {
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
		answer &= derevative_with_respect_to_sym(respected_sym, reg_e->term_l,
												 result->term_l);
		answer &= derevative_with_respect_to_sym(respected_sym, reg_e->term_r,
												 result->term_r);
		return answer;
	case Type::conc:
		subresult = new Regex();
		subresult->type = Type::conc;
		answer &= derevative_with_respect_to_sym(respected_sym, reg_e->term_l,
												 subresult->term_l);
		subresult->term_r = reg_e->copy();
		if (reg_e->term_l->is_eps_possible()) {
			result = new Regex();
			result->type = Type::alt;
			result->term_l = subresult;
			answer &= derevative_with_respect_to_sym(
				respected_sym, reg_e->term_r, result->term_r);
		} else {
			result = subresult;
		}
		return answer;
	case Type::star:
		result = new Regex();
		result->type = Type::conc;
		bool answer = derevative_with_respect_to_sym(
			respected_sym, reg_e->term_l, result->term_l);
		result->term_r = reg_e->copy();
		return answer;
	}
}

bool Regex::derevative_with_respect_to_str(std::string str, const Regex* reg_e,
										   Regex* result) const {
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

// Производная по символу
std::optional<Regex> Regex::symbol_derevative(
	const Regex& respected_sym) const {
	auto rs = respected_sym.copy();
	Regex* result = new Regex;
	std::optional<Regex> ans;
	if (derevative_with_respect_to_sym(rs, this, result))
		ans = *result;
	else
		ans = nullopt;
	delete rs;
	delete result;
	return ans;
}
// Производная по префиксу
std::optional<Regex> Regex::prefix_derevative(std::string respected_str) const {
	Regex* result = new Regex;
	std::optional<Regex> ans;
	if (derevative_with_respect_to_str(respected_str, this, result))
		ans = *result;
	else
		ans = nullopt;
	delete result;
	return ans;
}
// Длина накачки
int Regex::pump_length() const {
	std::map<std::string, bool> checked_prefixes;
	for (int i = 0;; i++) {
		std::set<std::string> prefs;
		get_prefix(i, &prefs);
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
				for (int k = j + 1; k < it->size(); k++) {
					Regex pumping;
					std::string pumped_prefix;
					pumped_prefix += it->substr(0, j);
					pumped_prefix += "(" + it->substr(j, k - j) + ")*";
					pumped_prefix += it->substr(j, it->size() - k);
					pumping.type = Type::conc;
					pumping.term_l = new Regex;
					pumping.term_l->from_string(pumped_prefix);
					pumping.term_r = new Regex;
					derevative_with_respect_to_str(*it, this, pumping.term_r);
					if (true) { // TODO: check if pumping language belongs reg_e
								// language
						checked_prefixes[*it] = true;
						return i;
					}
				}
			}
		}
	}
	return -1;
}

bool Regex::equal(Regex* r1, Regex* r2) {
	if (r1 == nullptr && r2 == nullptr) return true;
	if (r1 == nullptr || r2 == nullptr) return true;
	int r1_value, r2_value;
	if (r1->value.symbol)
		r1_value = (int)r1->value.symbol;
	else
		r1_value = r1->type;
	if (r2->value.symbol)
		r2_value = (int)r2->value.symbol;
	else
		r2_value = r2->type;

	if (r1_value != r2_value) return false;

	return equal(r1->term_l, r2->term_l) && equal(r1->term_r, r2->term_r) ||
		   equal(r1->term_r, r2->term_l) && equal(r1->term_l, r2->term_r);
}
