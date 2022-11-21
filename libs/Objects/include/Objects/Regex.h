#pragma once
#include "AlphabetSymbol.h"
#include "BaseObject.h"
#include "Logger.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>
using namespace std;

class Language;
class FiniteAutomaton;
struct State;

struct Lexem {
	enum Type {
		error,
		parL, // (
		parR, // )
		alt,  // |
		conc, // .
		star, // *
		symb, // alphabet symbol
		eps,  // Epsilon
	};

	Type type = error;
	alphabet_symbol symbol = "";
	int number = 0;
	Lexem(Type type = error, alphabet_symbol symbol = "", int number = 0);
};

class Regex : BaseObject {
  private:
	enum Type {
		// Epsilon
		eps,
		// Binary:
		alt,
		conc,
		// Unary:
		star,
		// Terminal:
		symb
	};
	set<alphabet_symbol> alphabet;
	Type type;
	Lexem value;
	Regex* term_p = nullptr;
	Regex* term_l = nullptr;
	Regex* term_r = nullptr;
	// Turns string into lexem vector
	vector<Lexem> parse_string(string);
	Regex* expr(const vector<Lexem>&, int, int);
	Regex* scan_conc(const vector<Lexem>&, int, int);
	Regex* scan_star(const vector<Lexem>&, int, int);
	Regex* scan_alt(const vector<Lexem>&, int, int);
	Regex* scan_symb(const vector<Lexem>&, int, int);
	Regex* scan_eps(const vector<Lexem>&, int, int);
	Regex* scan_par(const vector<Lexem>&, int, int);

	// Принадлежит ли эпсилон языку регулярки
	bool is_eps_possible();
	// Множество префиксов длины len
	void get_prefix(int len, std::set<std::string>* prefs) const;
	// Производная по символу
	bool derevative_with_respect_to_sym(Regex* respected_sym,
										const Regex* reg_e,
										Regex& result) const;
	bool partial_derevative_with_respect_to_sym(Regex* respected_sym,
												const Regex* reg_e,
												vector<Regex>& result) const;
	// Производная по префиксу
	bool derevative_with_respect_to_str(std::string str, const Regex* reg_e,
										Regex& result) const;
	pair<vector<State>, int> get_tompson(int) const;

	vector<Lexem>* first_state() const; // начальные состояния для to_glushkov
	bool contains_eps() const; // проверяет, входит ли eps в дерево regex
	vector<Lexem>* end_state() const; // конечные состояния для to_glushkov
	map<int, vector<int>> pairs() const;
	vector<Regex*> pre_order_travers_vect(); // список листьев дерева regex
	bool is_term(int, const vector<Lexem>&)
		const; // возвращает true, если состояние конечно
	static bool equality_checker(const Regex*, const Regex*);
	int search_replace_rec(
		const Regex& replacing, const Regex& replaced_by,
		Regex* original); //рекурсивный поиск заменяемого листа дерева
	void normalize_this_regex(
		const string& file); //переписывание regex по пользовательским правилам
	string to_str_log() const;

	// Рекурсивная генерация алфавита
	void generate_alphabet(set<alphabet_symbol>& _alphabet);
	// для print_tree
	void print_subtree(Regex* r, int level);

	void pre_order_travers() const;
	void clear();

  public:
	Regex();
	Regex(string);
	string to_txt() const override;
	// вывод дерева для дебага
	void print_tree();

	FiniteAutomaton to_tompson() const;
	FiniteAutomaton to_glushkov() const;
	FiniteAutomaton to_ilieyu() const;
	FiniteAutomaton to_antimirov() const;

	~Regex();
	Regex* copy() const;
	Regex(const Regex&);
	Regex& operator=(const Regex& other);

	// Генерация языка из алфавита
	void make_language();
	// Переписывание regex по пользовательским правилам
	Regex normalize_regex(const string& file) const;
	bool from_string(string);
	// проверка регулярок на равентсво(буквальное)
	static bool equal(const Regex&, const Regex&);
	// проверка регулярок на эквивалентность
	static bool equivalent(const Regex&, const Regex&);
	// проверка регулярок на вложенность (проверяет вложен ли аргумент в this)
	bool subset(const Regex&) const; // TODO

	// Производная по символу
	std::optional<Regex> symbol_derevative(const Regex& respected_sym) const;
	// Частичная производная по символу
	void partial_symbol_derevative(const Regex& respected_sym,
								   vector<Regex>& result) const;
	// Производная по префиксу
	std::optional<Regex> prefix_derevative(std::string respected_str) const;
	// Длина накачки
	int pump_length() const;
	// Слово, в котором все итерации Клини раскрыты n раз
	string get_iterated_word(int n) const;

