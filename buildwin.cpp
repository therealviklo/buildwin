#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <filesystem>
#include <memory>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include "parsecursor.h"

void execLine(ParseCursor& pc, const std::map<std::string, std::string>& reps)
{
	static bool globEcho = true;
	bool locEcho = globEcho;

	pc.skipws();
	if (pc.atLineEnd()) return;
	if (pc.tryGet("@"))
	{
		locEcho = false;
		pc.skipws();
	}

	if (locEcho)
	{
		ParseCursor echopc = pc;
		std::string rest;
		while (!echopc.atLineEnd()) rest += echopc.get();
		std::printf("\n%s> %s\n", std::filesystem::current_path().string().c_str(), rest.c_str());
	}

	if (pc.tryWord("echo"))
	{
		pc.skipws();
		if (pc.tryWord("off"))
		{
			globEcho = false;
		}
		else if (pc.tryWord("on"))
		{
			globEcho = true;
		}
		else
		{
			std::string rest;
			while (!pc.atLineEnd()) rest += pc.get();
			std::puts(rest.c_str());
		}
		pc.skipws();
		return;
	}

	auto getArg = [&pc]() -> std::string {
		std::string op;
		auto readUntil = [&pc, &op](char endc) -> void {
			while (!pc.atLineEnd())
			{
				const char c = pc.get();
				if (c == endc) return;
				op += c;
			}
		};
		if (pc.tryGet("\""))
		{
			readUntil('"');
		}
		else if (pc.tryGet("'"))
		{
			readUntil('\'');
		}
		else
		{
			while (!pc.atLineEnd() && !std::isspace(pc.peek()))
			{
				op += pc.get();
			}
		}
		pc.skipws();
		return op;
	};
	std::vector<std::string> args;
	while (!pc.atLineEnd())
	{
		std::string newArg = getArg();
		if (reps.contains(newArg))
			args.push_back(reps.at(newArg));
		else
			args.push_back(newArg);
	}
	if (args.size())
	{
		std::vector<std::unique_ptr<char[]>> argv;
		for (const auto& arg : args)
		{
			std::unique_ptr<char[]> buf = std::make_unique<char[]>(arg.size() + 1);
			std::memcpy(buf.get(), arg.c_str(), arg.size() + 1);
			argv.push_back(std::move(buf));
		}
		argv.push_back(nullptr);

		if (pid_t pid; (pid = fork()) == 0)
		{
			execvp(args[0].c_str(), (char* const*)argv.data());
			std::exit(127);
		}
		else
		{
			int status;
			waitpid(pid, &status, 0);
		}
	}
}

int main(int argc, char* argv[])
{
	try
	{
		std::string inpname = "build.bat";
		std::map<std::string, std::string> reps{
			{"clang++", "wincc"},
			{"clang", "wincc"},
			{"g++", "wincc"},
			{"gcc", "wincc"},
			{"windres", "/usr/bin/x86_64-w64-mingw32-windres"},
			{"-std=c++20", "-std=c++2a"}
		};
		auto dispHelp = []() -> void {
			std::printf(
				"Användning: buildwin [flaggor] [fil] [flaggor]\n"
				"Flaggor:\n"
				"    -h            Visa hjälp\n"
				"    -r FRÅN TILL  Byt ut FRÅN med TILL\n"
				"    -c            Rensa alla utbyten\n"
				"    -p            Skriv ut vilka utbyten som\n"
				"                  skulle ske med dessa flaggor\n"
				"Kör \"buildwin -p\" för att se standardutbyten.\n"
				"Defaultfilnamnet är \"build.bat\"."
			);
			std::exit(0);
		};
		for (int i = 1; i < argc; i++)
		{
#define NEXTARG if (++i >= argc) dispHelp();
			if (argv[i][0] == '-')
			{
				switch (argv[i][1])
				{
					case 'h': dispHelp();
					case 'r':
					{
						NEXTARG
						NEXTARG
						reps[argv[i - 1]] = argv[i];
					}
					break;
					case 'c':
					{
						reps.clear();
					}
					break;
					case 'p':
					{
						for (const auto& rep : reps)
						{
							std::printf("%s -> %s\n", rep.first.c_str(), rep.second.c_str());
						}
					}
					return 0;
				}
			}
			else
			{
				inpname = argv[i];
			}
		}

		if (!std::filesystem::exists(std::filesystem::path(inpname)))
		{
			std::printf("Kunde inte hitta filen \"%s\"", inpname.c_str());
			return 0;
		}

		std::vector<char> data;
		{
			std::ifstream f;
			f.exceptions(std::ios::failbit | std::ios::badbit);
			f.open(inpname, std::ios::in | std::ios::binary);
			f.seekg(0, std::ios::end);
			const size_t size = f.tellg();
			f.seekg(0, std::ios::beg);
			data.resize(size);
			f.read(&data[0], size);
		}

		ParseCursor pc(data.data(), data.data() + data.size());
		pc.tryGet("\xef\xbb\xbf");
		while (!pc.atEnd())
		{
			execLine(pc, reps);
			pc.nextLine();
		}
	}
	catch (const std::exception& e)
	{
		std::fprintf(stderr, "Fel: %s\n", e.what());
		return EXIT_FAILURE;
	}
	catch (...)
	{
		std::fputs("Okänt fel", stderr);
		return EXIT_FAILURE;
	}
}
