#include "Example.h"
#include <iostream>
using namespace std;

int main() {
	cout << "Chipollino :-)\n";
	// Example ex;
	//	 ex.regex_parsing();
	// Example::arden_test();
	Example::fa_semdet_check();
	Logger::init();
	Logger::activate();
	// Example::determinize();
	// Example::fa_merge_bisimilar();
	// Example::fa_bisimilar_check();
	// Example::fa_equal_check();
	// Example::fa_equivalent_check();
	// Example::fa_subset_check();
	Logger::finish();
	Logger::deactivate();
}