#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdint>
#include <codecvt>
#include <locale>
#include <iomanip>
#include <filesystem>

struct icon
{
	std::string name;
	std::string utf8_bytes;
};

std::string utf16_to_utf8(const std::string& hex_codepoint)
{
	uint32_t codepoint = std::stoul(hex_codepoint, nullptr, 16);

	// Convert codepoint to UTF-8 using standard library
	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
	std::u32string u32str(1, static_cast<char32_t>(codepoint));
	std::string utf8 = converter.to_bytes(u32str);

	// Convert to escaped hex format
	std::ostringstream result;
	for (uint8_t c : utf8)
	{
		result << "\\x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
	}
	return result.str();
}

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " <icons.list> <output.hpp>\n";
		return 1;
	}

	std::ifstream input(argv[1]);
	if (!input)
	{
		std::cerr << "Error: Cannot open input file: " << argv[1] << "\n";
		return 1;
	}

	std::vector<icon> icons;
	std::string line;

	while (std::getline(input, line))
	{
		// Trim and skip empty/comment lines (#)
		size_t start = line.find_first_not_of(" \t\r\n");
		if (start == std::string::npos || line[start] == '#')
		{
			continue;
		}

		size_t end = line.find_last_not_of(" \t\r\n");
		line = line.substr(start, end - start + 1);

		// Parse name:codepoint
		size_t colon = line.find(':');
		if (colon == std::string::npos)
		{
			std::cerr << "Warning: Invalid line format: " << line << "\n";
			continue;
		}

		icon icon;
		icon.name = line.substr(0, colon);

		// Extract codepoint and remove comments/whitespace
		std::string codepoint_str = line.substr(colon + 1);

		// Remove everything from the # character onwards (comments)
		size_t comment_pos = codepoint_str.find('#');
		if (comment_pos != std::string::npos)
		{
			codepoint_str = codepoint_str.substr(0, comment_pos);
		}

		// Trim trailing whitespace
		size_t cp_end = codepoint_str.find_last_not_of(" \t\r\n");
		if (cp_end != std::string::npos)
		{
			codepoint_str = codepoint_str.substr(0, cp_end + 1);
		}

		// Trim leading whitespace
		size_t cp_start = codepoint_str.find_first_not_of(" \t\r\n");
		if (cp_start != std::string::npos)
		{
			codepoint_str = codepoint_str.substr(cp_start);
		}

		icon.utf8_bytes = utf16_to_utf8(codepoint_str);
		icons.push_back(icon);
	}

	input.close();

	// Skip generation if output is newer than input
	{
		std::error_code ec;
		auto input_time = std::filesystem::last_write_time(argv[1], ec);
		if (!ec)
		{
			auto output_time = std::filesystem::last_write_time(argv[2], ec);
			if (!ec && output_time >= input_time)
			{
				std::cout << argv[2] << " is up to date\n";
				return 0;
			}
		}
	}

	// Generate output file
	std::ofstream output(argv[2]);
	if (!output)
	{
		std::cerr << "Error: Cannot open output file: " << argv[2] << "\n";
		return 1;
	}

	output << "#pragma once\n"
		<< "#include <vector>\n"
		<< "#include <string>\n\n"
		<< "namespace vt::icons\n{\n"
		<< "\t//https://fonts.google.com/icons?icon.set=Material+Symbols%5Cn%5Cn&icon.style=Sharp\n\n";

	for (const auto& icon : icons)
	{
		output << "\tinline constexpr auto " << icon.name << " = \"" << icon.utf8_bytes << "\";\n";
	}

	output << "\n\tinline std::vector<std::string> all\n\t({\n\t\t";

	// Write icon names list
	for (size_t i = 0; i < icons.size(); ++i)
	{
		if (i > 0) output << ", ";
		output << icons[i].name;
	}

	output << "\n\t});\n}\n";

	std::cout << "Generated " << argv[2] << " with " << icons.size() << " icons\n";
	return 0;
}
