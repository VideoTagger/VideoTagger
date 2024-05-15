#include <filesystem>
#include <string>
#include <vector>
namespace vt::utils::string
{
std::string replace_all(const std::string& input, const std::string& from, const std::string& to)
{
	std::string output="";
	const int from_size = from.size();
	for (int i=0; i<input.size()-from_size+1;i++)
	{
		char temp[100];
		std::size_t length=input.copy(temp, from_size, i);
		temp[length] = '\0';
		std::string temp_s(temp);
		if (temp_s == from)
		{
			output += to;
			i += from_size-1;

		}
		else
			output += input[i];
	}
	return output;
}

}
