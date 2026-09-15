#pragma once
#include <type_traits>
#include <map>
#include <optional>
#include <vector>
#include <impl/serializable.hpp>
#include <attributes/impl/shape.hpp>
#include <utils/timestamp.hpp>
#include <utils/iterator_range.hpp>
#include <attributes/impl/shape_interpolator.hpp>
#include <attributes/interpolators/static_shape_interpolator.hpp>
#include <core/app_context.hpp>
#include <utils/timestamp_span.hpp>

namespace vt
{
	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	class region_data : public impl::serializable
	{
	public:
		using iterator = typename std::map<timestamp, shape_type>::iterator;
		using const_iterator = typename std::map<timestamp, shape_type>::const_iterator;

		region_data() :
			region_data{fmt::format("Region #{}", utils::random::get_mono<region_id_t>())} {}

		region_data(const std::string& name) :
			interpolator_{ std::make_unique<static_shape_interpolator<shape_type>>("None") }, name_{ name }, interpolation_keyframe_data_(interpolator_->data_point_count()) {}

	private:
		std::map<timestamp, shape_type> keyframes_;
		std::unique_ptr<impl::shape_interpolator<shape_type>> interpolator_;
		std::string name_;

		mutable std::vector<shape_interpolator_data<shape_type>> interpolation_keyframe_data_;
		mutable std::vector<const_iterator> interpolation_keyframe_its;

	public:
		/**
		 * @brief Insert a new keyframe or replace an existing one
		 * @param ts Timestamp of the keyframe
		 * @param shape Keyframe shape
		 * @return Reference to the inserted shape
		 */
		shape_type& insert_keyframe(timestamp ts, shape_type shape)
		{
			auto& value = keyframes_[ts];
			value = std::move(shape);
			return value;
		}

		/**
		 * @brief Erase a keyframe
		 * @param ts Timestamp of the keyframe to erase
		 * @return true if a keyframe was erased, false otherwise
		 */
		bool erase_keyframe(timestamp ts)
		{
			auto it = keyframes_.find(ts);
			if (it == keyframes_.end()) return false;
			
			keyframes_.erase(it);
			return true;
		}

		/**
		 * @brief Erase a keyframe
		 * @param it const_iterator to the keyframe to erase
		 * @return iterator to the next keyframe after it or end() if there are no more keyframes
		 */
		iterator erase_keyframe(const_iterator it)
		{
			auto data_it = std::find_if(interpolation_keyframe_data_.begin(), interpolation_keyframe_data_.end(), [&it](const shape_interpolator_data<shape_type>& element)
			{
				return it->first == element.ts;
			});

			if (data_it != interpolation_keyframe_data_.end())
			{
				interpolation_keyframe_data_.erase(data_it);
			}

			return keyframes_.erase(it);
		}

		/**
		 * @brief Find a keyframe at the specified timestamp
		 * @param ts Timestamp of the keyframe
		 * @return const_iterator to the found element or end() if the keyframe doesn't exist 
		 */
		const_iterator find_keyframe(timestamp ts) const
		{
			return keyframes_.find(ts);
		}
		
		/**
		 * @brief Find a keyframe at the specified timestamp
		 * @param ts Timestamp of the keyframe
		 * @return iterator to the found element or end() if the keyframe doesn't exist
		 */
		iterator find_keyframe(timestamp ts)
		{
			return keyframes_.find(ts);
		}

		/**
		 * @brief Find the keyframe at the timestamp or the closest one after it
		 * @param ts Keyframe timestamp to search for
		 * @return const_iterator to the found element. If ts is after the last keyframe or there are no keyframes, returns end()
		 */
		const_iterator next_or_current_keyframe(timestamp ts) const
		{
			return keyframes_.lower_bound(ts);
		}

		/**
		 * @brief Find the keyframe at the timestamp or the closest one after it
		 * @param ts Keyframe timestamp to search for
		 * @return iterator to the found element. If ts is after the last keyframe or there are no keyframes, returns end()
		 */
		iterator next_or_current_keyframe(timestamp ts)
		{
			return keyframes_.lower_bound(ts);
		}

		/**
		 * @brief Find the closest keyframe after the timestamp
		 * @param ts Keyframe timestamp to search for
		 * @return const_iterator to the found element. If ts is after the last keyframe or there are no keyframes, returns end()
		 */
		const_iterator next_keyframe(timestamp ts) const
		{
			return keyframes_.upper_bound(ts);
		}
		
		/**
		 * @brief Find the closest keyframe after the timestamp
		 * @param ts Keyframe timestamp to search for
		 * @return iterator to the found element. If ts is after the last keyframe or there are no keyframes, returns end()
		 */
		iterator next_keyframe(timestamp ts)
		{
			return keyframes_.upper_bound(ts);
		}

		/**
		 * @brief Find the keyframe at the timestamp or the closest one before it
		 * @param ts Keyframe timestamp to search for
		 * @return const_iterator to the found element. If ts is before the first keyframe or there are no keyframes, returns end()
		 */
		const_iterator previous_or_current_keyframe(timestamp ts) const
		{
			if (keyframes_.empty()) return end();

			auto it = keyframes_.lower_bound(ts);
			if (it != end() and it->first == ts) return it;
			if (it == begin()) return end();

			return --it;
		}

		/**
		 * @brief Find the keyframe at the timestamp or the closest one before it
		 * @param ts Keyframe timestamp to search for
		 * @return iterator to the found element. If ts is before the first keyframe or there are no keyframes, returns end()
		 */
		iterator previous_or_current_keyframe(timestamp ts)
		{
			if (keyframes_.empty())
			{
				return end();
			}

			auto it = keyframes_.lower_bound(ts);
			if (it != end() and it->first == ts) return it;
			if (it == begin()) return end();

			return --it;
		}

		/**
		 * @brief Find the closest keyframe before the timestamp
		 * @param ts Keyframe timestamp to search for
		 * @return const_iterator to the found element. If ts is before the first keyframe or there are no keyframes, returns end()
		 */
		const_iterator previous_keyframe(timestamp ts) const
		{
			auto it = keyframes_.lower_bound(ts);
			if (it == begin()) return end();

			return --it;
		}

		/**
		 * @brief Find the closest keyframe before the timestamp
		 * @param ts Keyframe timestamp to search for
		 * @return iterator to the found element. If ts is before the first keyframe or there are no keyframes, returns end()
		 */
		iterator previous_keyframe(timestamp ts)
		{
			auto it = keyframes_.lower_bound(ts);
			if (it == begin()) return end();

			return --it;
		}

		void set_name(const std::string& name)
		{
			name_ = utils::string::trim_whitespace(name);
		}

		const std::string& name() const
		{
			return name_;
		}

		/// @return Whether the timestamp is within the bound of this shape i.e. is within
		/// [min_keyframe_timestamp; max_keyframe_timestamp] or is greater than min_keyframe_timestamp
		/// if there is only one keyframe. false if there are no keyframes.
		bool is_timestamp_in_bounds(timestamp ts) const
		{
			if (keyframes_.empty()) return false;

			auto start = keyframes_.begin()->first;
			auto end = keyframes_.rbegin()->first;

			if (keyframes_.size() == 1)
			{
				return start <= ts;
			}

			return start <= ts and ts <= end;
		}

		bool update_interpolation_keyframes_(timestamp ts) const
		{
			auto gather_keyframes = [this](std::vector<const_iterator>& keyframe_its, size_t data_point_count, timestamp ts) -> size_t
			{
				size_t gathered_count = 0;

				keyframe_its.reserve(data_point_count);
				keyframe_its.clear();

				auto it = previous_or_current_keyframe(ts);
				if (it == end()) return gathered_count;

				keyframe_its.push_back(it);
				++gathered_count;
				if (data_point_count == gathered_count) return gathered_count;

				++it;
				if (it == end()) return gathered_count;

				keyframe_its.push_back(it);
				++gathered_count;
				if (data_point_count == gathered_count) return gathered_count;

				//TODO: Handle more points
				return gathered_count;
			};

			if (!is_timestamp_in_bounds(ts))
			{
				return false;
			}

			size_t data_point_count = interpolation_keyframe_data_.size();
			
			gather_keyframes(interpolation_keyframe_its, data_point_count, ts);

			if (interpolation_keyframe_data_.size() != interpolation_keyframe_its.size())
			{
				interpolation_keyframe_data_.resize(interpolation_keyframe_its.size());
			}

			for (size_t i = 0; i < interpolation_keyframe_its.size(); ++i)
			{
				interpolation_keyframe_data_[i].ts = interpolation_keyframe_its[i]->first;
				interpolation_keyframe_data_[i].shape = interpolation_keyframe_its[i]->second;
			}

			return true;
		}

		/**
		 * @brief Get the shape instance at the given timestamp
		 * 
		 * @param ts Timestamp of the desired shape.
		 * @return Shape instance at the given timestamp. If there wasn't a keyframe at ts interpolation is used to create the shape.
		 * Empty if ts was outside bounds or interpolation failed
		 */
		std::optional<shape_type> get_shape_at(timestamp ts) const
		{
			if (keyframes_.empty()) return std::nullopt;

			{
				auto it = keyframes_.find(ts);
				if (it != keyframes_.end()) return it->second;
			}

			if (!update_interpolation_keyframes_(ts)) return std::nullopt;
		
			return interpolator_->interpolate(interpolation_keyframe_data_, ts);
		}

		/**
		 * @brief Set the interpolation method used when getting a shape inbetween keyframes
		 * 
		 * @param interpolator Instance of the interpolator to use. If nullptr, sets the interpolation method to static_shape_interpolator
		 */
		void set_interpolator(std::unique_ptr<impl::shape_interpolator<shape_type>>&& interpolator)
		{
			if (interpolator == nullptr)
			{
				auto& registry = ctx_.get_shape_interpolator_registry<shape_type>();
				interpolator_ = registry.new_default_interpolator();
				if (interpolator_ == nullptr)
				{
					debug::panic("No default interpolator registered");
				}
			}
			else
			{
				interpolator_ = std::move(interpolator);
			}

			interpolation_keyframe_data_.resize(interpolator_->data_point_count());
		}

		utils::timestamp_span keyframes_timespan() const
		{
			if (empty()) return {};

			auto start_ts = begin()->first;
			auto end_ts = (--end())->first;

			return { start_ts, end_ts };
		}

		impl::shape_interpolator<shape_type>& interpolator()
		{
			return *interpolator_;
		}

		const impl::shape_interpolator<shape_type>& interpolator() const
		{
			return *interpolator_;
		}

		const std::string& interpolator_name() const
		{
			return interpolator_->name();
		}

		bool is_keyframe(timestamp ts) const
		{
			return keyframes_.find(ts) != keyframes_.end();
		}

		bool empty() const
		{
			return keyframes_.empty();
		}

		size_t size() const
		{
			return keyframes_.size();
		}

		iterator begin()
		{
			return keyframes_.begin();
		}

		const_iterator begin() const
		{
			return keyframes_.begin();
		}

		const_iterator cbegin() const
		{
			return keyframes_.cbegin();
		}

		iterator end()
		{
			return keyframes_.end();
		}

		const_iterator end() const
		{
			return keyframes_.end();
		}

		const_iterator cend() const
		{
			return keyframes_.cend();
		}

		[[nodiscard]] virtual nlohmann::ordered_json serialize() const override
		{
			nlohmann::ordered_json json;

			json["name"] = name_;
			if (interpolator_->name() != "dummy")
			{
				json["interpolator"] = interpolator_->name();
			}

			auto& keyframes_json = json["keyframes"];
			keyframes_json = nlohmann::ordered_json::array();
			for (auto& [keyframe_ts, shape] : keyframes_)
			{
				auto keyframe_json = nlohmann::ordered_json{};
				keyframe_json["timestamp"] = keyframe_ts;
				keyframe_json["shape"] = shape;
				keyframes_json.push_back(keyframe_json);
			}

			return json;
		}

		virtual void deserialize(const nlohmann::ordered_json& json) override
		{
			if (!json.contains("keyframes")) return;

			if (json.contains("name"))
			{
				name_ = json["name"];
			}

			if (json.contains("interpolator"))
			{
				std::string interpolator_name = json["interpolator"];
				auto& registry = ctx_.get_shape_interpolator_registry<shape_type>();
				auto interpolator = registry.new_interpolator(interpolator_name);
				if (interpolator == nullptr)
				{
					debug::error("JSON contained unknown interpolator '{}'", interpolator_name);
				}
				set_interpolator(std::move(interpolator));
			}

			for (auto& keyframe_json : json["keyframes"])
			{
				if (!keyframe_json.contains("timestamp") or !keyframe_json.contains("shape"))
				{
					continue;
				}

				keyframes_[keyframe_json.at("timestamp").get<timestamp>()] = keyframe_json.at("shape").get<shape_type>();
			}
		}
	};

	template<typename shape_type>
	using region_data_container = std::unordered_map<region_id_t, region_data<shape_type>>;
}
