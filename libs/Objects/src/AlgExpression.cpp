#include <algorithm>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "Objects/AlgExpression.h"
#include "Objects/Language.h"

using std::cout;
using std::endl;
using std::make_shared;
using std::map;
using std::set;
using std::string;
using std::to_string;
using std::unordered_map;
using std::vector;

AlgExpression::AlgExpression() {
	type = Type::eps;
}

AlgExpression::Lexeme::Lexeme(Type type, const Symbol& symbol, int number)
	: type(type), symbol(symbol), number(number) {}

AlgExpression::AlgExpression(std::shared_ptr<Language> language, Type type, const Symbol& symbol,
							 Alphabet alphabet)
	: BaseObject(std::move(language)), type(type), symbol(symbol), alphabet(std::move(alphabet)) {}

AlgExpression::AlgExpression(Type type, const Symbol& symbol) : type(type), symbol(symbol) {}

AlgExpression::AlgExpression(Alphabet alphabet) : BaseObject(std::move(alphabet)) {}

AlgExpression::AlgExpression(Type type, AlgExpression* _term_l, AlgExpression* _term_r)
	: type(type) {
	if (_term_l) {
		term_l = _term_l->make_copy();
		alphabet = term_l->alphabet;
	}
	if (_term_r) {
		term_r = _term_r->make_copy();
		alphabet.insert(term_r->alphabet.begin(), term_r->alphabet.end());
	}
}

void AlgExpression::clear() {
	if (term_l) {
		delete term_l;
		term_l = nullptr;
	}
	if (term_r) {
		delete term_r;
		term_r = nullptr;
	}
}

AlgExpression::~AlgExpression() {
	clear();
}

AlgExpression::AlgExpression(const AlgExpression& other) : AlgExpression() {
	alphabet = other.alphabet;
	type = other.type;
	symbol = other.symbol;
	language = other.language;
	if (other.term_l)
		term_l = other.term_l->make_copy();
	if (other.term_r)
		term_r = other.term_r->make_copy();
}

Symbol AlgExpression::get_symbol() const {
	return symbol;
}

AlgExpression::Type AlgExpression::get_type() const {
	return type;
}

AlgExpression* AlgExpression::get_term_l() const {
	return term_l;
}

AlgExpression* AlgExpression::get_term_r() const {
	return term_r;
}

void AlgExpression::set_language(const Alphabet& _alphabet) {
	alphabet = _alphabet;
	language = make_shared<Language>(alphabet);
}

void AlgExpression::set_language(const std::shared_ptr<Language>& _language) {
	language = _language;
}

void AlgExpression::generate_alphabet() {
	if (type == Type::symb) {
		alphabet = {symbol};
		return;
	}
	alphabet.clear();
	if (term_l) {
		term_l->generate_alphabet();
		alphabet = term_l->alphabet;
	}
	if (term_r) {
		term_r->generate_alphabet();
		alphabet.insert(term_r->alphabet.begin(), term_r->alphabet.end());
	}
}

void AlgExpression::make_language() {
	generate_alphabet();
	language = make_shared<Language>(alphabet);
}

bool AlgExpression::is_terminal_type(Type t) {
	return t == Type::symb || t == Type::memoryWriter || t == Type::ref;
}

string AlgExpression::to_txt() const {
	string str1, str2;
	if (term_l) {
		str1 = term_l->to_txt();
	}
	if (term_r) {
		str2 = term_r->to_txt();
	}
	string symb;
	switch (type) {
	case Type::conc:
		if (term_l && term_l->type == Type::alt) {
			str1 = "(" + str1 + ")";
		}
		if (term_r && term_r->type == Type::alt) {
			str2 = "(" + str2 + ")";
		}
		break;
	case Type::symb:
		symb = symbol;
		break;
	case Type::eps:
		break;
	case Type::alt:
		symb = '|';
		break;
	case Type::star:
		symb = '*';
		if (!is_terminal_type(term_l->type))
			// ставим скобки при итерации, если символов > 1
			str1 = "(" + str1 + ")";
		break;
	case Type::negative:
		symb = '^';
		if (!is_terminal_type(term_l->type)) {
			return symb + "(" + str1 + ")";
		}
		return symb + str1;
	default:
		break;
	}

	return str1 + symb + str2;
}

