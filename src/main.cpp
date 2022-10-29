#include "Example.h"
#include <iostream>
using namespace std;

int main() {
	cout << "Chipollino :-)\n";
	Logger::init();
	Logger::activate();
	Example::determinize();
	// Example::fa_merge_bisimilar();
	// Example::fa_bisimilar_check();
	// Example::fa_equal_check();
	// Example::fa_equivalent_check();
	// Example::fa_subset_check();
	// Example::tester();
	Logger::finish();
	Logger::deactivate();
}