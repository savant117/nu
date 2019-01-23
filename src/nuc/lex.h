#pragma once

#include <string>
#include <vector>
using namespace std;

enum token_type
{
	tok_unknown,
	tok_ident,
	tok_number,
	tok_char,
	tok_string,
	tok_symbol,
};

struct token
{
	token_type type;	// actual token type
	int32_t line;		// line number
	int32_t start;		// start column
	int32_t end;		// end column
	wstring str_val;	// tok_ident, tok_string
	bool neg_val;		// tok_number
	uint64_t whole_val; // tok_number
	uint64_t frac_val;	// tok_number
	int32_t exp_val;	// tok_number
	wchar_t char_val;	// tok_char, tok_symbol

	token() : type(tok_unknown) 
	{}

	token(token_type type, int line, int start, int end)
		: type(type), line(line), start(start), end(end)
	{}

	token(token_type type, int line, int start, int end, const wstring& str_val)
		: type(type), line(line), start(start), end(end), str_val(str_val)
	{}

	token(token_type type, int line, int start, int end, bool neg_val, uint64_t whole_val, uint64_t frac_val, int32_t exp_val)
		: type(type), line(line), start(start), end(end), neg_val(neg_val), whole_val(whole_val), frac_val(frac_val), exp_val(exp_val)
	{}

	token(token_type type, int line, int start, int end, wchar_t char_val)
		: type(type), line(line), start(start), end(end), char_val(char_val)
	{}

	bool operator==(wchar_t c) const
	{
		return type == tok_symbol && char_val == c;
	}

	bool operator==(const wstring& s) const
	{
		return type == tok_ident && str_val == s;
	}
};

enum lex_error_type
{
	err_number_too_large,	// number was too large to fit into uint64
	err_unkown_token,		// encountered an unhandled token
	err_string_unclosed,	// string was not closed before end of line
	err_char_unclosed,		// char was not closed before end of line
	err_char_length,		// char contained more than one character
	err_char_empty,			// char is empty
	err_invalid_literal,	// literal contains invalid characters
	err_invalid_escape,		// invalid string or char escape character
};

struct lex_error
{
	lex_error_type type;	// actual error type
	int32_t line;			// line number
	int32_t start;			// start column
	int32_t end;			// end column

	lex_error() {}

	lex_error(lex_error_type type, int32_t line, int32_t start, int32_t end)
		: type(type), line(line), start(start), end(end)
	{}
};

void lex(const wstring& source, vector<token>& tokens, vector<lex_error>& errors);