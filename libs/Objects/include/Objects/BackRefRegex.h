#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AlgExpression.h"

class MemoryFiniteAutomaton;
class MFAState;

class BackRefRegex : public AlgExpression {
  private:
	// номер ячейки памяти (используется при Type: ref, memoryWriter)
	int cell_number = 0;
	// номер линеаризации (используется при Type: memoryWriter)
	int lin_number = 0;
	// номера ячеек, в которых "находится" нода (лежит в поддеревьях memoryWriter с этими номерами)
	std::unordered_set<int> in_cells;
	// номера ячеек, содержимое которых может начинаться на эту ноду
	std::unordered_set<int> first_in_cells;
	// номера ячеек, содержимое которых может заканчиваться на эту ноду
	std::unordered_set<int> last_in_cells;

	void copy(const AlgExpression*) override; // NOLINT(build/include_what_you_use)
	// возвращает указатель на new BackRefRegex
	BackRefRegex* make() const override;

	std::string type_to_str() const override;

	BackRefRegex* expr(const std::vector<Lexeme>&, int, int) override;
	BackRefRegex* scan_ref(const std::vector<Lexeme>&, int, int);
	BackRefRegex* scan_square_br(const std::vector<Lexeme>&, int, int);

	// возвращает вектор состояний без индексов и идентификаторов
	// 0-e состояние начальное
	std::vector<MFAState> _to_mfa() const;

	// возвращает вектор листьев дерева
	// устанавливает для них first_in_cells и last_in_cells
	std::vector<BackRefRegex*> preorder_traversal(std::unordered_set<int> _in_cells,
												  std::unordered_set<int> _first_in_cells,
												  std::unordered_set<int> _last_in_cells);

	// возвращает номера ячеек памяти, над которыми производится итерация
	void get_cells_under_iteration(std::unordered_set<int>&) const;
	// для каждой ноды возвращает множество номеров нод, которым она может предшествовать
	// для каждого перехода определяет, над какими ячейками он является итерацией
	std::unordered_map<int, std::vector<std::pair<int, std::unordered_set<int>>>> get_follow()
		const;

	// преобразует star в conc (раскрывает каждую итерацию один раз) и линеаризует memoryWriter
	void unfold_iterations(int& number); // NOLINT(runtime/references)
	// рекурсивно проверяет, является ли регулярное выражение ацикличным
	bool _is_acreg(
		std::unordered_set<int>, std::unordered_set<int>,
		std::unordered_map<int, std::unordered_set<int>>&) const; // NOLINT(runtime/references)

  public:
	BackRefRegex() = default;
	explicit BackRefRegex(const std::string&);
	explicit BackRefRegex(Type type, AlgExpression* = nullptr, AlgExpression* = nullptr);

	BackRefRegex* make_copy() const override;
	BackRefRegex(const BackRefRegex&);
	BackRefRegex& operator=(const BackRefRegex& other);

	// dynamic_cast к типу BackRefRegex*
	template <typename T> static BackRefRegex* cast(T* ptr, bool not_null_ptr = true);
	template <typename T> static const BackRefRegex* cast(const T* ptr, bool not_null_ptr = true);
	template <typename T>
	static std::vector<BackRefRegex*> cast(std::vector<T*> ptr, bool not_null_ptr = true);

	std::string to_txt() const override;

	// рекурсивное построение MFA
	MemoryFiniteAutomaton to_mfa(iLogTemplate* log = nullptr) const;
	// построение MFA по аналогии с алгоритмом Глушкова (экспериментальный метод)
	MemoryFiniteAutomaton to_mfa_additional(iLogTemplate* log = nullptr) const;

	// проверяет, является ли регулярное выражение ацикличным
	bool is_acreg(iLogTemplate* log = nullptr) const;
};