#include "parsecursor.h"

namespace
{
	inline bool newlinestart(char c)
	{
		return c == '\n' || c == '\r';
	}

	inline bool inlinespace(char c)
	{
		return !newlinestart(c) && std::isspace(c);
	}
}

bool ParseCursor::atLineEnd() noexcept
{
	return atEnd() || newlinestart(*cur);
}

void ParseCursor::move(size_t n) noexcept
{
	for (size_t i = 0; i < n; i++) if (!atEnd()) cur++;
}

char ParseCursor::peek() noexcept
{
	if (atEnd()) return '\0';
	return *cur;
}

char ParseCursor::get() noexcept
{
	if (atEnd()) return '\0';
	return *cur++;
}

bool ParseCursor::tryGet(const char* str) noexcept
{
	const char* temp = cur;
	while (*str)
	{
		if (temp == end || *temp != *str) return false;
		temp++;
		str++;
	}
	cur = temp;
	return true;
}

bool ParseCursor::tryWord(const char* str) noexcept
{
	const char* temp = cur;
	while (*str)
	{
		if (temp == end || *temp != *str) return false;
		temp++;
		str++;
	}
	if (temp != end && !std::isspace(*temp)) return false;
	cur = temp;
	return true;
}

void ParseCursor::skipws() noexcept
{
	while (cur != end && inlinespace(*cur)) cur++;
}

void ParseCursor::nextLine() noexcept
{
	while (!atEnd() && newlinestart(*cur)) cur++;
}