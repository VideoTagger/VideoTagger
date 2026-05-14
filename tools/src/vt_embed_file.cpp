#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include <regex>
#include <iomanip>
#include <cstdint>

std::string sanitize_name(const std::string& name)
{
	std::regex invalid_chars(R"([^a-zA-Z0-9_])");
	return std::regex_replace(name, invalid_chars, "_");
}

std::vector<uint8_t> read_file_bytes(const std::filesystem::path& filepath)
{
	std::ifstream file(filepath, std::ios::binary | std::ios::ate);
	if (!file)
	{
		throw std::runtime_error("Failed to open file: " + filepath.string());
	}

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<uint8_t> buffer(size);
	if (!file.read(reinterpret_cast<char*>(buffer.data()), size))
	{
		throw std::runtime_error("Failed to read file: " + filepath.string());
	}

	return buffer;
}

void write_header(const std::filesystem::path& output_path, const std::string& safe_name, const std::string& namespace_name)
{
	std::ofstream header(output_path);
	if (!header)
	{
		throw std::runtime_error("Failed to create header file: " + output_path.string());
	}

	header << "#pragma once\n";
	header << "#include <cstdint>\n";
	header << "#include <cstddef>\n\n";
	header << "namespace " << namespace_name << "\n{\n";
	header << "\textern const size_t " << safe_name << "_size;\n";
	header << "\textern const uint8_t " << safe_name << "[];\n";
	header << "}\n";
}

void write_source
(
	const std::filesystem::path& output_path, const std::string& safe_name,
	const std::string& header_name, const std::string& namespace_name,
	const std::vector<uint8_t>& bytes
)
{
	std::ofstream source(output_path);
	if (!source)
	{
		throw std::runtime_error("Failed to create source file: " + output_path.string());
	}

	source << "#include \"" << header_name << "\"\n\n";
	source << "namespace " << namespace_name << "\n{\n";
	source << "\tconst size_t " << safe_name << "_size = " << bytes.size() << ";\n\n";
	source << "\tconst uint8_t " << safe_name << "[] =\n\t{\n";

	const size_t line_length = 16;
	for (size_t i = 0; i < bytes.size(); i += line_length)
	{
		source << "\t\t";
		for (size_t j = i; j < i + line_length && j < bytes.size(); ++j) {
			source << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
				<< static_cast<int>(bytes[j]) << ", ";
		}
		source << "\n";
	}

	source << "\t};\n";
	source << "}\n";
}

void embed_file(const std::filesystem::path& source_file, const std::filesystem::path& output_dir, const std::string& namespace_name)
{
	if (!std::filesystem::exists(source_file))
	{
		throw std::runtime_error("Source file does not exist: " + source_file.string());
	}

	if (!std::filesystem::exists(output_dir))
	{
		std::filesystem::create_directories(output_dir);
	}

	std::string source_stem = source_file.stem().string();
	std::string safe_name = sanitize_name(source_stem);

	std::filesystem::path header_path = output_dir / (safe_name + ".hpp");
	std::filesystem::path source_path = output_dir / (safe_name + ".cpp");

	std::cout << "Embedding '" << source_file.filename().string() << "' into '" << safe_name << "'...\n";

	std::vector<uint8_t> bytes = read_file_bytes(source_file);

	write_header(header_path, safe_name, namespace_name);
	write_source(source_path, safe_name, safe_name + ".hpp", namespace_name, bytes);
}

int main(int argc, char* argv[])
{
	try
	{
		if (argc < 3)
		{
			std::cerr << "Usage: " << (argc > 0 ? argv[0] : "embed_file")
				<< " <source_file> <output_directory> [namespace]\n";
			std::cerr << "  source_file: Path to the file to embed\n";
			std::cerr << "  output_directory: Directory where .hpp and .cpp will be generated\n";
			std::cerr << "  namespace: Optional namespace (default: \"vt::embed\")\n";
			return 1;
		}

		std::filesystem::path source_file = argv[1];
		std::filesystem::path output_dir = argv[2];
		std::string namespace_name = (argc > 3) ? argv[3] : "vt::embed";

		embed_file(source_file, output_dir, namespace_name);

		std::cout << "Done!\n";
		return 0;

	}
	catch (const std::exception& ex)
	{
		std::cerr << "Error: " << ex.what() << "\n";
		return 1;
	}
}
