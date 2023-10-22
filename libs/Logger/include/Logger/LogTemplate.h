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
	void set_parameter(const string& key, const FiniteAutomaton& value) override;
	void set_parameter(const string& key, Regex value) override;
	void set_parameter(const string& key, string value) override;
	void set_parameter(const string& key, int value) override;
	void set_parameter(const string& key, Table value) override;
	void set_theory_flag(bool value);

	// Рендерит все логи, возвращает строку
	string render() const;
	// загрузка шаблона
	void load_tex_template(string filename);
	string get_tex_template();

  private:
	// кеш отрендеренных автоматов
	// inline static map<const FiniteAutomaton*, string> cache_automatons;
	inline static map<size_t, string> cache_automatons;
	//  Путь к папке с шаблонами
	const string template_path = "./resources/template/";

	// путь к LaTeX-шаблону
	string template_fullpath;
	// имя файла шаблона
	string template_filename;

	// флаг логирования подробной части
	bool render_theory = false;

	// Стуктура для хранения параметров
	struct LogParameter {
		variant<FiniteAutomaton, Regex, string, int, Table> value;
	};

	// Параметры
	map<string, LogParameter> parameters;

	// Добавление шаблона настоящего параметра
	void add_parameter(string parameter_name);
	// Преобразование рег. выр-я в tex-формат (устаревший метод)
	static string math_mode(string str);
	// Преобразование таблицы в tex-формат
	static string log_table(Table t);
	// Рекурсивно раскрывает include-выражения в файле
	stringstream expand_includes(string filename) const;
};