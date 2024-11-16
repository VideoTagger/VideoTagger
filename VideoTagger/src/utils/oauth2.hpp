#pragma once
#include <string>
#include <string_view>

namespace vt::utils::oauth2
{
	extern std::string generate_code_verifier(size_t code_verifier_length = 64);
	extern std::string generate_code_challenge(std::string_view code_verifier);
}
