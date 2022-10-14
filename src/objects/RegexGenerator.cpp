#include "RegexGenerator.h"

RegexGenerator::RegexGenerator() {}

RegexGenerator::RegexGenerator (vector<char> alphabet, int regex_length, int star_num) 
	: alphabet(alphabet), regex_length(regex_length), star_num(star_num) {
	cout << rand_symb();
	int max_alt_num = (regex_length - 1) / 2;
	int alt_num = rand() % max_alt_num;
	cout << max_alt_num;
}

char RegexGenerator::rand_symb()
{
	return alphabet[rand() % alphabet.size()];
}

string RegexGenerator::to_txt() {
	return "тут мог быть анекдот";
}