#include "FiniteAutomat.h"
#include "Regex.h"
#include <iostream>

/*
��� ����������� �����, ��� �� ������ ������ �������
������������� ������� �, ��������������, �� �������.
����� ��������� ������ main
*/
class Example {
  public:
	// ������ ���������� regex �� ������
	static void determinize();
	static void remove_eps();
	static void minimize();
	static void intersection();
	static void regex_parsing();
	static void fa_equal_check();
	static void fa_bisimilar_check();
	static void fa_merge_bisimilar();
};