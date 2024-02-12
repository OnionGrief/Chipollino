#include <AutomataParser/Lexer.h>

void Lexer::parse_buffer(lexy_ascii_tree& tree, lexy::buffer<lexy::ascii_encoding>& buffer) {
	auto result = lexy::parse_as_tree<production>(tree, buffer, lexy_ext::report_error);

	// lexy::visualize(stdout, tree, {lexy::visualize_default});

	if (!result) {
		throw std::runtime_error("AutomataParser::Lexer::parse_buffer() ERROR");
	}
}