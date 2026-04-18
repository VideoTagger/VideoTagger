#pragma once
#include <string>
#include <memory>
#include <impl/serializable.hpp>
#include <attributes/impl/attribute_instance.hpp>

namespace vt::impl
{
	struct attribute_factory;

	struct attribute : public serializable
	{
	public:
		attribute(attribute_factory* factory, const std::string& name);
		virtual ~attribute() = default;

	private:
		std::string name_;
		attribute_factory* factory_;

	public:
		void set_name(const std::string& name);

		std::string& name();
		const std::string& name() const;
		const std::string& type_name() const;

		attribute_factory* factory() const;

		std::unique_ptr<impl::attribute_instance> deserialize_instance(const nlohmann::ordered_json& json);

		[[nodiscard]] virtual nlohmann::ordered_json serialize() const override;
		virtual void deserialize(const nlohmann::ordered_json& json) override;

		virtual std::unique_ptr<impl::attribute_instance> instantiate() = 0;


		template<typename type, typename = std::enable_if_t<std::is_base_of_v<impl::attribute_instance, type>>>
		std::unique_ptr<type> instantiate()
		{
			auto ptr = instantiate();
			return std::unique_ptr<type>{ reinterpret_cast<type*>(ptr.release()) };
		}
	};

	inline void to_json(nlohmann::ordered_json& json, const attribute& attr)
	{
		json = attr.serialize();
	}

	inline void from_json(const nlohmann::ordered_json& json, attribute& attr)
	{
		attr.deserialize(json);
	}
}