// для дебага
void AlgExpression::print_subtree(AlgExpression* expr, int level) const {
	if (expr) {
		print_subtree(expr->term_l, level + 1);
		for (int i = 0; i < level; i++)
			cout << "   ";
		Symbol r_v;
		if (expr->symbol != "")
			r_v = expr->symbol;
		else
			r_v = to_string(expr->type);
		cout << r_v << endl;
		print_subtree(expr->term_r, level + 1);
	}
}

void AlgExpression::print_tree() const {
	print_subtree(term_l, 1);
	for (int i = 0; i < 0; i++)
		cout << "   ";
	Symbol r_v;
	if (symbol != "")
		r_v = symbol;
	else
		r_v = to_string(type);
	cout << r_v << endl;
	print_subtree(term_r, 1);
}

string AlgExpression::type_to_str() const {
	if (symbol != "")
		return symbol;
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
	case Type::negative:
		return "^";
	default:
		break;
	}
	return {};
}

string AlgExpression::print_subdot(AlgExpression* expr, const string& parent_dot_node,
								   int& id) const {
	string dot;
	if (expr) {
		string dot_node = "node" + to_string(id++);

		Symbol r_v;
		r_v = expr->type_to_str();

		dot += dot_node + " [label=\"" + string(r_v) + "\"];\n";

		if (!parent_dot_node.empty()) {
			dot += parent_dot_node + " -- " + dot_node + ";\n";
		}

		dot += print_subdot(expr->term_l, dot_node, id);
		dot += print_subdot(expr->term_r, dot_node, id);
	}
	return dot;
}

void AlgExpression::print_dot() const {
	int id = 0;

	string dot;
	dot += "graph {\n";

	Symbol r_v;
	r_v = type_to_str();

	string root_dot_node = "node" + to_string(id++);
	dot += root_dot_node + " [label=\"" + to_txt() + "\\n" + string(r_v) + "\"];\n";

	dot += print_subdot(term_l, root_dot_node, id);
	dot += print_subdot(term_r, root_dot_node, id);

	dot += "}\n";
	cout << dot << endl;
}

bool read_number(const string& str, size_t& pos, int& res) { // NOLINT(runtime/references)
	if (pos >= str.size() || !isdigit(str[pos])) {
		return false;
	}

	res = 0;
	while (pos < str.size() && isdigit(str[pos])) {
		res = res * 10 + (str[pos] - '0');
		pos++;
	}

	pos--;
	return true;
}

