#pragma once
#include "AlgExpression.h"
#include "AlphabetSymbol.h"
#include "BaseObject.h"
#include "iLogTemplate.h"
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

class Regex : public AlgExpression {
  private:
	// возвращает указатель на new Regex
	AlgExpression* make() const override;

	// Множество префиксов длины len
	void get_prefix(int len, std::set<std::string>& prefs) const;
	// Производная по символу
	bool derivative_with_respect_to_sym(Regex* respected_sym, const Regex* reg_e,
										Regex& result) const;
	bool partial_derivative_with_respect_to_sym(Regex* respected_sym, const Regex* reg_e,
												vector<Regex>& result) const;
	// Производная по префиксу
	bool derivative_with_respect_to_str(std::string str, const Regex* reg_e, Regex& result) const;
	pair<vector<State>, int> get_thompson(int) const;

	void normalize_this_regex(const vector<pair<Regex, Regex>>&); // переписывание regex по
																  // пользовательским правилам

  public:
	Regex() = default;
	Regex(const string&);
	Regex(const string&, const shared_ptr<Language>&);

	// dynamic_cast к типу Regex*
	template <typename T> static Regex* cast(T* ptr);
	// dynamic_cast каждого элемента вектора к типу Regex*
	template <typename T> static vector<Regex*> cast(vector<T*> ptr);

	FiniteAutomaton to_thompson(iLogTemplate* log = nullptr) const;
	FiniteAutomaton to_glushkov(iLogTemplate* log = nullptr) const;
	FiniteAutomaton to_ilieyu(iLogTemplate* log = nullptr) const;
	FiniteAutomaton to_antimirov(iLogTemplate* log = nullptr) const;

	// проверка регулярок на равенство(буквальное)
	static bool equal(const Regex&, const Regex&, iLogTemplate* log = nullptr);
	// проверка регулярок на эквивалентность
	static bool equivalent(const Regex&, const Regex&, iLogTemplate* log = nullptr);
	// проверка регулярок на вложенность (проверяет вложен ли аргумент в this)
	bool subset(const Regex&, iLogTemplate* log = nullptr) const;

	// Производная по символу
	optional<Regex> symbol_derivative(const Regex& respected_sym) const;
	// Частичная производная по символу
	void partial_symbol_derivative(const Regex& respected_sym, vector<Regex>& result) const;
	// Производная по префиксу
	optional<Regex> prefix_derivative(string respected_str) const;
	// Длина накачки
	int pump_length(iLogTemplate* log = nullptr) const;

	Regex linearize(iLogTemplate* log = nullptr) const;
	Regex delinearize(iLogTemplate* log = nullptr) const;
	Regex deannote(iLogTemplate* log = nullptr) const;

	// проверка регулярки на 1-однозначность
	bool is_one_unambiguous(iLogTemplate* log = nullptr) const;
	// извлечение 1-однозначной регулярки методом орбит Брюггеман-Вуда
	Regex get_one_unambiguous_regex(iLogTemplate* log = nullptr) const;

	// Переписывание regex по пользовательским правилам
	Regex normalize_regex(const vector<pair<Regex, Regex>>&, iLogTemplate* log = nullptr) const;
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

Почистите вашу мышь. Отсоедините ее поводок от компьютера, вытащите
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