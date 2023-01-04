#include "TestsApp/Example.h"
#include <iostream>
using namespace std;

int main() {
	cout << "Test\n";
	// Тестирование
	/*Example::test_all();
	Example::all_examples();*/
		Regex r("(a*bbb((a(|a)|)*||ba))*");
		r.to_ilieyu().annote().get_syntactic_monoid().to_txt();
}