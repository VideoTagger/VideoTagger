#pragma once
#include <type_traits>
#include <map>
#include <optional>
#include <impl/serializable.hpp>
#include <attributes/impl/shape.hpp>
#include <utils/timestamp.hpp>
#include <utils/iterator_range.hpp>
#include <attributes/impl/interpolated_shape_predictor.hpp>
#include <attributes/predictors/dummy_shape_predictor.hpp>
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
			interpolator_{ std::make_unique<dummy_shape_predictor<shape_type>>("dummy")}, interpolation_keyframe_timestamps_(interpolator_->data_point_count()),
			interpolation_keyframe_shapes_(interpolator_->data_point_count()) {}

	private:
		std::map<timestamp, shape_type> keyframes_;
		std::unique_ptr<impl::interpolated_shape_predictor<shape_type>> interpolator_;

		mutable std::vector<timestamp> interpolation_keyframe_timestamps_;
		mutable std::vector<shape_type> interpolation_keyframe_shapes_;

	public:
		/**
		 * @brief Insert a new keyframe or replace an existing one
		 * @param ts Timestamp of the keyframe
		 * @param shape Keyframe shape
		 * @return Reference to the inserted shape
		 */
		shape_type& insert_keyframe(timestamp ts, const shape_type& shape)
		{
			auto& value = keyframes_[ts];
			value = shape;
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
			auto timestamp_it = std::find(interpolation_keyframe_timestamps_.begin(), interpolation_keyframe_timestamps_.end(), it->first);
			if (timestamp_it != interpolation_keyframe_timestamps_.end())
			{
				auto index = timestamp_it - interpolation_keyframe_timestamps_.begin();
				interpolation_keyframe_timestamps_.erase(timestamp_it);
				interpolation_keyframe_shapes_.erase(interpolation_keyframe_shapes_.begin() + index);
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

		/**
		 * @brief Find the closest keyframe to the timestamp
		 * @param ts Keyframe timestamp to search for
		 * @return const_iterator to the found element. If there are no keyframes, returns end()
		 */
		const_iterator closest_keyframe(timestamp ts) const
		{
			if (keyframes_.empty()) return end();

			auto it = next_or_current_keyframe(ts);
			if (it != end() and it->first == ts) return it;

			return --it;
		}

		/**
		 * @brief Find the closest keyframe to the timestamp
		 * @param ts Keyframe timestamp to search for
		 * @return const_iterator to the found element. If there are no keyframes, returns end()
		 */
		iterator closest_keyframe(timestamp ts)
		{
			if (keyframes_.empty()) return end();

			auto it = next_or_current_keyframe(ts);
			if (it != end() and it->first == ts) return it;

			return --it;
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
			static constexpr auto update_vectors = [](const std::vector<const_iterator>& its, std::vector<timestamp>& ts, std::vector<shape_type>& sh)
			{
				if (ts.size() != its.size())
				{
					ts.resize(its.size());
				}
				if (sh.size() != its.size())
				{
					sh.resize(its.size());
				}

				for (size_t i = 0; i < its.size(); ++i)
				{
					ts[i] = its[i]->first;
					sh[i] = its[i]->second;
				}
			};

			if (!is_timestamp_in_bounds(ts))
			{
				return false;
			}

			static std::vector<const_iterator> keyframe_its;
			size_t data_point_count = interpolation_keyframe_timestamps_.size();
			keyframe_its.reserve(data_point_count);
			keyframe_its.clear();

			{
				auto it = previous_or_current_keyframe(ts);
				if (it == end()) return false;

				keyframe_its.push_back(it);

				if (data_point_count == 1)
				{
					update_vectors(keyframe_its, interpolation_keyframe_timestamps_, interpolation_keyframe_shapes_);
					return true;
				}

				++it;
				if (it == end()) return true;

				keyframe_its.push_back(it);
				if (data_point_count == 2)
				{
					update_vectors(keyframe_its, interpolation_keyframe_timestamps_, interpolation_keyframe_shapes_);
					return true;
				}

			}

			//TODO: Handle more points
			return false;
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
		
			return interpolator_->stateless_predict(interpolation_keyframe_shapes_, interpolation_keyframe_timestamps_, ts);
		}

		/**
		 * @brief Set the interpolation method used when getting a shape inbetween keyframes
		 * 
		 * @param interpolator Instance of the interpolator to use. If nullptr, sets the interpolation method to dummy_shape_predictor
		 */
		void set_interpolator(std::unique_ptr<impl::interpolated_shape_predictor<shape_type>>&& interpolator)
		{
			if (interpolator == nullptr)
			{
				auto& registry = ctx_.get_shape_predictor_registry<shape_type>();
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

			interpolation_keyframe_timestamps_.resize(interpolator_->data_point_count());
			interpolation_keyframe_shapes_.resize(interpolator_->data_point_count());
		}

		utils::timestamp_span keyframes_timespan() const
		{
			if (empty()) return {};

			auto start_ts = begin()->first;
			auto end_ts = (--end())->first;

			return { start_ts, end_ts };
		}

		impl::interpolated_shape_predictor<shape_type>& interpolator()
		{
			return *interpolator_;
		}

		const impl::interpolated_shape_predictor<shape_type>& interpolator() const
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

			if (json.contains("interpolator"))
			{
				std::string interpolator_name = json["interpolator"];
				auto& registry = ctx_.get_shape_predictor_registry<shape_type>();
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
