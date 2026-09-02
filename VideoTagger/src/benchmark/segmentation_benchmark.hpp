#pragma once
#include <chrono>

#include <stb_image.h>
#include <stb_image_write.h>

#include <core/app_context.hpp>
#include <utils/filesystem.hpp>
#include <utils/json.hpp>
#include <utils/vec.hpp>
#include <system/messagebox.hpp>
#include <image/image.hpp>
#include <tasks/cancellation_token.hpp>
#include <tasks/task.hpp>

#include <models/sam2/sam2_model.hpp>
#include <models/sam3/sam3_model.hpp>
#include <models/model_load_guard.hpp>

#include <benchmark/impl/dataset_benchmark_data.hpp>
#include <benchmark/impl/dataset_benchmark_item.hpp>

#include <benchmark/coco.hpp>
#include <benchmark/davis2017.hpp>

namespace vt
{
	struct segmentation_benchmark_result
	{
		bool success{};
		utils::vec4<int> bbox;
		image<image_pixel_format::gray8> output_mask;
		std::chrono::steady_clock::duration processing_duration{};
	};

	enum class segmentation_dataset : uint8_t
	{
		coco,
		davis2017,
	};

	struct benchmark_context
	{
		static constexpr size_t annotation_limit = 1000;

		std::vector<std::string> used_methods;
		std::vector<std::unique_ptr<impl::dataset_benchmark_item>> items;
		std::unique_ptr<impl::dataset_benchmark_data> data;
		std::vector<sam2_model_variant> sam2_variants
		{
			sam2_model_variant::hiera_tiny,
			sam2_model_variant::hiera_small,
			sam2_model_variant::hiera_base_plus,
			sam2_model_variant::hiera_large,
		};

		segmentation_dataset dataset{};
		size_t current_progress{};
		size_t total_progress{};
		bool is_running{};
		bool auto_confirm{};
		template<typename type, typename = std::enable_if_t<std::is_base_of_v<impl::dataset_benchmark_item, type>>>
		void add_item(type&& item)
		{
			items.push_back(std::make_unique<type>(std::move(item)));
		}

		void clear_items()
		{
			items.clear();
		}

		size_t item_count() const
		{
			return items.size();
		}
	};

	class segmentation_benchmark
	{
	public:
		segmentation_benchmark();

	private:
		cancellation_token cancel_token_;

		std::string progress_description_;
		std::filesystem::path dataset_path_;
		std::filesystem::path benchmark_path_;
		
		benchmark_context bctx_;
		std::shared_ptr<task_state<void>> benchmark_completion_state_;

	public:
		task<void> benchmark(segmentation_dataset dataset, bool auto_confirm = false);

		std::filesystem::path get_dataset_path(const std::string& name) const;
		std::filesystem::path get_benchmark_path(const std::string& name) const;

		void set_is_running(bool running);
		bool is_running() const;
		std::string dataset_name() const;

	private:
		//@return true if the benchmark should be cancelled, false otherwise
		bool handle_cancellation();
		void throw_error(const std::string& message);
		void show_info(const std::string& message, const std::function<void()>& callback = nullptr);

		size_t increment_progress();

		void setup_popup();

		void download_files(const std::vector<benchmark_download_data>& downloads, const std::function<void(bool download_success)>& callback = nullptr);

		void try_begin_benchmark();
		void predownlod_models();
		segmentation_benchmark_result benchmark_grabcut(const image<image_pixel_format::rgb8>& img, const utils::vec4<int>& bbox);
		segmentation_benchmark_result benchmark_sam2(const image<image_pixel_format::rgb8>& img, const utils::vec4<int>& bbox, std::shared_ptr<sam2_model> sam);
		segmentation_benchmark_result benchmark_sam3(const image<image_pixel_format::rgb8>& img, const utils::vec4<int>& bbox, std::shared_ptr<sam3_model> sam);

		///@return true if the benchmark was successful, false otherwise
		bool benchmark_grabcut(const std::filesystem::path& benchmark_out_dir);
		///@return true if the benchmark was successful, false otherwise
		bool benchmark_sam2(std::shared_ptr<sam2_model> sam, const std::filesystem::path& benchmark_out_dir);
		///@return true if the benchmark was successful, false otherwise
		bool benchmark_sam3(std::shared_ptr<sam3_model> sam, const std::filesystem::path& benchmark_out_dir);

		void coco_update_description(const std::string& method_name, const utils::vec4<int>& bbox, int image_id, const utils::vec2<int>& img_size, segmentation_benchmark_result& result, size_t current_progress, size_t total_progress);
		void davis2017_update_description(const std::string& method_name, const utils::vec4<int>& bbox, const std::filesystem::path& filename, const utils::vec2<int>& img_size, segmentation_benchmark_result& result, size_t current_progress, size_t total_progress);

		void coco_benchmark_save_result(const std::string& method, const segmentation_benchmark_result& result, int image_id, int annotation_id, const std::filesystem::path& output_dir);
		void davis2017_benchmark_save_result(const std::string& method, const segmentation_benchmark_result& result, const std::filesystem::path& filename, const std::filesystem::path& output_dir);
		void save_benchmark_info(const std::filesystem::path& output_dir);

		davis2017_annotation davis2017_get_annotation(const std::filesystem::path& img_filename) const;

		std::filesystem::path coco_id_to_img_path(int image_id) const;
		std::filesystem::path davis2017_filename_to_img_path(const std::filesystem::path& filename) const;
		std::filesystem::path davis2017_filename_to_annotation_path(const std::filesystem::path& filename) const;
		std::string coco_benchmark_outfile_id(const std::string& method, int image_id, int annotation_id) const;
		std::string davis2017_benchmark_outfile_id(const std::string& method, const std::filesystem::path& filename) const;

		bool coco_should_skip(const std::string& method, int image_id, int annotation_id) const;
		bool davis2017_should_skip(const std::string& method, const std::filesystem::path& filename) const;

		static image<image_pixel_format::rgb8> load_image(const std::filesystem::path& path);
		static bool save_image(const image<image_pixel_format::rgb8>& img, const std::filesystem::path& path);
		static bool save_image(const image<image_pixel_format::gray8>& img, const std::filesystem::path& path);
	};
}
