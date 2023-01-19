#include "AutomatonToImage/AutomatonToImage.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/Regex.h"
#include "Objects/iLogTemplate.h"
#include <map>
#include <sstream>
#include <string>
#include <variant>

using namespace std;

class LogTemplate : public iLogTemplate {
  public:
	void set_parameter(const string& key, FiniteAutomaton value) override;
	void set_parameter(const string& key, Regex value) override;
	void set_parameter(const string& key, string value) override;
	void set_parameter(const string& key, int value) override;
	void set_parameter(const string& key, Table value) override;
	void set_theory_flag(bool value);

	// Рендерит все логи, возвращает строку
	string render() const;
	// загрузка шаблона
	void load_tex_template(string filename);

	// struct Table {
	// 	vector<string> rows;	// названия строк
	// 	vector<string> columns; // названия столбцов
	// 	vector<string> data;	// данные
	// };

  private:
	// Путь к папке с шаблонами
	const string template_path = "./resources/template/";

	// LaTeX-шаблон
	string tex_template;

	// флаг логирования подробной части
	bool render_theory = false;

	// Стуктура для хранения параметров
	struct LogParameter {
		variant<FiniteAutomaton, Regex, string, int, Table> value;
	};

	// Параметры
	map<string, LogParameter> parameters;
	// math mode
	static string math_mode(string str);
	// счетчик картинок
	inline static int image_number = 0;
	// таблицы в общем виде
	static string log_table(Table t/*vector<string> rows, vector<string> columns,
							vector<string> data*/);
	// Рекурсивно раскрывает include-выражения в файле
	stringstream expand_includes(string filename) const;
};