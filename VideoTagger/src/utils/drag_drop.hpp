#pragma once
#include <string>
#include <optional>
#include <type_traits>
#include <typeinfo>

#include <core/debug.hpp>

#include <imgui.h>

namespace vt::utils
{
	struct drag_drop
	{
		template<typename type>
		struct payload
		{
			std::optional<type> data;
			const ImGuiPayload* imgui_payload{};
		};

		drag_drop() = delete;

		template<typename type>
		static bool set_payload(const std::string& name, const type& payload)
		{
			static_assert(std::is_standard_layout<type>::value, "Payload needs to be a primitive type");
			return ImGui::SetDragDropPayload(name.c_str(), &payload, sizeof(payload));
		}

		template<typename type>
		static payload<type> get_payload(const std::string& name, ImGuiDragDropFlags flags = 0)
		{
			auto imgui_payload = ImGui::AcceptDragDropPayload(name.c_str(), flags);
			if (imgui_payload == nullptr) return {};

			if (imgui_payload->DataSize != sizeof(type))
			{
				debug::error("Paylod size mismatch, expected {} but got {} for type: {}", imgui_payload->DataSize, sizeof(type), typeid(type).name());
				return {};
			}

			return { *reinterpret_cast<type*>(imgui_payload->Data), imgui_payload };
		}
	};
}
