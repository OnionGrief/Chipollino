#pragma once
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AlgExpression.h"
#include "MemoryCommon.h"
#include "iLogTemplate.h"

class MemoryFiniteAutomaton;
class MFAState;
class Regex;

// Cell -> pair{A, B}
// A = true, если потенциально пустая ячейка будет пройдена в любом случае
// (иначе есть пути, которые ее не включают).
// B - множество ячеек, от пустоты которых может зависеть пустота Cell.
using ToResetMap = std::unordered_map<Cell, std::pair<bool, CellSet>, Cell::Hasher>;

class BackRefRegex : public AlgExpression {
  private:
	// номер ячейки памяти (используется при Type: ref, memoryWriter)
	int cell_number = 0;
	// номер линеаризации (используется при Type: memoryWriter)
	int lin_number = 0;
	// для ref - указатель на memoryWriter
	BackRefRegex* ref_to = nullptr;
	// признак для ref - может ли ссылаться на пустую память
	bool may_be_eps = false;

	void copy(const AlgExpression*) override; // NOLINT(build/include_what_you_use)
	// возвращает указатель на new BackRefRegex
	BackRefRegex* make() const override;

	std::string type_to_str() const override;

	BackRefRegex* expr(const std::vector<Lexeme>&, int, int) override;
	BackRefRegex* scan_ref(const std::vector<Lexeme>&, int, int);
	BackRefRegex* scan_square_br(const std::vector<Lexeme>&, int, int);

	bool equals(const AlgExpression* other) const override;

	// возвращает вектор состояний без индексов и идентификаторов
	// 0-e состояние начальное
	std::vector<MFAState> _to_mfa() const;

	Cell get_cell() const;

	// возвращает вектор листьев дерева
	// устанавливает для них in_lin_cells, first_in_cells и last_in_cells
	// линеаризует memoryWriters
	void preorder_traversal(
		std::vector<BackRefRegex*>& terms,					// NOLINT(runtime/references)
		int& lin_counter,									// NOLINT(runtime/references)
		std::vector<std::unordered_set<int>>& in_lin_cells, // NOLINT(runtime/references)
		std::vector<CellSet>& first_in_cells,				// NOLINT(runtime/references)
		std::vector<CellSet>& last_in_cells,				// NOLINT(runtime/references)
		std::unordered_set<int> cur_in_lin_cells, CellSet cur_first_in_cells,
		CellSet cur_last_in_cells);
	bool contains_eps() const override;
	// вычисляет поля may_be_eps (передать пустой вектор на вход)
	void calculate_may_be_eps(
		std::unordered_map<int, std::vector<BackRefRegex*>>&); // NOLINT(runtime/references)

	// ToResetMap заполняется ячейками, которые нужно сбросить.
	std::pair<bool, ToResetMap> contains_eps_tracking_resets() const; // NOLINT(runtime/references)
	// пары {нода, {потенциально пустые memoryWriter, которые стоят до нее}}
	std::vector<std::pair<AlgExpression*, ToResetMap>> get_first_nodes_tracking_resets();
	// пары {нода, {потенциально пустые memoryWriter, которые стоят после нее}}
	std::vector<std::pair<AlgExpression*, ToResetMap>> get_last_nodes_tracking_resets();
	// возвращает номера ячеек памяти, над которыми производится итерация
	void get_cells_under_iteration(std::unordered_set<int>&) const;
	// для каждого терма определяет множество номеров термов, которым он может предшествовать
	// для каждого перехода определяет, над какими ячейками он является итерацией,
	// и какие ячейки памяти он должен сбросить
	void get_follow(std::vector<std::vector<std::tuple<int, std::unordered_set<int>,
													   CellSet>>>& // NOLINT(runtime/references)
	) const;

	// преобразует star в conc (раскрывает каждую итерацию один раз) и линеаризует memoryWriters
	void unfold_iterations(int& number); // NOLINT(runtime/references)
	// рекурсивно проверяет, является ли регулярное выражение ацикличным
	bool _is_acreg(
		std::unordered_set<int>, std::unordered_set<int>,
		std::unordered_map<int, std::unordered_set<int>>&) const; // NOLINT(runtime/references)

	void linearize_refs(int& number); // NOLINT(runtime/references)
	void _check_memory_writers(std::unordered_map<int, std::unordered_set<int>>&,
							   std::unordered_set<int>&,		// NOLINT(runtime/references)
							   std::unordered_set<int>&) const; // NOLINT(runtime/references)

	// меняет порядок конкатенаций в дереве (swap term_l и term_r)
	void _reverse(std::unordered_map<int, BackRefRegex*>&); // NOLINT(runtime/references)
	// используется при обращении ([]:i <-> &i)
	void swap_memory_operations(std::unordered_set<BackRefRegex*>&); // NOLINT(runtime/references)

  public:
	BackRefRegex() = default;
	explicit BackRefRegex(const std::string&);
	BackRefRegex(const Regex* regex, const Alphabet& _alphabet);
	explicit BackRefRegex(const Regex* regex);

	BackRefRegex* make_copy() const override;
	BackRefRegex(const BackRefRegex&);
	BackRefRegex& operator=(const BackRefRegex& other);

	// dynamic_cast к типу BackRefRegex*
	template <typename T> static BackRefRegex* cast(T* ptr, bool not_null_ptr = true);
	template <typename T> static const BackRefRegex* cast(const T* ptr, bool not_null_ptr = true);
	template <typename T>
	static std::vector<BackRefRegex*> cast(std::vector<T*> ptr, bool not_null_ptr = true);

	std::string to_txt() const override;

	// проверка регулярок на равенство (пока работает только для стандартного построения)
	static bool equal(const BackRefRegex&, const BackRefRegex&, iLogTemplate* log = nullptr);
	// рекурсивное построение MFA
	MemoryFiniteAutomaton to_mfa(iLogTemplate* log = nullptr) const;
	// построение MFA по аналогии с алгоритмом Глушкова (экспериментальный метод)
	MemoryFiniteAutomaton to_mfa_additional(iLogTemplate* log = nullptr) const;
	// проверяет, является ли регулярное выражение ацикличным
	bool is_acreg(iLogTemplate* log = nullptr) const;
	// обращение выражения (для СНФ)
	BackRefRegex reverse(iLogTemplate* log = nullptr) const;
	// проверяет, что каждая ссылка может следовать за записью в память (соответствующую ячейку)
	// и что каждый memoryWriter не будет однозначно переинициализирован без возможности
	// сослаться на него (существует хотя бы один путь, в котором присутствует ссылка на него)
	bool check_refs_and_memory_writers_usefulness() const;
	BackRefRegex rewrite_aci() const;
};