#include "trackers_benchmark_popup.hpp"
#include <pch.hpp>

#include <stb_image.h>

#include <core/app_context.hpp>
#include <ui/widgets/text_input.hpp>
#include <ui/widgets/button_bar.hpp>
#include <widgets/controls.hpp>
#include <ui/widgets/common.hpp>
#include <ui/widgets/text.hpp>
#include <utils/filesystem.hpp>
#include <tasks/cancellation_token.hpp>
#include <attributes/impl/shape_tracker.hpp>
#include <utils/string.hpp>

#include <image/image_opencv.hpp>
#include <opencv2/highgui.hpp>

namespace vt::ui
{
	enum class predict_special_case_type : int
	{
		inactive = 0,
		initialized = 1,
		target_lost = 2,
	};

	class trackers_benchmark
	{
	public:
		using predict_type = std::variant<predict_special_case_type, rectangle_shape>;

		trackers_benchmark(const std::vector<std::string>& trackers, const std::filesystem::path& dataset_path, const std::filesystem::path& result_path) :
			trackers_{ trackers }, dataset_path_{ dataset_path }, result_path_{ result_path }
		{}

	private:
		std::vector<std::string> trackers_;
		std::filesystem::path dataset_path_;
		std::filesystem::path result_path_;
		float progress_{ 0.f };
		bool is_running_{ false };

	public:
		float get_progress() const
		{
			return progress_;
		}

		bool is_running() const
		{
			return is_running_;
		}

		void operator()(cancellation_token cancel_token)
		{
			run(cancel_token);
		}

		void run(cancellation_token cancel_token)
		{
			is_running_ = true;
			progress_ = 0.f;

			on_run(cancel_token);

			is_running_ = false;
			progress_ = 1.f;
		}

	private:
		std::optional<image<image_pixel_format::rgb8>> load_image(const std::filesystem::path& image_path)
		{
			std::optional<image<image_pixel_format::rgb8>> result;

			int image_width;
			int image_height;
			int image_channels;
			uint8_t* image_data = stbi_load(image_path.u8string().c_str(), &image_width, &image_height, &image_channels, 3);
			if (image_data != nullptr)
			{
				if (image_channels != 3)
				{
					debug::error("Sequence image {} has invalid number of channels: {}", image_path.u8string(), image_channels);
					stbi_image_free(image_data);
					return result;
				}

				result.emplace(image_width, image_height);
				result->set_data(image_data);
				stbi_image_free(image_data);
			}

			return result;
		}

		std::vector<rectangle_shape> load_groundtruth(const std::filesystem::path& groundtruth_path)
		{
			std::vector<rectangle_shape> result;
			std::ifstream file(groundtruth_path);
			if (!file.is_open())
			{
				debug::error("Failed to open ground truth file: {}", groundtruth_path.u8string());
				return result;
			}

			std::string line;
			while (std::getline(file, line))
			{
				auto components = utils::string::split(line, ',');
				if (components.size() == 4)
				{
					float x = std::stof(components[0]);
					float y = std::stof(components[1]);
					float w = std::stof(components[2]);
					float h = std::stof(components[3]);
					result.emplace_back
					(
						utils::vec2<int>{ static_cast<int>(x), static_cast<int>(y) },
						utils::vec2<int>{ static_cast<int>(x + w), static_cast<int>(y + h) }
					);
					continue;
				}
				else if (components.size() == 8)
				{
					std::array<float, 4> x{};
					std::array<float, 4> y{};

					for (size_t i = 0; i < 4; i++)
					{
						x[i] = std::stof(components[i * 2]);
						y[i] = std::stof(components[i * 2 + 1]);
					}

					auto [min_x, max_x] = std::minmax_element(x.begin(), x.end());
					auto [min_y, max_y] = std::minmax_element(y.begin(), y.end());

					result.emplace_back
					(
						utils::vec2<int>{static_cast<int>(*min_x), static_cast<int>(*min_y) },
						utils::vec2<int>{ static_cast<int>(*max_x), static_cast<int>(*max_y) }
					);
				}
			}

			return result;
		}

		std::set<std::string> load_frame_names(const std::filesystem::path& frames_path)
		{
			std::set<std::string> result;
			for (auto& entry : std::filesystem::directory_iterator(frames_path))
			{
				result.insert(entry.path().filename().u8string());
			}
			return result;
		}

		bool save_results(const std::filesystem::path& result_directory, std::vector<predict_type> predictions, std::vector<float> prediction_times)
		{
			if (!std::filesystem::is_directory(result_directory))
			{
				std::filesystem::create_directories(result_directory);
			}

			{
				auto predictions_path = result_directory / "predictions.csv";
				std::ofstream predictions_file(predictions_path);
				if (!predictions_file.is_open())
				{
					return false;
				}

				for (auto& prediction : predictions)
				{
					if (std::holds_alternative<predict_special_case_type>(prediction))
					{
						predictions_file << static_cast<int>(std::get<predict_special_case_type>(prediction));
					}
					else
					{
						const auto& rectangle = std::get<rectangle_shape>(prediction);
						predictions_file << rectangle.start.x() << ',' << rectangle.start.y() << ',' << rectangle.end.x() << ',' << rectangle.end.y();
					}

					predictions_file << '\n';
				}
			}
			
			{
				auto predictions_times_path = result_directory / "times.csv";
				std::ofstream prediction_times_file(predictions_times_path);
				if (!prediction_times_file.is_open())
				{
					return false;
				}

				for (auto& time : prediction_times)
				{
					prediction_times_file << time << '\n';
				}
			}

			return true;
		}

