#include "AutomatonToImage/AutomatonToImage.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/Regex.h"
#include "Objects/iLogTemplate.h"
#include <map>
#include <string>
#include <variant>

using namespace std;

class LogTemplate : public iLogTemplate {
  public:
	void set_parameter(const string& key, FiniteAutomaton value) override;
	void set_parameter(const string& key, Regex value) override;
	void set_parameter(const string& key, string value) override;
	void set_parameter(const string& key, int value) override;

	// Рендерит все логи, возвращает строку
	string render() const;
	// загрузка шаблона
	void load_tex_template(string filename);

  private:
	// LaTeX-шаблон
	string tex_template;

	// Стуктура для хранения параметров
	struct LogParameter {
		variant<FiniteAutomaton, Regex, string, int> value;
	};

	// Параметры
	map<string, LogParameter> parameters;

	//Добавление шаблона настоящего параметра
	void add_parameter(string parameter_name);
	// math mode
	static string math_mode(string str);
	// счетчик картинок
	inline static int image_number = 0;
};