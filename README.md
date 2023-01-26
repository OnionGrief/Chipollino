# Оглавление
* [О проекте](#about)
* [Сборка и запуск](#build)
* [Генерация случайных тестов](#generator)
* [Синтаксис входных данных](#syntax)
* [Функции преобразователя](#functions)

*Если останутся вопросы по функциональности, задайте их [здесь](https://github.com/StarikTenger/Chipollino/issues/203)*

# <a id="about" /> О проекте
Принцип работы — на вход даётся файл с операциями над объектами, на выходе генерируется *latex*-файл с подробным описанием выполненных преобразований.

**ВАЖНО!** Для корректного составления *latex*-файла нужно поставить [Graphviz](https://graphviz.org/download/), чтобы в системе поддерживалась команда `dot`. Без этого не будет работать генерация изображений автоматов.

Если в системе поддерживается команда `pdflatex`, то тогда сразу будет генерироваться отчет в виде *pdf*-документа. Если нет, необходимо установить [Latex](https://www.latex-project.org/get/).

# <a id="build"/> Сборка и запуск

**Сборка** осуществляется через *cmake*. В [wiki](https://github.com/StarikTenger/Chipollino/wiki/%F0%9F%9B%A0%EF%B8%8F-%D0%A1%D0%B1%D0%BE%D1%80%D0%BA%D0%B0) есть пример, как можно собрать.

Для **запуска** надо перейти в корневую папку проекта и там запустить собранное приложение. Пример на windows:
```
.\build\apps\InterpreterApp\Debug\InterpreterApp
```
В качестве опционального аргумента указывается путь к файлу с командами (по умолчанию test.txt).

# <a id="generator"/> Генерация случайных тестов

Для удобства тестирования был создан генератор входных данных.  
Его запуск на windows:
```
.\build\apps\InputGeneratorApp\Debug\InputGeneratorApp
```

В файле *apps\InputGeneratorApp\src\main.cpp* можно поменять параметры генерируемых тестов:  
TG.generate_task(3, 5, false, false);  
где 1 аргумент - количество операций, 2 аргумент - максимальное количество функций в последовательности.

# <a id="syntax"/> Синтаксис входных данных

### Типизация
Интерпретатор поддерживает следующие типы:
* Regex — регулярное выражение (с операциями '|' и '*')
* NFA — недетерминированный конечный автомат
* DFA — детерминированный конечный автомат
* PrefixGrammar - префиксная грамматика
* Int
* Boolean (где-то обозначен как t/f)
* AmbiguityValue - мера неоднозначности  
(Может принимать одно из 4-х значений: *Unambigious, Almost unambigious, Polynomially ambiguous, Exponentially ambiguous*)

### Синтаксические конструкции
* **Regex**. Записываются в фигурных скобках. Пример: `{a*(b|c)*}`
* **Function**. Название и список аргументов через пробел. Пример: `Glushkov {ab|a}`
* **Function sequence**. Последовательное применение функций справа-налево. Пример: `Annote.Glushkov.DeAnnote {a}`
* **Int**. Целое число. Пример: `8`
* **Varaible** - переменная. Примеры: `N1`, `N2`, `N3`
* **Expression**. Function sequence, Int, Regex или Varaible

Для функций и последовательностей функций должны выполняться соответствия типов. Больше про типы фунцкий - ниже в разделе [Функции преобразователя](#functions).

### Команды
В каждой строчке записана ровно одна команда. Поддерживаются команды пяти типов:
* **declaration**  
Присвоение переменной значения. Если в конце стоит `!!`, выражение логируется в *latex*.  
***Синтаксис:***  
`<varaible> = <expression> '!!'?`  
***Пример:***

        A = Complement.Annote (Glushkov {a*}) !!
        B = States.Reverse A

* **test**  
***Аргументы:***  
1\. НКА или регулярное выражение (язык 1)  
2\. регулярное выражение — тестовый сет (язык 2)  
3\. натуральное число — шаг итерации в сете  
Подробнее про то, как работает — см. в [презентации](https://github.com/TonitaN/FormalLanguageTheory/blob/main/2022/tasks/lab_tfl_2022_2.pdf).  
***Синтаксис:***  
`Test <expression> <expression> <int>`  
***Пример:***

        Test {(aa)*} {a*} 3
        Test (Glushkov {(aa)*}) {a*} 1
        
* **predicate**  
Булева функция, записанная одной строчкой. Будет логироваться в любом случае.  
***Синтаксис:***  
`<function> <expression>+`  
***Пример:***  
`Equiv (Antimirov {ab|a}) (Glushkov {ab|a})`

* **verify**  
***Аргументы:***  
1\. Предикат (с конструкцией `*`, на место которой подставляются сгенерированные регулярные выражения)  
2\. *(опциональный)* натуральное число — количество тестов  
***Синтаксис:***  
`Verify <expression> <int>?`  
***Пример:***

        Verify (Equal (Ambiguity.Glushkov.Arden.Glushkov *) (Ambiguity.Glushkov *))

* **set**  
***Аргументы:***  
1\. Имя флага:  
\- `weak_type_comparison` — устанавливает эквивалентность типов `DFA` и `NFA`, т.е. допускает на вход `NFA` для функций, требующих `DFА`  
**TODO:**  
\- `log_theory` — добавляет теоретический блок к функциям в отчете  
\- `auto_remove_trap_states` — отвечает за удаление ловушек  
2\. Значение флага: `true` / `false`  
***Синтаксис:***  
`Set <flagName> <true/false>`  


# <a id="functions"/> Функции преобразователя

**\*** `NFA` здесь можно понимать как `FA`, для которого не обязан быть включённым флаг детерминизма.


**Преобразования со сменой класса:**
- `Thompson: Regex -> NFA` — строит автомат Томпсона по регулярному выражению
- `Antimirov: Regex -> NFA`
- `Glushkov: Regex -> NFA`
- `IlieYu: Regex -> NFA` — follow-автомат
- `PrefixGrammar: NFA -> PG` — возвращает префиксную грамматику для НКА
- `PGtoNFA: PG -> NFA` — преобразует префиксную грамматику в НКА  
**TODO:**  
- `Arden: NFA -> Regex` — строит регуялрное выражение по автомату

**Преобразования внутри класса**

*Над регулярными выражениями:*
- `Linearize: Regex -> Regex` — размечает буквы в регулярном выражении, как в алгоритме Глушкова
- `DeLinearize: Regex -> Regex` — снимает разметку Linearize
- `DeAnnote: Regex -> Regex` — снимает разметку Annote  
**TODO:**  
- `Disambiguate :: Regex -> Regex` — для 1-однозначных языков возвращает 1-однозначное регулярное выражение, для остальных возвращает данное на вход

*Над конечными автоматами:*
- `Determinize+: NFA -> DFA` — детерминизация с добавлением состояния-ловушки
- `Minimize+: NFA -> DFA` — минимизация с добавлением состояния-ловушки
- `RemoveTrap: DFA -> DFA` - удаление состояний-ловушек
- `Reverse: NFA -> NFA` — обращение ("переворачивает" автомат)
- `Complement: DFA -> DFA` — дополнение
- `RemEps: NFA -> NFA` — удаление ε-правил
- `MergeBisim: NFA -> NFA` — объединение эквивалентных по бисимуляции состояний
- `Annote: NFA -> DFA` — навешивает разметку на все буквы в автомате, стоящие на недетерминированных переходах
- `DeAnnote: NFA -> NFA` — снимает разметку Annote
- `DeLinearize: NFA -> NFA` — снимает разметку Linearize
- `Intersect: (NFA, NFA) -> NFA` — пересечение языков
- `Union: (NFA, NFA) -> NFA` — объединение языков
- `Difference: (NFA, NFA) -> NFA` — разность языков  
**TODO:**  
- `Determinize: NFA -> DFA` — детерминизация без добавления состояния-ловушки
- `Minimize: NFA -> DFA` — минимизация без добавления состояния-ловушки

**Многосортные функции**
- `States: NFA -> Int` — возвращает количество состояний в автомате
- `ClassCard: DFA -> Int` — число классов эквивалентности в трасформационном моноиде
- `ClassLength: DFA -> Int` — самое длинное слово в классе эквивалентности трасформационного моноида
- `MyhillNerode: DFA -> Int` — возвращает число классов эквивалентности по Майхиллу–Нероуду 
- `GlaisterShallit :: NFA -> Int` — определяет нижнюю границу количества состояний в НКА для языка
- `Ambiguity: NFA -> AmbiguityValue` — определяет меру неоднозначности  автомата. Если число путей, по которым распознается слово (длины от минимальной до $s^2$) растёт быстрее, чем полином степени |s|, где s — число состояний НКА, то автомат экспоненциально неоднозначен. Если число путей растёт медленнее, чем линейная функция, то автомат почти однозначен (либо однозначен, если путь всегда один). Иначе автомат полиномиально неоднозначен.  
**TODO**:  
- `PumpLength: Regex -> Int` — возвращает длину накачки языка
- `Normalize: (Regex, FileName) -> Regex`

**Предикаты**

*Для регулярных выражений:*
- `Subset: (Regex, Regex) -> t/f` — проверяет вложенность второго языка в первый 
- `Equiv: (Regex, Regex) -> t/f` — проверяет, описывают ли регулярные выражения один язык
- `OneUnambiguity: Regex -> t/f` — проверяет, является ли регулярное выражение 1-однозначным. Регулярное выражение является 1-однозначным, если возможно однозначно определить, какая позиция символа в регулярном выражении должна соответствовать символу во входном слове, не заглядывая за пределы этого символа во входном слове.
- `Equal: (Regex, Regex) -> t/f` — проверяет буквальное равенство регулярных выражений (как строк, с учетом альтернатив)

*Для конечных автоматов:* 
- `Bisimilar: (NFA, NFA) -> t/f` — проверяет бисимилярность автоматов
- `Equiv: (NFA, NFA) -> t/f` — проверяет, описывают ли автоматы один язык
- `Equal: (NFA, NFA) -> t/f` — проверяет буквальное равенство автоматов
- `Subset: (NFA, NFA) -> t/f` — проверяет вложенность второго языка в первый
- `Minimal: DFA -> t/f` — проверяет, минимален ли детерминированный автомат
- `Minimal : NFA -> t/f/u` — проверяет, минимален ли недетерминированный автомат
- `Deterministic : NFA -> t/f` — проверяет автомат на детерминизированность
- `OneUnambiguity : NFA -> t/f` — проверяет, является ли язык 1-однозначным. 1-однозначный язык - это язык, описываемый 1-однозначным регулярным выражением.  
**TODO**:  
- `SemDet: NFA -> t/f` — проверяет, является ли автомат семантически детерминированным. Язык состояния q — это \{w \| q→<sup>w</sup>q<sub>f</sub>} Семантическая детерминированность означает, что для всякой неоднозначности q<sub>i</sub>→<sup>a</sup>q<sub>j<sub>1</sub></sub>, ...,  q<sub>i</sub>→<sup>a</sup>q<sub>j<sub>k</sub></sub> существует такое состояние q<sub>j<sub>s</sub></sub>, что языки всех  q<sub>j<sub>t</sub></sub> (1 ⩽ t ⩽ k) вкладываются в его язык.

*Многосортные:*
- `Equal: (Int, Int) -> t/f`
- `Equal: (AmbiguityValue, AmbiguityValue) -> t/f`

**Метод Test**  

`Test: (Regex|NFA, Regex, Int) -> таблица` — порождает слова, принадлежащие второму языку (2 аргумент), раскрывая каждую итерацию Клини начиная с 0 (пустого слова) и увеличивая с каждым разом на заданное значение (3 аргумент). Если в регулярном выражении (для 2 языка) присутствуют альтернативы, то выбирается 1ая из них.  
То есть  
`a*b` c шагом 2 раскрывается в ряд: b, aab, aaaab ...  
`(a*b)*c` c шагом 1 раскрывается в ряд: c, abc, aabaabc ...  
Возвращает таблицу, в которой указаны результаты проверки на принадлежность каждого из порожденных слов первому языку (1 аргумент).  
Завершает тестирование, когда сделано более 12 шагов, либо парсинг входной строки занимает больше 3 минут.  

**Верификатор гипотез**

`Verify: (Boolean, Int?) -> Int` - принимает на вход набор действий, содержащий единственный предикат. Строится сет тестов, размер которого определяет 2й аргумент (значение по умолчанию - 20). В выражение на место конструкции `*` подставляются случайные регулярные выражения.  
В результатах тестирования выделяется доля тестов с положительным значением предиката, и примеры кейсов, когда гипотеза не выполнилась.


Дополнительно о некоторых функциях и требованиях к входным данным можно почитать в [файле](https://github.com/TonitaN/FormalLanguageTheory/blob/main/2022/tasks/lab_tfl_2022_2.pdf).

Успехов!