		void on_run(cancellation_token cancel_token)
		{
			size_t total_progress = 0;
			size_t current_progress = 0;

			for (auto& entry : std::filesystem::directory_iterator(dataset_path_))
			{
				if (!entry.is_directory()) continue;

				auto frames_path = entry.path() / "color";
				if (!std::filesystem::is_directory(frames_path)) continue;

				size_t sequence_length = 0;
				for (auto& frame_entry : std::filesystem::directory_iterator(frames_path))
				{
					sequence_length++;
				}

				total_progress += sequence_length * trackers_.size();
			}

			if (!std::filesystem::exists(result_path_))
			{
				std::filesystem::create_directories(result_path_);
			}

			if (!std::filesystem::is_directory(result_path_))
			{
				debug::error("Failed to read directory: {}", result_path_.u8string());
				return;
			}

			for (auto& entry : std::filesystem::directory_iterator(dataset_path_))
			{
				if (!entry.is_directory()) continue;

				auto frames_path = entry.path() / "color";
				if (!std::filesystem::is_directory(frames_path))
				{
					debug::error("Failed to read frames directory: {}. Skipping sequence.", frames_path.u8string());
					continue;
				}

				auto groundtruth_path = entry.path() / "groundtruth.txt";
				if (!std::filesystem::is_regular_file(groundtruth_path))
				{
					debug::error("Failed to read ground truth file: {}. Skipping sequence.", groundtruth_path.u8string());
					continue;
				}
				
				auto sequence_name = entry.path().filename().u8string();
				auto groundtruth = load_groundtruth(groundtruth_path);

				auto& tracker_registry = ctx_.get_shape_tracker_registry<rectangle_shape>();
				
				std::vector<std::unique_ptr<vt::impl::shape_tracker<rectangle_shape>>> tracker_instances(trackers_.size());
				std::vector<std::vector<predict_type>> predicted_shapes(tracker_instances.size());
				std::vector<std::vector<float>> prediction_times(tracker_instances.size());

				size_t current_frame = 0;
				for (auto& frame_name : load_frame_names(frames_path))
				{
					auto frame_path = frames_path / frame_name;
					auto image_opt = load_image(frame_path);
					if (!image_opt.has_value())
					{
						debug::error("Failed to load image: {} from sequence: {}. Skipping sequence.", frame_path.u8string(), sequence_name);
						break;
					}

					if (ctx_.global_progress_popup != nullptr)
					{
						ctx_.global_progress_popup->set_description(fmt::format("Benchmarking sequence: {}\nFrame: {}", sequence_name, current_frame));
					}

					const auto& current_groundtruth = groundtruth.at(current_frame);

					for (size_t tracker_index = 0; tracker_index < tracker_instances.size(); tracker_index++)
					{
						if (cancel_token.is_cancelled())
						{
							//cv::destroyWindow("Frame");
							return;
						}

						current_progress++;
						progress_ = static_cast<double>(current_progress) / total_progress;

						const auto& tracker_name = trackers_.at(tracker_index);
						auto& tracker_ptr = tracker_instances[tracker_index];
						if (tracker_ptr == nullptr or !tracker_ptr->is_initialized())
						{
							tracker_ptr = tracker_registry.new_tracker(tracker_name);
							if (tracker_ptr == nullptr)
							{
								debug::error(fmt::format("Failed to create tracker: {}", tracker_name));
								predicted_shapes[tracker_index].push_back(predict_special_case_type::inactive);
								prediction_times[tracker_index].push_back(std::numeric_limits<float>::quiet_NaN());
								continue;
							}

							if (!tracker_ptr->init(current_groundtruth, *image_opt))
							{
								debug::error(fmt::format("Failed to initialize tracker: {}", tracker_name));
								predicted_shapes[tracker_index].push_back(predict_special_case_type::inactive);
								prediction_times[tracker_index].push_back(std::numeric_limits<float>::quiet_NaN());
								continue;
							}

							predicted_shapes[tracker_index].push_back(predict_special_case_type::initialized);
							prediction_times[tracker_index].push_back(std::numeric_limits<float>::quiet_NaN());
							continue;
						}

						auto start_time = std::chrono::steady_clock::now();

						auto prediciton = tracker_ptr->predict(*image_opt);

						auto end_time = std::chrono::steady_clock::now();

						if (!prediciton.has_value())
						{
							predicted_shapes[tracker_index].push_back(predict_special_case_type::target_lost);
							prediction_times[tracker_index].push_back(std::numeric_limits<float>::quiet_NaN());
							continue;
						}

						//auto cv_rect = cv::Rect(prediciton->start.x(), prediciton->start.y(), prediciton->width(), prediciton->height());
						//auto cvmat = image_to_cvmat(*image_opt);
						//cv::rectangle(cvmat, cv_rect, cv::Scalar(0, 255, 0), 2);
						//cv::imshow("Frame", cvmat);
						//cv::waitKey(1);

						auto prediction_time = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time).count();
						predicted_shapes[tracker_index].push_back(*prediciton);
						prediction_times[tracker_index].push_back(prediction_time);
					}

					current_frame++;
				}

				//cv::destroyWindow("Frame");
				
				for (size_t tracker_index = 0; tracker_index < tracker_instances.size(); tracker_index++)
				{
					const auto& tracker_name = trackers_.at(tracker_index);
					auto tracker_result_dir = result_path_ / tracker_name / sequence_name;

					if (!save_results(tracker_result_dir, predicted_shapes.at(tracker_index), prediction_times.at(tracker_index)))
					{
						debug::error("Failed to save results for tracker: {} on sequence: {}", tracker_name, sequence_name);
					}
				}
			}
		}
	};

	static void start_trackers_benchmark(const std::vector<std::string>& trackers, const std::filesystem::path& dataset_path, const std::filesystem::path& result_path)
	{
		std::shared_ptr<trackers_benchmark> benchmark = std::make_shared<trackers_benchmark>(trackers, dataset_path, result_path);
		cancellation_token cancel_token;

		ctx_.tasks.run([benchmark](cancellation_token cancel_token) mutable
		{
			benchmark->run(cancel_token);
		}, cancel_token);

		ctx_.global_progress_popup = std::make_unique<ui::progress_popup>
		(
			"Benchmark in progress",
			[benchmark](ui::progress_popup&)
			{
				return benchmark->get_progress();
			},
			[benchmark](const std::optional<float>& progress)
			{
				return *progress >= 1.f or !benchmark->is_running();
			},
			[cancel_token]() mutable
			{
				cancel_token.cancel();
			}
		);
	}

	vt::ui::trackers_benchmark_popup::trackers_benchmark_popup(std::optional<bool*> open) :
		modal_popup{ "trackers-benchmark-popup", "Benchmark Trackers", open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar },
		dataset_path_input_{ "##DatasetPath", "Dataset path..." }, result_path_input_{ "##ResultPath", "Result path..." }
	{}

	void trackers_benchmark_popup::on_display()
	{
		auto& registry = ctx_.get_shape_tracker_registry<rectangle_shape>();
		const auto& tracker_names = registry.tracker_names();

		trackers_.resize(tracker_names.size());
		for (size_t i = 0; i < tracker_names.size(); i++)
		{
			trackers_[i] = { tracker_names[i], false };
		}
	}

	void trackers_benchmark_popup::on_render()
	{
		if (ImGui::BeginChild("##TrackerList", { 0.f, 150.f }))
		{
			for (auto& [name, used] : trackers_)
			{
				ui::checkbox(name, used);
			}

			ImGui::EndChild();
		}
		if (ui::button("Select all"))
		{
			for (auto& [name, used] : trackers_)
			{
				used = true;
			}
		}
		ImGui::SameLine();
		if (ui::button("Deselect all"))
		{
			for (auto& [name, used] : trackers_)
			{
				used = false;
			}
		}
		ImGui::NewLine();

		dataset_path_input_.render_with_label("Dataset path");
		ImGui::SameLine();
		if (ui::button("Browse##Dataset"))
		{
			auto result = utils::filesystem::get_folder();
			if (result)
			{
				dataset_path_input_.set_input(result.path.u8string());
			}
		}

		result_path_input_.render_with_label("Result path");
		ImGui::SameLine();
		if (ui::button("Browse##Result"))
		{
			auto result = utils::filesystem::get_folder();
			if (result)
			{
				result_path_input_.set_input(result.path.u8string());
			}
		}

		std::vector<std::pair<int, std::string>> buttons
		{
			{ 0, ctx_.lang->get("generic.confirm") },
			{ 1, ctx_.lang->get("generic.cancel") },
		};

		bool any_tracker_selected = std::any_of(trackers_.begin(), trackers_.end(), [](const auto& pair) { return pair.second; });
		bool confirm_enabled = !dataset_path_input_.input().empty() and !result_path_input_.input().empty() and any_tracker_selected;

		ImGui::NewLine();
		ui::button_bar<int>::render(buttons, confirm_enabled, [&](int id)
		{
			switch (id)
			{
			case 0:
			{
				std::vector<std::string> selected_trackers;
				for (const auto& [name, used] : trackers_)
				{
					if (used)
					{
						selected_trackers.push_back(name);
					}
				}

				start_trackers_benchmark(selected_trackers, dataset_path_input_.input(), result_path_input_.input());

				close();
				break;
			}
			default: close(); break;
			}
		});
	}
}
