#include <imgui.h>
#include <string>
#include <vector>
namespace vt::widgets
{
	struct messagebox_result {
		int64_t index;
	};
	messagebox_result message_box(const std::string& title, std::string& description, const std::vector<std::string>& buttons, bool is_modal, size_t default_button);
	
	
}
