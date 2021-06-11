#pragma once
#include <cstddef>
#include <cctype>

class ParseCursor
{
private:
	const char* cur;
	const char* end;
public:
	constexpr ParseCursor(const char* start, const char* end) noexcept:
		cur(start),
		end(end) {}

	constexpr bool atEnd() noexcept { return cur == end; }
	bool atLineEnd() noexcept;
	
	void move(size_t n = 1) noexcept;
	char peek() noexcept;
	char get() noexcept;
	bool tryGet(const char* str) noexcept;
	bool tryWord(const char* str) noexcept;

	void skipws() noexcept;
	void nextLine() noexcept;
};