vector<AlgExpression::Lexeme> AlgExpression::parse_string(string str, bool allow_ref,
														  bool allow_negation) {
	vector<AlgExpression::Lexeme> lexemes;
	std::stack<char> brackets_checker;
	bool brackets_are_empty = true;
	std::stack<size_t> memory_opening_indexes;

	bool regex_is_eps = true;

	for (size_t index = 0; index < str.size(); index++) {
		char c = str[index];
		Lexeme lexeme;
		switch (c) {
		case '^':
			if (!allow_negation)
				return {Lexeme::Type::error};

			lexeme.type = Lexeme::Type::negative;
			break;
		case '(':
			lexeme.type = Lexeme::Type::parL;
			brackets_checker.push('(');
			brackets_are_empty = true;
			break;
		case ')':
			lexeme.type = Lexeme::Type::parR;
			if (brackets_are_empty || brackets_checker.empty() || brackets_checker.top() != '(')
				return {Lexeme::Type::error};

			brackets_checker.pop();
			break;
		case '[':
			if (!allow_ref)
				return {Lexeme::Type::error};

			lexeme.type = Lexeme::Type::squareBrL;
			brackets_checker.push('[');
			brackets_are_empty = true;
			break;
		case ']':
			if (brackets_checker.empty() || brackets_checker.top() != '[')
				return {Lexeme::Type::error};
			brackets_checker.pop();

			if (brackets_are_empty) {
				if (lexemes.back().type != Lexeme::Type::squareBrL)
					return {Lexeme::Type::error};
				lexemes.emplace_back(Lexeme::Type::eps);
				brackets_are_empty = false;
			}

			index++;
			if (index >= str.size() && str[index] != ':')
				return {Lexeme::Type::error};

			if (!read_number(str, ++index, lexeme.number))
				return {Lexeme::Type::error};

			lexeme.type = Lexeme::Type::squareBrR;
			lexemes[memory_opening_indexes.top()].number = lexeme.number;
			memory_opening_indexes.pop();
			break;
		case '&':
			if (!read_number(str, ++index, lexeme.number))
				return {Lexeme::Type::error};

			if (allow_ref)
				lexeme.type = Lexeme::Type::ref;
			else
				lexeme.type = Lexeme::Type::symb;

			lexeme.symbol = Symbol::Ref(lexeme.number);
			regex_is_eps = false;
			brackets_are_empty = false;
			break;
		case '|':
			if (index != 0 && lexemes.back().type == Lexeme::Type::negative)
				return {Lexeme::Type::error};

			lexeme.type = Lexeme::Type::alt;
			break;
		case '*':
			if (index == 0 || (index != 0 && (lexemes.back().type == Lexeme::Type::star ||
											  lexemes.back().type == Lexeme::Type::alt ||
											  lexemes.back().type == Lexeme::Type::negative)))
				return {Lexeme::Type::error};

			lexeme.type = Lexeme::Type::star;
			break;
		default:
			if (isalpha(c)) {
				lexeme.type = Lexeme::Type::symb;
				lexeme.symbol = c;
				for (size_t j = index + 1; j < str.size(); j++) {
					bool lin = false;
					bool annote = false;

					if (str[j] == Symbol::linearize_marker) {
						lin = true;
						j++;
					} else if (str[j] == Symbol::annote_marker) {
						annote = true;
						j++;
					} else if (!MemorySymbols::is_memory_char(c) || !isdigit(str[j])) {
						break;
					}

					int number;
					if (!read_number(str, j, number))
						return {Lexeme::Type::error};
					index = j;

					if (lin) {
						lexeme.symbol.linearize(number);
					} else if (annote) {
						lexeme.symbol.annote(number);
					} else { // memory
						if (c == MemorySymbols::CloseChar) {
							lexeme.symbol = MemorySymbols::Close(number);
						} else if (c == MemorySymbols::ResetChar) {
							lexeme.symbol = MemorySymbols::Reset(number);
						} else {
							lexeme.symbol = MemorySymbols::Open(number);
						}
					}
				}

				regex_is_eps = false;
				brackets_are_empty = false;
			} else {
				return {Lexeme::Type::error};
			}
			break;
		}

		if (!lexemes.empty() &&
			(
				// AlgExpression::Lexeme left
				lexemes.back().type == Lexeme::Type::symb ||
				lexemes.back().type == Lexeme::Type::star ||
				lexemes.back().type == Lexeme::Type::parR ||
				lexemes.back().type == Lexeme::Type::squareBrR ||
				lexemes.back().type == Lexeme::Type::ref) &&
			(
				// AlgExpression::Lexeme right
				lexeme.type == Lexeme::Type::symb || lexeme.type == Lexeme::Type::parL ||
				lexeme.type == Lexeme::Type::squareBrL || lexeme.type == Lexeme::Type::ref ||
				lexeme.type == Lexeme::Type::negative)) {
			// We place . between
			lexemes.emplace_back(Lexeme::Type::conc);
		}

		if (!lexemes.empty() &&
			((lexeme.type == Lexeme::Type::alt &&
			  (lexemes.back().type == Lexeme::Type::parL ||
			   lexemes.back().type == Lexeme::Type::squareBrL)) ||
			 ((lexemes.back().type == Lexeme::Type::alt ||
			   lexemes.back().type == Lexeme::Type::negative) &&
			  (lexeme.type == Lexeme::Type::parR || lexeme.type == Lexeme::Type::squareBrR ||
			   lexeme.type == Lexeme::Type::alt)))) {
			//  We place eps between
			lexemes.emplace_back(Lexeme::Type::eps);
		}

		if (lexeme.type == Lexeme::Type::squareBrL) {
			memory_opening_indexes.push(lexemes.size());
		}

		lexemes.emplace_back(lexeme);
	}

	if (regex_is_eps || !brackets_checker.empty())
		return {Lexeme::Type::error};

	// проверка на отсутствие вложенных захватов памяти для одной ячейки
	std::unordered_set<int> opened_memory_cells;
	for (const auto& l : lexemes) {
		switch (l.type) {
		case Lexeme::Type::squareBrL:
			if (opened_memory_cells.count(l.number))
				return {Lexeme::Type::error};
			opened_memory_cells.insert(l.number);
			break;
		case Lexeme::Type::squareBrR:
			opened_memory_cells.erase(l.number);
			break;
		case Lexeme::Type::ref:
			if (opened_memory_cells.count(l.number))
				return {Lexeme::Type::error};
		default:
			break;
		}
	}

	if (!lexemes.empty() && lexemes[0].type == Lexeme::Type::alt) {
		lexemes.insert(lexemes.begin(), {Lexeme::Type::eps});
	}

	if (lexemes.back().type == Lexeme::Type::alt || lexemes.back().type == Lexeme::Type::negative) {
		lexemes.emplace_back(Lexeme::Type::eps);
	}

	return lexemes;
}