	void regex_union(Regex* a, Regex* b);
	void regex_alt(Regex* a, Regex* b);
	void regex_star(Regex* a);
	void regex_eps();

	Regex linearize() const;
	Regex delinearize() const;
	Regex deannote() const;
};

/*
	 Гуртовщики Мыши

		~~~~~~~~~~~~~~~

Microsoft компания получает много откликов после появления Окон 95. Мы
выявили, что много пользователей встретили проблему мыши. В этом
документе Служба Техничного Упора Microsoft компании сводит вместе всю
полезную информацию о возможных проблемах с мышами и гуртовщиками мыши и
забота-стреляние.

Если вы только что закрепили себе Окна 95, вы можете увидеть, что ваша
мышь плохо себя ведет. Курсор может не двигаться или движение мыши может
проявлять странные следы на поверхности стола, окнах и обоях. Мышь может
неадекватно реагировать на щелчок по почкам. Но не спешите! Это могут
быть физические проблемы, а не клоп Окон 95.

Почистите вашу мышь. Отсоедините ее поводок от компьютера , вытащите
гениталий и промойте его и ролики внутренностей спиртом. Снова зашейте
мышь. Проверьте на переломы поводка. Подсоедините мышь к компьютеру.
Приглядитесь к вашей прокладке (подушке) - она не должна быть источником
мусора и пыли в гениталии и роликах. Поверхность прокладки не должна
стеснять движения мыши.

Может быть вам стоит купить новую мышь. Мы настоятельно рекомендуем
Microsoft мышь. Она эргономично спроектирована, особо сделана под Окна
95 и имеет третью почку в виде колеса, которые могут завивать окна.
Совокупление Microsoft мыши и Окон 95 делает вашу повседневную работу
легко приятной.

Испытайте все это. Если проблемы остались - ваш гуртовщик мыши плохо
стоит под Окнами 95. Его придется убрать.

Вам нужен новый гуртовщик мыши. Если вы пользователь Microsoft мыши
посетите Microsoft Слугу Паутины, где в особом подвале вы сможете
опустить-загрузить самого текущего гуртовщика Microsoft мыши. Если
производитель вашей мыши другой, узнайте о ее гуртовщике. Все основные
производители мыши уже имеют гуртовщиков мыши для Окон 95.

Перед тем как вы будете закреплять гуртовщика мыши, сделайте
заднюю-верхнюю копию ваших досье. Почистить ваш винчестер имеет смысл. У
вас должен быть старт-вверх диск от Окон 95.

После того, как вы закрепили нового гуртовщика, скорее всего ваши
проблемы решены. Если они остались, напишите в Службу Техничного Упора
Microsoft, и вашим случаем займется Особый Отдел.

Для эффективной помощи техничного упора, наш инженер должен знать
торговую марку вашей мыши, тип (в-портовая мышь, периодическая мышь,
автобусная мышь, Полицейский Участок /2 мышь, без поводка мышь,
гениталий на гусеничном ходу и т.п.), версию гуртовщика, производителя
компьютера (матери-доски), положение портов и рубильников на
матери-доске (и расклад карт), а также содержимое досье Авто-#####.bat,
config.sys и Сапог-полено.txt.

Кроме того, несколько полезных советов

1) не закрепляйте себе Окна 95 в то же самое место, где у вас закреплены
Окна 3.икс, вы не сможете хорошо делать кое-что привычное.

2) если вы новичок под Окнами 95, привыкните к новым возможностям мыши.
Щелкните по левой почке - выделите пункт, щелкните по правой кнопке меню
с контекстом всплывет, быстро ударьте два раза по левой почке -
запустите повестку в суд.

4) отработайте быстрый двойной удар по почкам мыши с помощью
специального тренажера на пульте управления Окнами 95

6) специалисты Microsoft компании после большого числа опытов выявили,
что наиболее эффективной командой из-под Окон 95 является "Послать на
...", которая доступна в любом времени и месте при ударе по правой почке
мыши. Если вы только что закрепили себе окна 95, вы сумеете послать
только на А (Б) и в специальное место "Мой портфель". Но по мере того
как вы будете закреплять себе новые программы для Окон 95, вы начнете
посылать на все более сложные и интересные места и объекты.

Особую эффективность команда "Послать на ..." приобретет при передачи
посланий через Е-почту и общение с вашими коллегами и друзьями в местной
сети-работе. Попробуйте мощь команды "Послать на ...", и вы быстро
убедитесь, что без нее трудно существовать под Окнами 95.

Пишите нам и помните, что Microsoft компания всегда думает о том, как
вас лучше сделать.

*/