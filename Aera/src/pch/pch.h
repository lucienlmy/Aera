#pragma once
#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wldap32.lib")
#pragma comment(lib, "crypt32.lib")

#include <numbers>

#include "framework.h"
#include "rage/enums.h"
#define BRAND "Aera"
#define UPPER_BRAND "AERA"
#define LOWER_BRAND "aera"
#define DEBUG

#define SIZEOF(a) sizeof(a) / sizeof(std::remove_pointer_t<decltype(a)>)
#define COUNT(a) ((sizeof(a)/sizeof(0[a])) / ((size_t)(!(sizeof(a) % sizeof(0[a])))))
#define ONCE_PER_FRAME(a) do a while (false)
#define ONCE(v, a) static bool v{ ([&] { a }(), true) };
constexpr long double pi{std::numbers::pi_v<long double>};

template <typename T>
using comPtr = Microsoft::WRL::ComPtr<T>;
template <typename T> requires std::is_function_v<T>
using fnptr = std::add_pointer_t<T>;

template <typename Invokable, typename... InvokableArgs>
concept is_invokable_with_args = requires(Invokable callable, const InvokableArgs&... va_args)
{
	callable(va_args...);
};


class stackWalker : public StackWalker
{
public:
	stackWalker()
	{
	}

	explicit stackWalker(ExceptType exType) : StackWalker(exType)
	{
	}

	void OnOutput(LPCSTR sz_text) override;
	void OnLoadModule(LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size, DWORD result, LPCSTR symType,
	                  LPCSTR pdbName, ULONGLONG fileVersion) override;
	void OnSymInit(LPCSTR szSearchPath, DWORD symOptions, LPCSTR szUserName) override;
	void OnDbgHelpErr(LPCSTR szFuncName, DWORD gle, DWORD64 addr) override;
	void OnCallstackEntry(CallstackEntryType eType, CallstackEntry& entry) override;
};

namespace defines
{
	inline std::optional<std::string> get_environment_variable(const char* name)
	{
		char* value{};
		size_t value_length{};
		if (_dupenv_s(&value, &value_length, name) != 0 || !value)
		{
			return std::nullopt;
		}

		std::string result{value};
		std::free(value);
		return result;
	}

	inline std::optional<std::filesystem::path> get_environment_path(const char* name)
	{
		if (const auto value = get_environment_variable(name))
		{
			return std::filesystem::path(*value);
		}

		return std::nullopt;
	}

	inline std::filesystem::path get_storage_root()
	{
		if (const auto appdata = get_environment_path("APPDATA"))
		{
			return *appdata;
		}

		std::error_code error{};
		const auto temp = std::filesystem::temp_directory_path(error);
		if (!error)
		{
			return temp;
		}

		const auto current = std::filesystem::current_path(error);
		return error ? std::filesystem::path{} : current;
	}

	inline bool g_running{true};
	inline HMODULE g_module{};
	inline HANDLE g_thread{};
	inline LPDWORD g_main_thread{};

	inline bool is_number(const std::string& str)
	{
		return !str.empty() && std::ranges::all_of(str, [](const unsigned char c) { return std::isdigit(c) != 0; });
	}

	inline bool contains_an_number(const std::string& str)
	{
		return std::ranges::any_of(str, [](const unsigned char c) { return std::isdigit(c) != 0; });
	}

	inline std::string string_to_lower(const std::string& str)
	{
		std::string result{str};
		std::ranges::transform(result, result.begin(), [](const unsigned char c)
		{
			return static_cast<char>(std::tolower(c));
		});
		return result;
	}

	inline std::string string_to_upper(const std::string& str)
	{
		std::string result{str};
		std::ranges::transform(result, result.begin(), [](const unsigned char c)
		{
			return static_cast<char>(std::toupper(c));
		});
		return result;
	}

	inline std::vector<std::string> get_matches(const std::string& str, const std::string& ex)
	{
		std::vector<std::string> matches{};
		const std::regex expression{ex};
		std::sregex_iterator iter{str.begin(), str.end(), expression};
		const std::sregex_iterator end{};
		while (iter != end)
		{
			matches.push_back(iter->str());
			++iter;
		}
		return matches;
	}

	inline std::vector<u64> find_all_occurrences(const std::string& str, const std::string& substr)
	{
		std::vector<u64> indexes{};
		std::string::size_type index{};
		while ((index = str.find(substr, index)) != std::string::npos)
		{
			indexes.push_back(static_cast<u64>(index));
			index += substr.length();
		}
		return indexes;
	}

	inline std::string get_file_contents(const std::filesystem::path& path)
	{
		std::ifstream file{path};
		return {(std::istreambuf_iterator(file)), std::istreambuf_iterator<char>()};
	}

	inline std::string trim_string(std::string string, const char character)
	{
		string.erase(std::ranges::remove(string, character).begin(), string.end());
		return string;
	}

	template <typename T>
	float to_fixed(T number, int amount)
	{
		const float multiplier = std::pow(10.0f, static_cast<float>(amount));
		return std::round(number * multiplier) / multiplier;
	}

	inline std::string remove_zeros(float number)
	{
		std::string str = std::to_string(number);
		str.erase(str.find_last_not_of('0') + 1, std::string::npos);
		str.erase(str.find_last_not_of('.') + 1, std::string::npos);
		return str;
	}

	inline std::vector<std::string> split_string(const std::string& string, const char split)
	{
		std::vector<std::string> output{};
		size_t previous_position{};
		size_t position{string.find(split)};
		while (position != std::string::npos)
		{
			std::string substring{string.substr(previous_position, position - previous_position)};
			output.push_back(substring);
			previous_position = position + 1;
			position = string.find(split, previous_position);
		}
		const std::string last_substring{string.substr(previous_position)};
		output.push_back(last_substring);
		return output;
	}

	// Function to check if a vector contains a string
	inline bool contains_string(const std::vector<std::string>& vec, const std::string& str)
	{
		return std::ranges::find(vec, str) != vec.end();
	}

	// Function to check if a string is an integer
	inline bool is_integer(const std::string& str)
	{
		try
		{
			std::stoi(str);
			return true;
		}
		catch (const std::invalid_argument&)
		{
			return false;
		}
		catch (const std::out_of_range&)
		{
			return false;
		}
	}

	// Function to check if a string is a floating-point number
	inline bool is_float(const std::string& str)
	{
		try
		{
			std::stof(str);
			return true;
		}
		catch (const std::invalid_argument&)
		{
			return false;
		}
		catch (const std::out_of_range&)
		{
			return false;
		}
		catch (...)
		{
			return false;
		}
	}

	inline bool contains_a_character(const std::string& str)
	{
		return std::ranges::any_of(str, [](const unsigned char c) { return std::isalpha(c) != 0; });
	}
}

using namespace defines;

namespace fs = std::filesystem;