bool AlgExpression::from_string(const string& str, bool allow_ref, bool allow_negation) {
	if (str.empty()) {
		symbol = Lexeme::Type::eps;
		type = Type::eps;
		language = make_shared<Language>();
		return true;
	}

	vector<Lexeme> l = parse_string(str, allow_ref, allow_negation);
	AlgExpression* root = expr(l, 0, l.size());

	if (root == nullptr || root->type == eps) {
		delete root;
		return false;
	}

	copy(root);
	language = make_shared<Language>(alphabet);

	delete root;
	return true;
}

void AlgExpression::update_balance(const AlgExpression::Lexeme& l, int& balance) {
	if (l.type == Lexeme::Type::parL || l.type == Lexeme::Type::squareBrL) {
		balance++;
	}
	if (l.type == Lexeme::Type::parR || l.type == Lexeme::Type::squareBrR) {
		balance--;
	}
}

AlgExpression* AlgExpression::scan_conc(const vector<AlgExpression::Lexeme>& lexemes,
										int index_start, int index_end) {
	AlgExpression* p = nullptr;
	int balance = 0;
	for (int i = index_start; i < index_end; i++) {
		update_balance(lexemes[i], balance);
		if (lexemes[i].type == Lexeme::Type::conc && balance == 0) {
			AlgExpression* l = expr(lexemes, index_start, i);
			AlgExpression* r = expr(lexemes, i + 1, index_end);
			if (l == nullptr || r == nullptr || r->type == Type::eps ||
				l->type == Type::eps) { // Проверка на адекватность)
				delete r;
				delete l;
				return p;
			}

			p = make();
			p->term_l = l;
			p->term_r = r;
			p->symbol = lexemes[i].symbol;
			p->type = Type::conc;

			p->alphabet = l->alphabet;
			p->alphabet.insert(r->alphabet.begin(), r->alphabet.end());
			return p;
		}
	}
	return nullptr;
}

AlgExpression* AlgExpression::scan_star(const vector<AlgExpression::Lexeme>& lexemes,
										int index_start, int index_end) {
	AlgExpression* p = nullptr;
	int balance = 0;
	for (int i = index_start; i < index_end; i++) {
		update_balance(lexemes[i], balance);
		if (lexemes[i].type == Lexeme::Type::star && balance == 0) {
			AlgExpression* l = expr(lexemes, index_start, i);
			if (l == nullptr || l->type == Type::eps) {
				delete l;
				return p;
			}

			p = make();
			p->term_l = l;
			p->type = Type::star;

			p->alphabet = l->alphabet;
			return p;
		}
	}
	return nullptr;
}

AlgExpression* AlgExpression::scan_alt(const vector<AlgExpression::Lexeme>& lexemes,
									   int index_start, int index_end) {
	AlgExpression* p = nullptr;
	int balance = 0;
	for (int i = index_start; i < index_end; i++) {
		update_balance(lexemes[i], balance);
		if (lexemes[i].type == Lexeme::Type::alt && balance == 0) {
			AlgExpression* l = expr(lexemes, index_start, i);
			AlgExpression* r = expr(lexemes, i + 1, index_end);
			if (l == nullptr || r == nullptr) { // Проверка на адекватность)
				delete r;
				delete l;
				return nullptr;
			}

			p = make();
			p->term_l = l;
			p->term_r = r;
			p->type = Type::alt;

			p->alphabet = l->alphabet;
			p->alphabet.insert(r->alphabet.begin(), r->alphabet.end());
			return p;
		}
	}
	return nullptr;
}

