#include "Regex.h"
#include <set>

Lexem::Lexem(Type type, char symbol, int number)
	: type(type), symbol(symbol), number(number) {}

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

Regex* Regex::scan_conc(vector<Lexem> lexems, int index_start, int index_end) {
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

Regex* Regex::scan_star(vector<Lexem> lexems, int index_start, int index_end) {
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

Regex* Regex::scan_alt(vector<Lexem> lexems, int index_start, int index_end) {
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

Regex* Regex::scan_symb(vector<Lexem> lexems, int index_start, int index_end) {
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

Regex* Regex::scan_eps(vector<Lexem> lexems, int index_start, int index_end) {
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

Regex* Regex::scan_par(vector<Lexem> lexems, int index_start, int index_end) {
	Regex* p = nullptr;

	if (lexems.size() <= (index_end - 1) ||
		(lexems[index_start].type != Lexem::parL ||
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

Regex* Regex::copy() {
	Regex* c = new Regex();
	c->type = type;
	c->value = value;
	if (/*type != regex_cell_state::epsilon && У нас нет лексемы пустоты*/
		type != Regex::symb) {
		c->term_l = term_l->copy();
		if (type != Regex::conc) c->term_r = term_r->copy();
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

FiniteAutomat Regex::to_tompson(int max_index) {
	string str;		 //идентификатор состояния
	FiniteAutomat a; // новый автомат
	vector<State> s = {}; //вектор состояний нового автомата
	vector<char> alfa = {}; //алфавит новго автомата
	map<char, vector<int>> m, p, map_l, map_r; // словари автоматов
	vector<int> trans; // новые транзишены
	int offset; // сдвиг для старых индексов состояний в новом автомате
	FiniteAutomat al; // левый автомат относительно операции
	FiniteAutomat ar; // левый автомат относительно операции
	set<char> alfas; // множество алфавита для удаления дубликатов в нем
	switch (type) {
	case Regex::alt: // |

		al = term_l->to_tompson(max_index);
		ar = term_r->to_tompson(al.max_index);
		max_index = ar.max_index;

		str = "q" + to_string(max_index + 1);
		m['\0'] = {1, int(al.states.size()) + 1};
		s.push_back(State(0, {}, str, false, m));

		for (size_t i = 0; i < al.states.size(); i++) {
			State test;
			test = al.states[i];
			for (auto el : test.transitions) {
				char elem = el.first; // al->alphabet[i];
				if (elem != '\0') {
					alfa.push_back(elem);
				}
				trans = {};
				for (size_t j = 0; j < test.transitions[elem].size(); j++) {
					trans.push_back(test.transitions[elem][j] + 1);
				}
				map_l[elem] = trans;
			}

			if (test.is_terminal) {
				map_l['\0'] = {int(al.states.size() + ar.states.size()) + 1};
			}
			s.push_back(State(al.states[i].index + 1, {},
							  al.states[i].identifier, false, map_l));
			map_l = {};
		}
		offset = s.size();
		for (size_t i = 0; i < ar.states.size(); i++) {
			State test;
			test = ar.states[i];
			for (auto el : test.transitions) {
				char elem = el.first;
				if (elem != '\0') {
					alfa.push_back(elem);
				}
				trans = {};
				for (size_t j = 0; j < test.transitions[elem].size(); j++) {
					trans.push_back(test.transitions[elem][j] + offset);
				}
				map_r[elem] = trans;
			}
			if (test.is_terminal) {
				map_r['\0'] = {offset + int(ar.states.size())};
			}

			s.push_back(State(ar.states[i].index + offset, {},
							  ar.states[i].identifier, false, map_r));
			map_r = {};
		}

		str = "q" + to_string(max_index + 2);
		s.push_back(State(int(al.states.size() + ar.states.size()) + 1, {}, str,
						  true, p));
		alfas = set(alfa.begin(), alfa.end());
		alfa.assign(alfas.begin(), alfas.end());

		a = FiniteAutomat(0, alfa, s, false);
		a.max_index = max_index + 2;
		return a;
	case Regex::conc: // .
		al = term_l->to_tompson(max_index);
		ar = term_r->to_tompson(al.max_index);
		max_index = ar.max_index;

		for (size_t i = 0; i < al.states.size(); i++) {
			State test;
			test = al.states[i];
			for (auto el : test.transitions) {
				char elem = el.first; // al->alphabet[i];
				if (elem != '\0') {
					alfa.push_back(elem);
				}
				trans = {};
				for (size_t j = 0; j < test.transitions[elem].size(); j++) {
					trans.push_back(test.transitions[elem][j]);
				}
				map_l[elem] = trans;
			}

			if (test.is_terminal) {
				State test_r = ar.states[0];
				for (auto el : test_r.transitions) {
					char elem = el.first; // al->alphabet[i];
					if (elem != '\0') {
						alfa.push_back(elem);
					}
					for (size_t j = 0; j < test_r.transitions[elem].size();
						 j++) {
						// trans.push_back(test.transitions[elem][j] + 1);
						map_l[elem].push_back(test_r.transitions[elem][j] +
											  al.states.size() - 1);
					}
					// map_l[elem] = trans;
				}
			}
			// cout << al->states[i].identifier << " " << al->states[i].index
			// <<"\n";
			s.push_back(State(al.states[i].index, {}, al.states[i].identifier,
							  false, map_l));
			map_l = {};
		}
		offset = s.size();
		for (size_t i = 1; i < ar.states.size(); i++) {
			State test;
			test = ar.states[i];
			for (auto el : test.transitions) {
				char elem = el.first; // al->alphabet[i];
				if (elem != '\0') {
					alfa.push_back(elem);
				}
				trans = {};
				// alfa.push_back(elem);
				for (size_t j = 0; j < test.transitions[elem].size(); j++) {
					trans.push_back(test.transitions[elem][j] + offset - 1);
				}
				map_r[elem] = trans;
			}

			s.push_back(State(ar.states[i].index + offset - 1, {},
							  ar.states[i].identifier, test.is_terminal,
							  map_r));
			map_r = {};
		}

		alfas = set(alfa.begin(), alfa.end());
		alfa.assign(alfas.begin(), alfas.end());

		a = FiniteAutomat(0, alfa, s, false);
		a.max_index = max_index;
		return a;
	case Regex::star: // *
		al = term_l->to_tompson(max_index);
		max_index = al.max_index;

		str = "q" + to_string(max_index + 1);
		m['\0'] = {1, int(al.states.size()) + 1};
		s.push_back(State(0, {}, str, false, m));

		for (size_t i = 0; i < al.states.size(); i++) {
			State test;
			test = al.states[i];
			for (auto el : test.transitions) {
				char elem = el.first; // al->alphabet[i];
				if (elem != '\0') {
					alfa.push_back(elem);
				}
				trans = {};
				for (size_t j = 0; j < test.transitions[elem].size(); j++) {
					trans.push_back(test.transitions[elem][j] + 1);
				}
				map_l[elem] = trans;
			}

			if (test.is_terminal) {
				map_l['\0'] = {1, int(al.states.size()) + 1};
			}
			s.push_back(State(al.states[i].index + 1, {},
							  al.states[i].identifier, false, map_l));
			map_l = {};
		}
		offset = s.size();

		str = "q" + to_string(max_index + 2);
		s.push_back(State(int(al.states.size()) + 1, {}, str, true, p));
		alfas = set(alfa.begin(), alfa.end());
		alfa.assign(alfas.begin(), alfas.end());
		// alfa.erase( unique( alfa.begin(), alfa.end() ), alfa.end() );

		a = FiniteAutomat(0, alfa, s, false);
		a.max_index = max_index + 2;
		return a;
	case Regex::eps:
		str = "q" + to_string(max_index + 1);

		m['\0'] = {1};
		s.push_back(State(0, {}, str, false, m));
		str = "q" + to_string(max_index + 2);
		s.push_back(State(1, {}, str, true, p));

		a = FiniteAutomat(0, {}, s, false);
		a.max_index = max_index + 2;
		return a;
	default:

		str = "q" + to_string(max_index + 1);

		m[value.symbol] = {1};
		s.push_back(State(0, {}, str, false, m));
		str = "q" + to_string(max_index + 2);
		s.push_back(State(1, {}, str, true, p));

		a = FiniteAutomat(0, {value.symbol}, s, false);
		a.max_index = max_index + 2;
		return a;
	}
	return FiniteAutomat();
}
int Regex::L() {
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
		if (l != 0 and r != 0) {
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
vector<Lexem>* Regex::first_state() {
	vector<Lexem>* l;
	vector<Lexem>* r;
	switch (type) {
	case Regex::alt:
		l = term_l->first_state();
		r = term_r->first_state();
		l->insert(l->end(), r->begin(), r->end());
		return l;
	case Regex::star:
		l = term_l->first_state();
		return l;
	case Regex::conc:
		l = term_l->first_state();
		if (term_l->L() != 0) {
			r = term_r->first_state();
			l->insert(l->end(), r->begin(), r->end());
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

vector<Lexem>* Regex::end_state() {
	vector<Lexem>* l;
	vector<Lexem>* r;
	switch (type) {
	case Regex::alt:
		l = term_l->end_state();
		r = term_r->end_state();
		l->insert(l->end(), r->begin(), r->end());
		return l;
	case Regex::star:
		l = term_l->end_state();
		return l;
	case Regex::conc:
		l = term_r->end_state();
		if (term_r->L() != 0) {

			r = term_l->end_state();
			l->insert(l->end(), r->begin(), r->end());
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

map<int, vector<int>> Regex::pairs() {
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
		return l;
	default:
		break;
	}
	return {};
}

vector<Regex*> Regex::pre_order_travers_vect() {
	vector<Regex*> r;
	vector<Regex*> ret;
	if (value.symbol) {
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
bool Regex::is_term(int number, vector<Lexem> list) {
	for (size_t i = 0; i < list.size(); i++) {
		if (list[i].number == number) {
			return true;
		}
	}
	return false;
}
FiniteAutomat Regex::to_glushkov() {

	vector<Regex*> list = this->pre_order_travers_vect();
	for (size_t i = 0; i < list.size(); i++) {
		list[i]->value.number = i;
	}
	vector<Lexem>* first = this->first_state();
	vector<Lexem>* end = this->end_state();
	map<int, vector<int>> p = this->pairs();

	int index = 0;
	vector<char> alph;
	vector<State> st;
	set<char> alfas;
	for (size_t i = 0; i < list.size(); i++) {
		alph.push_back(list[i]->value.symbol);
	}

	alfas = set(alph.begin(), alph.end());
	alph.assign(alfas.begin(), alfas.end());
	map<char, vector<int>> tr;
	for (size_t i = 0; i < first->size(); i++) {
		tr[(*first)[i].symbol].push_back((*first)[i].number + 1);
	}

	first;
	st.push_back(State(0, {}, "S", false, tr));

	for (size_t i = 0; i < list.size(); i++) {
		Lexem elem = list[i]->value;
		tr = {};

		for (size_t j = 0; j < p[elem.number].size(); j++) {
			tr[list[p[elem.number][j]]->value.symbol].push_back(
				p[elem.number][j] + 1);
			set<int> s(tr[list[p[elem.number][j]]->value.symbol].begin(),
					   tr[list[p[elem.number][j]]->value.symbol].end());
			tr[list[p[elem.number][j]]->value.symbol].assign(s.begin(),
															 s.end());
		}
		string s = elem.symbol + to_string(i + 1);
		st.push_back(State(i + 1, {}, s, is_term(elem.number, (*end)), tr));
	}
	return FiniteAutomat(0, alph, st, false);
}

FiniteAutomat Regex::to_ilieyu() {
	FiniteAutomat glushkov = this->to_glushkov();
	vector<State> states = glushkov.states;
	vector<int> follow;
	for (size_t i = 0; i < states.size(); i++) {
		State st1 = states[i];
		map<char, vector<int>> map1 = st1.transitions;
		for (size_t j = i + 1; j < states.size(); j++) {
			State st2 = states[j];
			map<char, vector<int>> map2 = st2.transitions;
			bool flag = true;
			if (i == j || map2.size() != map1.size()) {
				continue;
			}

			for (auto& it1 : map1) {
				vector<int> v1 = it1.second;
				vector<int> v2 = map2[it1.first];
				sort(v1.begin(), v1.end());
				sort(v2.begin(), v2.end());
				if (v1 != v2 /*equal(v1.begin(), v1.end(), v2.begin())*/) {
					flag = false;
					break;
				}
			}
			if (flag) {
				follow.push_back(j);
				states[i].label.push_back(j);
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

	for (size_t i = 0; i < new_states.size(); i++) {
		State v1 = new_states[i];
		map<char, vector<int>> old_map = v1.transitions;
		map<char, vector<int>> new_map;
		for (auto& it1 : old_map) {
			vector<int> v1 = it1.second;
			for (size_t j = 0; j < v1.size(); j++) {
				for (size_t k = 0; k < new_states.size(); k++) {
					if (find(new_states[k].label.begin(),
							 new_states[k].label.end(),
							 v1[j]) != new_states[k].label.end() ||
						v1[j] == new_states[k].index) {
						new_map[it1.first].push_back(k);
					}
				}
			}
		}
		new_states[i].transitions = new_map;
	}

	for (size_t i = 0; i < new_states.size(); i++) {
		new_states[i].index = i;
	}

	return FiniteAutomat(0, glushkov.alphabet, new_states, false);
}