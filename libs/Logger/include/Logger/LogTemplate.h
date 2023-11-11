#ifndef LIBS_LOGGER_INCLUDE_LOGGER_LOGTEMPLATE_H_
#define LIBS_LOGGER_INCLUDE_LOGGER_LOGTEMPLATE_H_

#include <map>
#include <sstream>
#include <string>
#include <variant>

#include "AutomatonToImage/AutomatonToImage.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/Regex.h"
#include "Objects/iLogTemplate.h"

// Возможные заранее заданные стили текста
enum Decoration {
	italic,
	regexstyle,
	typewriter,
	roman
};

// Возможные заранее заданные размеры текста
enum TextSize {
	footnote,
	small,
	normal,
	large,
	none
};

class LogTemplate : public iLogTemplate {
  public:
	void set_parameter(const string& key, const LogObject& value, const MetaInfo& meta = {});
	void set_theory_flag(bool value);

	// Рендерит все логи, возвращает строку
	string render() const;
	// загрузка шаблона
	void load_tex_template(string filename);
	string get_tex_template();

	struct TextStyle {
		string tag;
		bool is_math;
	};

	// Определяет мапу идентификатора декорации и пары <тег в латехе, нужен ли мат.режим>
	inline static const unordered_map<Decoration, TextStyle> decor_data = {{italic, {"\\textit", false}},
															{regexstyle, {"\\regexpstr", true}},
															{typewriter, {"\\ttfamily", false}},
															{roman, {"\\mathrm", true}}};

	// Определяет мапу идентификатора размера и его тега в латехе
	inline static const unordered_map<TextSize, string> textsize_to_str = {{footnote, "\\footnotesize"},
												   {small, "\\small"},
												   {normal, "\\normalsize"},
												   {large, "\\large"},
												   {none, ""}};

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
		LogObject value;
		MetaInfo meta; /* Дополнительные данные о структуре, которые не кэшируются
						(раскраска) */
	};

	// Параметры
	unordered_map<string, LogParameter> parameters;


	// Добавление шаблона настоящего параметра
	void add_parameter(string parameter_name);
	// math mode (устаревший метод)
	static string math_mode(string str);
	// счетчик картинок
	inline static int image_number = 0;
	// таблицы в общем виде
	static string log_table(Table t/*vector<string> rows, vector<string> columns,
							vector<string> data*/);
	// графики
	static string log_plot(Plot p /*vector<<int,long>,string>*/);
	// Рекурсивно раскрывает include-выражения в файле
	std::stringstream expand_includes(string filename) const;
};
#endif // LIBS_LOGGER_INCLUDE_LOGGER_LOGTEMPLATE_H_