AlgExpression* AlgExpression::scan_symb(const vector<AlgExpression::Lexeme>& lexemes,
										int index_start, int index_end) {
	AlgExpression* p = nullptr;
	if (index_start >= lexemes.size() || (index_end - index_start > 1) ||
		lexemes[index_start].type != Lexeme::Type::symb) {
		return nullptr;
	}

	p = make();
	p->symbol = lexemes[index_start].symbol;
	p->type = Type::symb;
	p->alphabet = {lexemes[index_start].symbol};
	return p;
}

AlgExpression* AlgExpression::scan_eps(const vector<AlgExpression::Lexeme>& lexemes,
									   int index_start, int index_end) {
	AlgExpression* p = nullptr;
	if (index_start >= lexemes.size() || (index_end - index_start != 1) ||
		lexemes[index_start].type != Lexeme::Type::eps) {
		return nullptr;
	}

	p = make();
	p->symbol = Symbol::Epsilon;
	p->type = Type::eps;
	return p;
}

AlgExpression* AlgExpression::scan_par(const vector<AlgExpression::Lexeme>& lexemes,
									   int index_start, int index_end) {
	AlgExpression* p = nullptr;
	if ((index_end - 1) >= lexemes.size() || (lexemes[index_start].type != Lexeme::Type::parL ||
											  lexemes[index_end - 1].type != Lexeme::Type::parR)) {
		return nullptr;
	}

	p = expr(lexemes, index_start + 1, index_end - 1);
	return p;
}

// bool AlgExpression::equality_checker(const AlgExpression* expr1, const AlgExpression* expr2) {
//	if (expr1 == nullptr && expr2 == nullptr)
//		return true;
//	if (expr1 == nullptr || expr2 == nullptr)
//		return false;
//	if (expr1->type != expr2->type || expr1->symbol != expr2->symbol || !expr1->equals(expr2))
//		return false;
//
//	if (equality_checker(expr1->term_l, expr2->term_l) &&
//		equality_checker(expr1->term_r, expr2->term_r))
//		return true;
//	if (expr1->type != Type::conc && equality_checker(expr1->term_r, expr2->term_l) &&
//		equality_checker(expr1->term_l, expr2->term_r))
//		return true;
//	return false;
// }

bool AlgExpression::equality_checker(const AlgExpression* expr1, const AlgExpression* expr2) {
	AlgExpression *temp1 = expr1->make_copy(), *temp2 = expr2->make_copy();
	vector<AlgExpression*> alts;
	temp1->_rewrite_aci(alts, false, false);
	alts.clear();
	temp2->_rewrite_aci(alts, false, false);
	return temp1->to_txt() == temp2->to_txt();
}

// для метода test
string AlgExpression::get_iterated_word(int n) const {
	string str;
	/*if (type == Type::alt) {
		cout << "ERROR: regex with '|' is passed to the method Test\n";
		return "";
	}*/
	if (term_l) {
		if (type == Type::star) {
			for (int i = 0; i < n; i++)
				str += term_l->get_iterated_word(n);
		} else {
			str += term_l->get_iterated_word(n);
		}
	}
	if (term_r && type != Type::alt) {
		str += term_r->get_iterated_word(n);
	}
	if (symbol != "") {
		str += symbol;
	}
	return str;
}

vector<AlgExpression*> AlgExpression::get_first_nodes() {
	vector<AlgExpression*> l, r;
	switch (type) {
	case Type::alt:
		l = term_l->get_first_nodes();
		r = term_r->get_first_nodes();
		l.insert(l.end(), r.begin(), r.end());
		return l;
	case Type::conc:
		l = term_l->get_first_nodes();
		if (term_l->contains_eps()) {
			r = term_r->get_first_nodes();
			l.insert(l.end(), r.begin(), r.end());
		}
		return l;
	case Type::star:
	case Type::memoryWriter:
		return term_l->get_first_nodes();
	case Type::eps:
		return {};
	default:
		return {this};
	}
}

