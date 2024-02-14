#pragma once
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "AutomatonToImage/AutomatonToImage.h"
#include "Objects/BackRefRegex.h"
#include "Objects/FiniteAutomaton.h"
#include "Objects/MemoryFiniteAutomaton.h"
#include "Objects/PushdownAutomaton.h"
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
	void set_parameter(const std::string& key, const LogObject& value, const MetaInfo& meta = {});
	void set_theory_flag(bool value);

	// Рендерит все логи, возвращает строку
	std::string render() const;
	// загрузка шаблона
	void load_tex_template(const std::string& filename);
	std::string get_tex_template();

	struct TextStyle {
		std::string tag;
		bool is_math;
	};

	// Определяет мапу идентификатора декорации и пары <тег в латехе, нужен ли мат.режим>
	inline static const std::unordered_map<Decoration, TextStyle> decor_data = {
		{italic, {"\\textit", false}},
		{regexstyle, {"\\regexpstr", true}},
		{typewriter, {"\\ttfamily", false}},
		{roman, {"\\mathrm", true}}};

	// Определяет мапу идентификатора размера и его тега в латехе
	inline static const std::unordered_map<TextSize, std::string> textsize_to_str = {
		{footnote, "\\footnotesize"},
		{small, "\\small"},
		{normal, "\\normalsize"},
		{large, "\\large"},
		{none, ""}};

  private:
	// кеш отрендеренных автоматов
	inline static std::unordered_map<size_t, std::string> cache_automatons;
	//  Путь к папке с шаблонами
	const std::string template_path = "./resources/template/";

	// путь к LaTeX-шаблону
	std::string template_fullpath;
	// имя файла шаблона
	std::string template_filename;

	// флаг логирования подробной части
	bool render_theory = false;

	// Стуктура для хранения параметров
	struct LogParameter {
		LogObject value;
		MetaInfo meta; // Дополнительные данные о структуре, которые не кэшируются (раскраска)
	};

	// Параметры
	std::unordered_map<std::string, LogParameter> parameters;

	// Добавление шаблона настоящего параметра
	void add_parameter(std::string parameter_name);
	// счетчик картинок
	inline static int image_number = 0;
	// таблицы в общем виде
	static std::string log_table(Table t);
	// графики
	static std::string log_plot(Plot p);
	// Рекурсивно раскрывает include-выражения в файле
	std::stringstream expand_includes(std::string filename) const;
};