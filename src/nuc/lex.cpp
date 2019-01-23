#include "lex.h"

static bool is_letter(wchar_t c)
{
	return (c >= L'a' && c <= L'z') || (c >= L'A' && c <= 'Z');
}

static bool is_digit(wchar_t c)
{
	return c >= L'0' && c <= L'9';
}

static bool is_symbol(wchar_t c)
{
	return
		c == L'(' || c == L')' || c == L'[' || c == L']' || c == L'{' || c == L'}' ||
		c == L'~' || c == L'!' || c == L'%' || c == L'^' || c == L'&' || c == L'*' ||
		c == L'-' || c == L'+' || c == L'=' || c == L'/' || c == L',' || c == L'.' ||
		c == L'?' || c == L';' || c == L':' || c == L'|';
}

void lex(const wstring& source, vector<token>& tokens, vector<lex_error>& errors)
{
	int32_t line = 1;
	int32_t start = 1;

	for (size_t i = 0; i < source.size(); i++)
	{
		wchar_t c = source[i];

		if (c == L'\n' || c == L'\r\n')
		{
			// newline
			line++;
			start = 1;
		}
		else if (c == L' ' || c == L'\t')
		{
			// blank space
			start++;
		}
		else if (c == L'/' && i + 1 < source.size() && source[i + 1] == L'/')
		{
			// single line comment
			size_t j = i + 2;
			for (; j < source.size(); j++)
			{
				c = source[j];
				if (c == L'\n' || c == L'\r\n')
					break;
			}
			i = j;
		}
		else if (c == L'/' && i + 1 < source.size() && source[i + 1] == L'*')
		{
			// multi line comment
			int count = 1;
			size_t j = i + 2;
			for (; j < source.size(); j++)
			{
				// end of file
				if (j == source.size() - 1)
					break;

				c = source[j];
				if (c == L'*' && source[j + 1] == L'/')
				{
					// close
					if (--count == 0)
					{
						start += 2;
						j++;
						break;
					}
					else
						start++;
				}
				else if (c == L'/' && source[j + 1] == L'*')
				{
					// open
					count++;
					start += 2;
					j++;
				}
				else if (c == L'\n' || c == L'\r\n')
				{
					line++;
					start = 1;
				}
				else
					start++;
			}
			i = j;
		}
		else if (is_letter(c))
		{
			// identifier
			wstring ident = L"" + c;
			size_t j = i + 1;
			for (; j < source.size(); j++)
			{
				c = source[j];
				if (is_digit(c) || is_letter(c) || c == L'_')
					ident += c;
				else
					break;
			}

			int32_t end = start + (j - i);
			tokens.push_back(token(tok_ident, line, start, end, ident));

			start = end;
			i = j - 1;
		}
		else if (c == L'\"' || c == L'\'')
		{
			// string or char
			wstring str = L"";
			size_t j = i + 1;
			for (; j < source.size(); j++)
			{
				c = source[j];
				if (c == L'\\')
				{
					if (j + 1 == source.size())
					{
						if (source[i] == L'\"')
							errors.push_back(lex_error(err_string_unclosed, line, start, start + (j - i)));
						else
							errors.push_back(lex_error(err_char_unclosed, line, start, start + (j - i)));
						break;
					}

					if (source[j + 1] == L't')
					{
						j++;
						str += L'\t';
					}
					else if (source[j + 1] == L'n')
					{
						j++;
						str += L'\n';
					}
					else if (source[j + 1] == L'\\')
					{
						j++;
						str += L'\\';
					}
					else if (source[j + 1] == L'\'')
					{
						j++;
						str += L'\'';
					}
					else if (source[j + 1] == L'\"')
					{
						j++;
						str += L'\"';
					}
					else
						errors.push_back(lex_error(err_invalid_escape, line, start + (j - i) - 1, start + (j - i)));
				}
				else if (c == L'\n' || c == L'\r\n')
				{
					if (source[i] == L'\"')
						errors.push_back(lex_error(err_string_unclosed, line, start, start + (j - i)));
					else
						errors.push_back(lex_error(err_char_unclosed, line, start, start + (j - i)));
					j--;
					break;
				}
				else if (c == L'\"' && source[i] == L'\"')
					break;
				else if (c == L'\'' && source[i] == L'\'')
					break;
				else if (j + 1 == source.size())
				{
					if (source[i] == L'\"')
						errors.push_back(lex_error(err_string_unclosed, line, start, start + (j - i)));
					else
						errors.push_back(lex_error(err_char_unclosed, line, start, start + (j - i)));
					break;
				}
				else
					str += c;
			}

			int32_t end = start + (j - i);

			if (source[i] == L'\'')
			{
				if (str.length() == 0)
				{
					errors.push_back(lex_error(err_char_empty, line, start, start + (j - i)));
					tokens.push_back(token(tok_char, line, start, end, (wchar_t)0));
				}
				else if (str.length() > 1)
				{
					errors.push_back(lex_error(err_char_length, line, start, start + (j - i)));
					tokens.push_back(token(tok_char, line, start, end, str[0]));
				}
				else
					tokens.push_back(token(tok_char, line, start, end, str[0]));
			}
			else
				tokens.push_back(token(tok_string, line, start, end, str));

			start = end;
			i = j;
		}
		else if (is_digit(c))
		{
			// number
			wstring n = L"" + c;
			size_t j = i + 1;
			for (; j < source.size(); j++)
			{
				c = source[j];
				if (is_digit(c) || is_letter(c) || c == L'_')
					n += c;
				else
					break;
			}

			if (tokens.size() >= 1 && tokens.back() == L'.')
			{

			}

			// if preceded by '.', is fractional number
			// if preceded by '-', and '-' NOT preceded by '}', ']', ')', '"', ''', ident -> its a negative number
		}
		else if (is_symbol(c))
		{
			// symbol
			tokens.push_back(token(tok_symbol, line, start, start + 1, c));
			start++;
		}
		else
		{
			// unknown token
			errors.push_back(lex_error(err_unkown_token, line, start, start + 1));
			start++;
		}
	}
}