vector<AlgExpression*> AlgExpression::get_last_nodes() {
	vector<AlgExpression*> l, r;
	switch (type) {
	case Type::alt:
		l = term_l->get_last_nodes();
		r = term_r->get_last_nodes();
		l.insert(l.end(), r.begin(), r.end());
		return l;
	case Type::conc:
		r = term_r->get_last_nodes();
		if (term_r->contains_eps()) {
			l = term_l->get_last_nodes();
			r.insert(r.end(), l.begin(), l.end());
		}
		return r;
	case Type::star:
	case Type::memoryWriter:
		return term_l->get_last_nodes();
	case Type::eps:
		return {};
	default:
		return {this};
	}
}

void AlgExpression::sort_alts(std::vector<AlgExpression*>& alts, bool erase_alts) {
	// rule w1 | w2 = w2 | w1
	// rule (w1 | w2) | w3 = w1 | (w2 | w3)
	std::sort(alts.begin(), alts.end(), [](AlgExpression*& a, AlgExpression*& b) {
		return a->to_txt() < b->to_txt();
	});
	if (erase_alts) {
		// rule w | w = w
		alts.erase(std::unique(alts.begin(),
							   alts.end(),
							   [](AlgExpression*& a, AlgExpression*& b) {
								   return a->to_txt() == b->to_txt();
							   }),
				   alts.end());
		bool found_eps = false;
		for (auto i : alts)
			if (i->type != Type::eps && i->contains_eps())
				found_eps = true;
		if (found_eps)
			alts.erase(std::remove_if(alts.begin(),
									  alts.end(),
									  [](AlgExpression*& a) { return a->type == eps; }),
					   alts.end());
	}
}

vector<AlgExpression*> AlgExpression::join_alts(std::vector<AlgExpression*> alts_to_join,
												AlgExpression* root) const {
	vector<AlgExpression*> res;
	if (!root)
		return res;
	if (alts_to_join.size() == 1) {
		root->copy(alts_to_join[0]);
		res.emplace_back(root);
		return res;
	}

	root->type = Type::alt;
	root->term_l = alts_to_join[0]->make_copy();
	res.emplace_back(root->term_l);
	for (int i = 1; i < alts_to_join.size() - 1; i++) {
		root->term_r = make();
		root = root->term_r;
		root->type = Type::alt;
		root->term_l = alts_to_join[i]->make_copy();
		res.emplace_back(root->term_l);
	}
	root->term_r = alts_to_join[alts_to_join.size() - 1]->make_copy();
	res.emplace_back(root->term_r);
	return res;
}

void AlgExpression::_rewrite_aci(std::vector<AlgExpression*>& alts, bool from_alt,
								 bool erase_alts) {
	auto t = to_txt();
	vector<AlgExpression*> cur_alts;
	switch (type) {
	case Type::alt: {
		term_l->_rewrite_aci(cur_alts, true, erase_alts);
		term_r->_rewrite_aci(cur_alts, true, erase_alts);
		if (!from_alt) {
			sort_alts(cur_alts, erase_alts);
			vector<AlgExpression*> aci_alts(cur_alts.size());
			for (int i = 0; i < aci_alts.size(); i++)
				aci_alts[i] = cur_alts[i]->make_copy();
			clear(); // очистятся в том числе cur_alts
			auto root_l =
				language; // чтобы сохранить указатель в корне на случай удаления (пример a|a)
			vector<AlgExpression*> res_alts = join_alts(aci_alts, this);
			language = root_l;
			for (auto& aci_alt : aci_alts)
				delete aci_alt;
			alts.insert(alts.end(), res_alts.begin(), res_alts.end());
		} else {
			alts.insert(alts.end(), cur_alts.begin(), cur_alts.end());
		}
		break;
	}
	case Type::conc:
		alts.emplace_back(this);
		term_l->_rewrite_aci(cur_alts, false, erase_alts);
		term_r->_rewrite_aci(cur_alts, false, erase_alts);
		break;
	case Type::star:
	case Type::negative:
	case Type::memoryWriter:
		alts.emplace_back(this);
		term_l->_rewrite_aci(cur_alts, false, erase_alts);
		break;
	default:
		alts.emplace_back(this);
		break;
	}
}
