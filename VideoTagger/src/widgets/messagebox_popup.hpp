#include <cstdint>
#include <string>
#include <vector>

namespace vt::widgets
{
	struct messagebox_result
	{
		int64_t index = -1;

		operator bool() const
		{
			return index > -1;
		}
	};

	extern messagebox_result messagebox_popup(const std::string& title, const std::string& description, const std::vector<std::string>& buttons, size_t default_button = 0);
}
