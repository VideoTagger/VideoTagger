#include "pch.hpp"
#include "oauth2.hpp"
#include <utils/hash.hpp>
#include <utils/random.hpp>
#include <codec/base64.hpp>

namespace vt::utils::oauth2
{
	//https://datatracker.ietf.org/doc/html/rfc7636#section-4.1
	
	std::string generate_code_verifier(size_t code_verifier_length)
	{
		static constexpr std::string_view code_verifier_alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~";

		std::string code_verifier(code_verifier_length, 0);

		for (auto& c : code_verifier)
		{
			c = code_verifier_alphabet[random::get_from_zero<size_t>(code_verifier_alphabet.size() - 1)];
		}

		return code_verifier;
	}

	std::string generate_code_challenge(std::string_view code_verifier)
	{
		auto code_verfier_hash = hash::sha256(code_verifier);
		return codec::base64::encode(std::vector<uint8_t>{ code_verfier_hash.begin(), code_verfier_hash.end() }, codec::base64::encode_table::url, true);
	}
}
