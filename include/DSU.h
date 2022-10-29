#pragma once
#include <map>

template <class identifier_class> class DSU {
  private:
	map<identifier_class, identifier_class> parent;

  public:
	// Создание множества
	void make_set(identifier_class v) {
		parent[v] = v;
	}

	// Поиск предка
	identifier_class find_set(identifier_class v) {
		if (v == parent[v]) return v;
		return parent[v] = find_set(parent[v]);
	}

	// Объединение множеств
	void union_sets(identifier_class a,
										   identifier_class b) {
		a = find_set(a);
		b = find_set(b);
		if (a != b) {
			parent[b] = a;
		}
	}
};