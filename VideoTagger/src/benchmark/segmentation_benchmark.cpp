#include "pch.hpp"
#include "segmentation_benchmark.hpp"
#include <image/image_opencv.hpp>
#include <fmt/format.h>
#include <set>
#include <algorithm>

namespace vt
{
	segmentation_benchmark::segmentation_benchmark()
	{
		dataset_path_ = ctx_.downloads_dir_filepath / "datasets";
		benchmark_path_ = ctx_.storage_path() / "benchmarks";
	}

	task<void> segmentation_benchmark::benchmark(segmentation_dataset dataset, bool auto_confirm)
	{
		if (is_running() and benchmark_completion_state_ != nullptr)
		{
			return task<void>{ benchmark_completion_state_ };
		}

		auto completion_state = std::make_shared<task_state<void>>();
		benchmark_completion_state_ = completion_state;

		bctx_.dataset = dataset;
		bctx_.auto_confirm = auto_confirm;
		auto dataset_path = get_dataset_path(dataset_name());

		switch (bctx_.dataset)
		{
			case segmentation_dataset::coco:
			{
				bctx_.data = std::make_unique<coco_benchmark_data>();
				auto& data = *reinterpret_cast<coco_benchmark_data*>(bctx_.data.get());

				data.annotations_file = dataset_path / "annotations_trainval2017" / "annotations" / "instances_val2017.json";
				data.images_dir = dataset_path / "val2017" / "val2017";

				data.downloads.push_back({ "COCO Annotations 2017", "http://images.cocodataset.org/annotations/annotations_trainval2017.zip", dataset_path / "annotations_trainval2017.zip" });
				data.downloads.push_back({ "COCO Images 2017", "http://images.cocodataset.org/zips/val2017.zip", dataset_path / "val2017.zip" });
			}
			break;
			case segmentation_dataset::davis2017:
			{
				bctx_.data = std::make_unique<davis2017_benchmark_data>();
				auto& data = *reinterpret_cast<davis2017_benchmark_data*>(bctx_.data.get());

				auto davis_path = dataset_path / "davis2017" / "DAVIS";
				data.images_dir = davis_path / "JPEGImages" / "480p";
				data.annotations_dir = davis_path / "Annotations" / "480p";
				data.set_list_path = davis_path / "ImageSets" / "2017" / "val.txt";
				std::ifstream set_list_file(data.set_list_path);
				if (!set_list_file.is_open())
				{
					throw_error(fmt::format("Failed to open the '{}' file", data.set_list_path.string()));
					return task<void>{ completion_state };
				}

				while (set_list_file.good())
				{
					std::string line;
					std::getline(set_list_file, line);
					line = utils::string::trim_whitespace(line);
					if (line.empty()) continue;
					data.used_sets.insert(line);
				}

				data.downloads.push_back({ "DAVIS 480p 2017 Dataset", "https://data.vision.ee.ethz.ch/csergi/share/davis/DAVIS-2017-trainval-480p.zip", dataset_path / "davis2017.zip" });
			}
			break;
		}

		download_files(bctx_.data->downloads, [this](bool download_success)
		{
			if (!download_success)
			{
				throw_error(fmt::format("Failed to download the '{}' dataset", dataset_name()));
				return;
			}
			try_begin_benchmark();
		});

		return task<void>{ completion_state };
	}

	std::filesystem::path segmentation_benchmark::get_dataset_path(const std::string& name) const
	{
		return dataset_path_ / name;
	}

	std::filesystem::path segmentation_benchmark::get_benchmark_path(const std::string& name) const
	{
		return benchmark_path_ / name;
	}

	void segmentation_benchmark::set_is_running(bool running)
	{
		bctx_.is_running = running;
		if (!running and benchmark_completion_state_ != nullptr)
		{
			benchmark_completion_state_->set_value();
			benchmark_completion_state_.reset();
		}
	}

	bool segmentation_benchmark::is_running() const
	{
		return bctx_.is_running;
	}

	std::string segmentation_benchmark::dataset_name() const
	{
		switch (bctx_.dataset)
		{
			case segmentation_dataset::coco: return "coco";
			case segmentation_dataset::davis2017: return "davis2017";
		}
		return "unknown";
	}

	bool segmentation_benchmark::handle_cancellation()
	{
		if (cancel_token_.is_cancelled())
		{
			debug::log("Segmentation benchmark cancelled");
			set_is_running(false);
			return true;
		}
		return false;
	}

	void segmentation_benchmark::throw_error(const std::string& message)
	{
		debug::error(message);
		messagebox::show("Error", message, messagebox_icon::error);
		set_is_running(false);
	}

	void segmentation_benchmark::show_info(const std::string& message, const std::function<void()>& callback)
	{
		debug::log(message);
		if (bctx_.auto_confirm)
		{
			if (callback != nullptr)
			{
				callback();
			}
			return;
		}
		messagebox::show("Info", message, messagebox_icon::info, callback);
	}

	size_t segmentation_benchmark::increment_progress()
	{
		return ++bctx_.current_progress;
	}

	void segmentation_benchmark::setup_popup()
	{
		progress_description_ = "Segmentation benchmark in progress";
		ctx_.global_progress_popup = std::make_unique<ui::progress_popup>
		(
			progress_description_,
			[this](ui::progress_popup& popup)
			{
				popup.set_description(progress_description_);
				return bctx_.current_progress / static_cast<float>(bctx_.total_progress);
			},
			[this](const std::optional<float>& progress)
			{
				return *progress >= 1.f or !is_running();
			},
			[this]() mutable
			{
				cancel_token_.cancel();
			}
		);
	}

	void segmentation_benchmark::download_files(const std::vector<benchmark_download_data>& downloads, const std::function<void(bool download_success)>& callback)
	{
		if (downloads.empty())
		{
			if (callback != nullptr)
			{
				callback(true);
			}
			return;
		}

		auto task = ctx_.tasks.run([this, downloads, callback]()
		{
			std::vector<download_entry*> download_entries;
			for (const auto& [name, download_url, download_path] : downloads)
			{
				auto install_dir = download_path;
				install_dir.replace_extension("");

				if (std::filesystem::exists(install_dir))
				{
					debug::log(fmt::format("{} already exists, skipping download", name));
					continue;
				}

				debug::log(fmt::format("Downloading {}", name));
				download_entries.push_back(&ctx_.downloads.submit_entry(name, download_url, download_path, [this, download_path, install_dir, name](download_entry& entry)
				{
					if (!std::filesystem::exists(install_dir))
					{
						std::filesystem::create_directories(install_dir);
					}

					auto status = entry.status();
					if (status == download_entry_status::completed)
					{
						debug::log(fmt::format("Download of {} completed", name));
						debug::log(fmt::format("Unpacking {}", name));
						auto unzip_result = utils::filesystem::unzip(entry.destination(), install_dir, true);
						if (!unzip_result.has_value())
						{
							debug::error(fmt::format("Unpacking of {} failed", name));
							return;
						}
						std::filesystem::remove(entry.destination());
					}
					else if (status == download_entry_status::failed)
					{
						debug::error(fmt::format("Download of {} failed", name));
					}
					debug::log(fmt::format("Finished downloading {}", name));
				}));
			}
			
			bool success = true;
			for (auto& entry : download_entries)
			{
				entry->wait_for_completion();
				if (entry->status() != download_entry_status::completed)
				{
					success = false;
				}
			}

			if (callback != nullptr)
			{
				callback(success);
			}
		});
	}

	void segmentation_benchmark::try_begin_benchmark()
	{
		if (is_running()) return;

		for (const auto& [name, download_url, download_path] : bctx_.data->downloads)
		{
			auto install_dir = download_path;
			install_dir.replace_extension("");

			if (!std::filesystem::exists(install_dir))
			{
				throw_error(fmt::format("Required dataset directory not found at {}", install_dir.u8string()));
				return;
			}
		}
		set_is_running(true);
		debug::log("All files exist, starting segmentation benchmark");
		auto begin_benchmark = [this]()
		{
			ctx_.tasks.run([this]()
			{
				auto name = dataset_name();
				debug::log("Starting segmentation benchmark on dataset {}...", name);
				auto dataset_path = get_dataset_path(name);

				debug::log("Benchmarking {} dataset", name);
				switch (bctx_.dataset)
				{
					case segmentation_dataset::coco:
					{
						auto& data = *reinterpret_cast<coco_benchmark_data*>(bctx_.data.get());
						const auto& annotation_json_path = data.annotations_file;

						if (!std::filesystem::exists(annotation_json_path))
						{
							auto msg = fmt::format("COCO annotation JSON file not found at {}", annotation_json_path.u8string());
							throw_error(msg);
							return;
						}

						auto annotation_json = utils::json::load_from_file(annotation_json_path);
						if (!annotation_json.contains("images"))
						{
							throw_error("COCO annotation JSON file does not contain 'images' key");
							return;
						}

						debug::log("Loading images (JSON)");
						data.images_json = annotation_json["images"];
						debug::log("Loading annotations (JSON)");
						data.annotation_json = annotation_json["annotations"];

						debug::log("Found {} images in COCO annotation JSON", data.images_json.size());
						debug::log("Found {} annotations in COCO annotation JSON", data.annotation_json.size());
						debug::log("Verifying image files...");

						for (auto& image : data.images_json)
						{
							std::string file_name = image["file_name"];
							if (!std::filesystem::is_regular_file(data.images_dir / file_name))
							{
								throw_error(fmt::format("COCO image file not found at {}", (data.images_dir / file_name).u8string()));
								return;
							}
						}
					}
					break;
					case segmentation_dataset::davis2017:
					{
						debug::log("Benchmarking DAVIS 2017 dataset");
						auto& data = *reinterpret_cast<davis2017_benchmark_data*>(bctx_.data.get());

						if (!std::filesystem::exists(data.set_list_path))
						{
							auto msg = fmt::format("DAVIS 2017 set list file not found at {}", data.set_list_path.u8string());
							throw_error(msg);
							return;
						}

						if (!std::filesystem::exists(data.images_dir))
						{
							auto msg = fmt::format("DAVIS 2017 images directory not found at {}", data.images_dir.u8string());
							throw_error(msg);
							return;
						}

						if (!std::filesystem::exists(data.annotations_dir))
						{
							auto msg = fmt::format("DAVIS 2017 annotations directory not found at {}", data.annotations_dir.u8string());
							throw_error(msg);
							return;
						}
					}
					break;
				}
				
				show_info("All dataset files successfully verified, press OK to start the benchmark", [this]()
				{
					ctx_.tasks.run([this]()
					{
						bctx_.used_methods.clear();
						bctx_.clear_items();

						switch (bctx_.dataset)
						{
							case segmentation_dataset::coco:
							{
								auto& data = *reinterpret_cast<coco_benchmark_data*>(bctx_.data.get());
								/*
								std::unordered_set<int> used_image_ids;
								for (auto& image : json_images)
								{
									auto image_path = image_dir / image["file_name"].get<std::string>();
									if (used_image_ids.size() >= annotation_limit) break;

									int image_id = image["id"];
									used_image_ids.insert(image_id);
									//auto image = load_image(image_path);
								}
								*/

								for (auto& annotation : data.annotation_json)
								{
									coco_annotation result;
									result.id = annotation["id"];
									result.image_id = annotation["image_id"];

									//if (used_image_ids.find(image_id) == used_image_ids.end()) continue;

									auto& access_count = data.image_id_to_access_count[result.image_id];
									if (access_count != 0) continue;
									++access_count;

									std::vector<float> coco_bbox = annotation["bbox"];

									for (size_t i = 0; i < std::min(coco_bbox.size(), size_t{ 4 }); ++i)
									{
										result.bbox[i] = static_cast<int>(std::round(coco_bbox[i]));
									}
									result.bbox[2] += result.bbox[0];
									result.bbox[3] += result.bbox[1];

									bctx_.add_item(std::move(result));
									if (bctx_.item_count() >= bctx_.annotation_limit) break;
								}
							}
							break;
							case segmentation_dataset::davis2017:
							{
								auto& data = *reinterpret_cast<davis2017_benchmark_data*>(bctx_.data.get());
								for (const auto& dir_entry : std::filesystem::directory_iterator(data.images_dir))
								{
									if (!dir_entry.is_directory()) continue;

									std::string set_name = dir_entry.path().filename().u8string();
									if (data.used_sets.find(set_name) == data.used_sets.end()) continue;

									for (const auto& set_entry : std::filesystem::directory_iterator(dir_entry.path()))
									{
										if (!set_entry.is_regular_file()) continue;

										auto path = set_entry.path();
										if (path.extension() != ".png" and path.extension() != ".jpg") continue;

										auto filename = std::filesystem::relative(path, data.images_dir);
										filename.replace_extension("");
										auto filename_str = filename.u8string();

										std::replace(filename_str.begin(), filename_str.end(), '\\', '/');

										auto result = davis2017_get_annotation(filename_str);
										bctx_.add_item(std::move(result));
										//if (bctx_.item_count() >= bctx_.annotation_limit) break;
									}
								}
							}
							break;
						}

						predownlod_models();

						static constexpr size_t method_count = 2;
						auto sam2_variant_count = bctx_.sam2_variants.size();
						auto sam2_1_variant_count = sam2_variant_count;
						//size_t total_progress = used_image_ids.size();

						bctx_.current_progress = 0;
						bctx_.total_progress = bctx_.item_count() * (method_count + sam2_variant_count + sam2_1_variant_count);

						setup_popup();

						auto benchmark_path = get_benchmark_path(dataset_name());

						//GrabCut
						{
							if (!benchmark_grabcut(benchmark_path)) return;
						}

						//SAM 2
						{
							auto sam = ctx_.model_registry.get_model<sam2_model>();
							for (auto variant : bctx_.sam2_variants)
							{
								sam->set_variant(variant);
								if (!benchmark_sam2(sam, benchmark_path)) return;

								std::this_thread::sleep_for(std::chrono::seconds(3));
							}
							sam->set_variant(sam2_model_variant::default_variant);
							std::this_thread::sleep_for(std::chrono::seconds(3));
						}

						//SAM 2.1
						{
							auto sam = ctx_.model_registry.get_model<sam2_1_model>();
							for (auto variant : bctx_.sam2_variants)
							{
								sam->set_variant(variant);
								if (!benchmark_sam2(sam, benchmark_path)) return;

								std::this_thread::sleep_for(std::chrono::seconds(3));
							}
							sam->set_variant(sam2_model_variant::default_variant);
							std::this_thread::sleep_for(std::chrono::seconds(3));
						}

						//SAM 3
						{
							auto sam = ctx_.model_registry.get_model<sam3_model>();
							if (!benchmark_sam3(sam, benchmark_path)) return;
						}

						save_benchmark_info(benchmark_path);
						show_info("Segmentation benchmark completed successfully", [this]()
						{
							set_is_running(false);
						});
					});
				});
			});
		};

		if (bctx_.auto_confirm)
		{
			begin_benchmark();
		}
		else
		{
			messagebox::show("Beginning Benchmark", "All files exist, starting segmentation benchmark", messagebox_icon::info, begin_benchmark);
		}
	}

	void segmentation_benchmark::predownlod_models()
	{
		auto sam2 = ctx_.model_registry.get_model<sam2_model>();
		for (auto variant : bctx_.sam2_variants)
		{
			sam2->set_variant(variant);
			debug::log("Checking if {} model exists", sam2->name());
			sam2->download(true);
		}

		auto sam21 = ctx_.model_registry.get_model<sam2_1_model>();
		for (auto variant : bctx_.sam2_variants)
		{
			sam21->set_variant(variant);
			debug::log("Checking if {} model exists", sam21->name());
			sam21->download(true);
		}

		auto sam3 = ctx_.model_registry.get_model<sam3_model>();
		if (sam3 != nullptr)
		{
			debug::log("Checking if {} model exists", sam3->name());
			sam3->download(true);
		}
	}

	segmentation_benchmark_result segmentation_benchmark::benchmark_grabcut(const image<image_pixel_format::rgb8>& img, const utils::vec4<int>& bbox)
	{
		auto img_size = img.size();
		auto start = bbox.pos_min().max({ 0, 0 });
		auto end = bbox.pos_max().min({ img_size.x() - 1, img_size.y() - 1 });

		segmentation_benchmark_result result;
		result.bbox = bbox;

		auto start_time = std::chrono::steady_clock::now();
		auto bgr_img = img.convert<image_pixel_format::bgr8>([](const image_pixel_format::rgb8& pixel)
		{
			return image_pixel_format::bgr8{ pixel.b, pixel.g, pixel.r };
		});
		auto cv_img = image_to_cvmat(bgr_img);

		cv::Rect cv_rect{ start.x(), start.y(), end.x() - start.x(), end.y() - start.y() };

		cv::Mat mask;
		cv::Mat model_fg;
		cv::Mat model_bg;

		int iterations = 1;
		try
		{
			cv::grabCut(cv_img, mask, cv_rect, model_bg, model_fg, iterations, cv::GC_INIT_WITH_RECT);
		}
		catch (const cv::Exception& e)
		{
			debug::error("Failed to perform GrabCut: {}", e.what());
			result.success = false;
			result.processing_duration = std::chrono::steady_clock::now() - start_time;
			return result;
		}
		cv::Mat result_mask = (mask == cv::GC_FGD) | (mask == cv::GC_PR_FGD);

		result.output_mask.allocate(result_mask.cols, result_mask.rows);
		auto cv_mask_data = image_to_cvmat(result.output_mask);
		result_mask.copyTo(cv_mask_data);

		result.processing_duration = std::chrono::steady_clock::now() - start_time;
		result.success = true;
		return result;
	}

	segmentation_benchmark_result segmentation_benchmark::benchmark_sam2(const image<image_pixel_format::rgb8>& img, const utils::vec4<int>& bbox, std::shared_ptr<sam2_model> sam)
	{
		auto img_size = img.size();
		auto start = bbox.pos_min().max({ 0, 0 });
		auto end = bbox.pos_max().min({ img_size.x() - 1, img_size.y() - 1 });

		segmentation_benchmark_result result;
		result.bbox = bbox;

		auto start_time = std::chrono::steady_clock::now();

		auto encoder = sam->encoder();
		if (encoder == nullptr) throw std::runtime_error("SAM2 encoder is null");
		auto decoder = sam->decoder();
		if (decoder == nullptr) throw std::runtime_error("SAM2 decoder is null");

		auto res = encoder->encode(img);

		sam2_decoder_prompt prompt;
		utils::vec4<float> sam_rect;
		for (size_t i = 0; i < 4; ++i)
		{
			sam_rect[i] = static_cast<float>(bbox[i]);
		}
		prompt.rect = sam_rect;
		auto dec_res = decoder->decode(res, prompt);
		if (dec_res.masks.empty())
		{
			return result;
		}

		cv::Mat result_mask = dec_res.masks[0];

		result.output_mask.allocate(result_mask.cols, result_mask.rows);
		auto cv_mask_data = image_to_cvmat(result.output_mask);
		result_mask.copyTo(cv_mask_data);

		result.processing_duration = std::chrono::steady_clock::now() - start_time;
		result.success = true;
		return result;
	}

	segmentation_benchmark_result segmentation_benchmark::benchmark_sam3(const image<image_pixel_format::rgb8>& img, const utils::vec4<int>& bbox, std::shared_ptr<sam3_model> sam)
	{
		auto img_size = img.size();
		auto start = bbox.pos_min().max({ 0, 0 });
		auto end = bbox.pos_max().min({ img_size.x() - 1, img_size.y() - 1 });

		segmentation_benchmark_result result;
		result.bbox = bbox;

		auto start_time = std::chrono::steady_clock::now();

		auto encoder = sam->encoder();
		if (encoder == nullptr) throw std::runtime_error("SAM2 encoder is null");
		auto decoder = sam->decoder();
		if (decoder == nullptr) throw std::runtime_error("SAM2 decoder is null");

		auto res = encoder->encode(img);

		sam3_decoder_prompt prompt;
		utils::vec4<float> sam_rect;
		for (size_t i = 0; i < 4; ++i)
		{
			sam_rect[i] = static_cast<float>(bbox[i]);
		}
		prompt.rect = sam_rect;
		auto dec_res = decoder->decode(res, prompt);
		if (dec_res.masks.empty())
		{
			return result;
		}

		cv::Mat result_mask = dec_res.masks[0];

		result.output_mask.allocate(result_mask.cols, result_mask.rows);
		auto cv_mask_data = image_to_cvmat(result.output_mask);
		result_mask.copyTo(cv_mask_data);

		result.processing_duration = std::chrono::steady_clock::now() - start_time;
		result.success = true;
		return result;
	}

	bool segmentation_benchmark::benchmark_grabcut(const std::filesystem::path& benchmark_out_dir)
	{
		std::string method_name = "grabcut";
		bctx_.used_methods.push_back(method_name);

		switch (bctx_.dataset)
		{
			case segmentation_dataset::coco:
			{
				for (auto& items : bctx_.items)
				{
					auto& annot = *reinterpret_cast<coco_annotation*>(items.get());

					auto img = load_image(coco_id_to_img_path(annot.image_id));
					auto img_size = img.size();

					if (coco_should_skip(method_name, annot.image_id, annot.id))
					{
						increment_progress();
						continue;
					}

					auto result = benchmark_grabcut(img, annot.bbox);
					coco_update_description(method_name, annot.bbox, annot.image_id, img_size, result, increment_progress(), bctx_.total_progress);
					coco_benchmark_save_result(method_name, result, annot.image_id, annot.id, benchmark_out_dir);

					if (handle_cancellation()) return false;
				}
			}
			break;
			case segmentation_dataset::davis2017:
			{
				for (auto& items : bctx_.items)
				{
					auto& annot = *reinterpret_cast<davis2017_annotation*>(items.get());

					auto img = load_image(davis2017_filename_to_img_path(annot.filename));
					auto img_size = img.size();

					if (davis2017_should_skip(method_name, annot.filename))
					{
						increment_progress();
						continue;
					}

					auto result = benchmark_grabcut(img, annot.bbox);
					davis2017_update_description(method_name, annot.bbox, annot.filename, img_size, result, increment_progress(), bctx_.total_progress);
					davis2017_benchmark_save_result(method_name, result, annot.filename, benchmark_out_dir);

					if (handle_cancellation()) return false;
				}
			}
			break;
		}
		return true;
	}

	bool segmentation_benchmark::benchmark_sam2(std::shared_ptr<sam2_model> sam, const std::filesystem::path& benchmark_out_dir)
	{
		if (sam == nullptr)
		{
			throw_error("Failed to load SAM 2 model");
			return false;
		}
		sam->download(true);
		if (!sam->load_if_needed())
		{
			throw_error("Failed to load SAM 2 model");
			return false;
		}

		auto method_name = sam->name();
		bctx_.used_methods.push_back(method_name);
		auto load_guard = model_load_guard{ sam };

		for (const auto& items : bctx_.items)
		{
			switch (bctx_.dataset)
			{
				case segmentation_dataset::coco:
				{
					auto& annot = *reinterpret_cast<coco_annotation*>(items.get());
					auto img = load_image(coco_id_to_img_path(annot.image_id));
					auto img_size = img.size();

					if (coco_should_skip(method_name, annot.image_id, annot.id))
					{
						++bctx_.current_progress;
						continue;
					}

					auto result = benchmark_sam2(img, annot.bbox, sam);
					coco_update_description(method_name, annot.bbox, annot.image_id, img_size, result, increment_progress(), bctx_.total_progress);
					coco_benchmark_save_result(method_name, result, annot.image_id, annot.id, benchmark_out_dir);
				}
				break;
				case segmentation_dataset::davis2017:
				{
					auto& annot = *reinterpret_cast<davis2017_annotation*>(items.get());
					auto img = load_image(davis2017_filename_to_img_path(annot.filename));
					auto img_size = img.size();

					if (davis2017_should_skip(method_name, annot.filename))
					{
						increment_progress();
						continue;
					}

					auto result = benchmark_sam2(img, annot.bbox, sam);
					davis2017_update_description(method_name, annot.bbox, annot.filename, img_size, result, increment_progress(), bctx_.total_progress);
					davis2017_benchmark_save_result(method_name, result, annot.filename, benchmark_out_dir);
				}
				break;
			}
			if (handle_cancellation()) return false;
		}
		return true;
	}

	bool segmentation_benchmark::benchmark_sam3(std::shared_ptr<sam3_model> sam, const std::filesystem::path& benchmark_out_dir)
	{
		if (sam == nullptr)
		{
			throw_error("Failed to load SAM 3 model");
			return false;
		}
		sam->download(true);
		if (!sam->load_if_needed())
		{
			throw_error("Failed to load SAM 3 model");
			return false;
		}

		auto method_name = sam->name();
		bctx_.used_methods.push_back(method_name);
		auto load_guard = model_load_guard{ sam };

		for (const auto& items : bctx_.items)
		{
			switch (bctx_.dataset)
			{
				case segmentation_dataset::coco:
				{
					auto& annot = *reinterpret_cast<coco_annotation*>(items.get());
					auto img = load_image(coco_id_to_img_path(annot.image_id));
					auto img_size = img.size();

					if (coco_should_skip(method_name, annot.image_id, annot.id))
					{
						increment_progress();
						continue;
					}

					auto result = benchmark_sam3(img, annot.bbox, sam);
					coco_update_description(method_name, annot.bbox, annot.image_id, img_size, result, increment_progress(), bctx_.total_progress);
					coco_benchmark_save_result(method_name, result, annot.image_id, annot.id, benchmark_out_dir);
				}
				break;
				case segmentation_dataset::davis2017:
				{
					auto& annot = *reinterpret_cast<davis2017_annotation*>(items.get());
					auto img = load_image(davis2017_filename_to_img_path(annot.filename));
					auto img_size = img.size();

					if (davis2017_should_skip(method_name, annot.filename))
					{
						increment_progress();
						continue;
					}

					auto result = benchmark_sam3(img, annot.bbox, sam);
					davis2017_update_description(method_name, annot.bbox, annot.filename, img_size, result, increment_progress(), bctx_.total_progress);
					davis2017_benchmark_save_result(method_name, result, annot.filename, benchmark_out_dir);
				}
				break;
			}
			if (handle_cancellation()) return false;
		}
		return true;
	}

	void segmentation_benchmark::coco_update_description(const std::string& method_name, const utils::vec4<int>& bbox, int image_id, const utils::vec2<int>& img_size, segmentation_benchmark_result& result, size_t current_progress, size_t total_progress)
	{
		auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(result.processing_duration).count();

		debug::log("BM [{}/{}] {} image: {}x{}, bbox: {}, annotation-id: {}: result: {} time: {}ms", current_progress, total_progress, method_name, img_size.x(), img_size.y(), bbox, image_id, result.success, duration_ms);
		progress_description_ = fmt::format("Segmentation benchmark in progress\nMethod: {}\nImage Size: {}x{}\nImage ID: {}\nBounding Box: {}\nTime: {}ms\n\nBenchmarked {} of {} items", method_name, img_size.x(), img_size.y(), image_id, bbox, duration_ms, current_progress, total_progress);
	}

	void segmentation_benchmark::davis2017_update_description(const std::string& method_name, const utils::vec4<int>& bbox, const std::filesystem::path& filename, const utils::vec2<int>& img_size, segmentation_benchmark_result& result, size_t current_progress, size_t total_progress)
	{
		auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(result.processing_duration).count();

		debug::log("BM [{}/{}] {} image: {}x{}, bbox: {}, annotation-id: {}: result: {} time: {}ms", current_progress, total_progress, method_name, img_size.x(), img_size.y(), bbox, filename.u8string(), result.success, duration_ms);
		progress_description_ = fmt::format("Segmentation benchmark in progress\nMethod: {}\nImage Size: {}x{}\nImage ID: {}\nBounding Box: {}\nTime: {}ms\n\nBenchmarked {} of {} items", method_name, img_size.x(), img_size.y(), filename.u8string(), bbox, duration_ms, current_progress, total_progress);
	}

	void segmentation_benchmark::coco_benchmark_save_result(const std::string& method, const segmentation_benchmark_result& result, int image_id, int annotation_id, const std::filesystem::path& output_dir)
	{
		auto out_id = coco_benchmark_outfile_id(method, image_id, annotation_id);
		auto out_path = output_dir / out_id;

		if (!std::filesystem::exists(output_dir))
		{
			std::filesystem::create_directories(output_dir);
		}

		std::filesystem::path json_path = fmt::format("{}.json", out_path.u8string());
		std::filesystem::path out_img_path = fmt::format("{}.png", out_path.u8string());

		nlohmann::ordered_json json;
		json["method"] = method;
		json["image_id"] = image_id;
		json["annotation_id"] = annotation_id;
		json["success"] = result.success;
		auto bbox_copy = result.bbox;

		//converts bbox from [x1, y1, x2, y2] to [x, y, width, height]
		bbox_copy[2] -= bbox_copy[0];
		bbox_copy[3] -= bbox_copy[1];
		json["bbox"] = bbox_copy;
		json["duration"] = std::chrono::duration_cast<std::chrono::milliseconds>(result.processing_duration).count();

		utils::json::write_to_file(json, json_path, true);

		if (result.success)
		{
			save_image(result.output_mask, out_img_path);
		}
		debug::log("Saved benchmark result for image {} annotation {} method {} with id: {}", image_id, annotation_id, method, out_id);
	}

	void segmentation_benchmark::davis2017_benchmark_save_result(const std::string& method, const segmentation_benchmark_result& result, const std::filesystem::path& filename, const std::filesystem::path& output_dir)
	{
		auto out_id = davis2017_benchmark_outfile_id(method, filename);
		auto filename_parent = filename.parent_path();

		auto out_dir = output_dir / filename_parent;
		auto out_path = out_dir / out_id;

		if (!std::filesystem::exists(out_dir))
		{
			std::filesystem::create_directories(out_dir);
		}

		std::filesystem::path json_path = fmt::format("{}.json", out_path.u8string());
		std::filesystem::path out_img_path = fmt::format("{}.png", out_path.u8string());

		nlohmann::ordered_json json;
		json["method"] = method;
		json["filename"] = filename.u8string();
		json["success"] = result.success;
		auto bbox_copy = result.bbox;

		//converts bbox from [x1, y1, x2, y2] to [x, y, width, height]
		bbox_copy[2] -= bbox_copy[0];
		bbox_copy[3] -= bbox_copy[1];
		json["bbox"] = bbox_copy;
		json["duration"] = std::chrono::duration_cast<std::chrono::milliseconds>(result.processing_duration).count();

		utils::json::write_to_file(json, json_path, true);

		if (result.success)
		{
			save_image(result.output_mask, out_img_path);
		}
		debug::log("Saved benchmark result for filename {} method {} with id: {}", filename.u8string(), method, out_id);
	}

	void segmentation_benchmark::save_benchmark_info(const std::filesystem::path& output_dir)
	{
		nlohmann::ordered_json json;
		json["dataset"] = dataset_name();
		auto& json_methods = json["used-methods"];
		json_methods = nlohmann::ordered_json::array();
		for (const auto& method : bctx_.used_methods)
		{
			json_methods.push_back(method);
		}

		switch (bctx_.dataset)
		{
			case segmentation_dataset::coco:
			{
				auto& data = *reinterpret_cast<coco_benchmark_data*>(bctx_.data.get());
				auto& json_images = json["used-images"];
				json_images = nlohmann::ordered_json::array();
				for (const auto& [k, v] : data.image_id_to_access_count)
				{
					json_images.push_back(k);
				}

				auto& json_annotations = json["used-annotations"];
				json_annotations = nlohmann::ordered_json::array();
				for (const auto& items : bctx_.items)
				{
					auto& annot = *reinterpret_cast<coco_annotation*>(items.get());
					json_annotations.push_back(annot.id);
				}
			}
			break;
			case segmentation_dataset::davis2017:
			{
				auto& data = *reinterpret_cast<davis2017_benchmark_data*>(bctx_.data.get());

				auto& json_images = json["used-images"];
				json_images = nlohmann::ordered_json::array();
				for (const auto& item : bctx_.items)
				{
					auto& annot = *reinterpret_cast<davis2017_annotation*>(item.get());
					json_images.push_back(annot.filename.string());
				}

				auto& json_annotations = json["used-annotations"];
				json_annotations = nlohmann::ordered_json::array();
				for (const auto& item : bctx_.items)
				{
					auto& annot = *reinterpret_cast<davis2017_annotation*>(item.get());
					json_annotations.push_back(annot.filename.string());
				}
			}
			break;
		}

		std::filesystem::create_directories(output_dir);
		auto output_path = output_dir / "benchmark_info.json";
		utils::json::write_to_file(json, output_path);
	}

	davis2017_annotation segmentation_benchmark::davis2017_get_annotation(const std::filesystem::path& img_filename) const
	{
		davis2017_annotation result;
		const auto& data = *reinterpret_cast<davis2017_benchmark_data*>(bctx_.data.get());
		auto img_path = davis2017_filename_to_annotation_path(img_filename);
		result.filename = img_filename;

		auto img = load_image(img_path);
		auto bgr_img = img.convert<image_pixel_format::bgr8>([](const image_pixel_format::rgb8& pixel)
		{
			return image_pixel_format::bgr8{ pixel.b, pixel.g, pixel.r };
		});
		auto mat = image_to_cvmat(bgr_img);

		std::set<cv::Vec3b, std::less<>> colors;
		for (int y = 0; y < mat.rows; ++y)
		{
			const auto* row = mat.ptr<cv::Vec3b>(y);
			for (int x = 0; x < mat.cols; ++x)
			{
				colors.insert(row[x]);
			}
		}

		if (colors.empty())
		{
			return result;
		}

		auto it = std::find_if(colors.begin(), colors.end(), [](const cv::Vec3b& color)
		{
			bool is_background = color[0] == 0 and color[1] == 0 and color[2] == 0;
			bool is_void = color[0] == 255 and color[1] == 255 and color[2] == 255;
			return !is_background and !is_void;
		});
		if (it == colors.end())
		{
			return result;
		}

		const auto& color = *it;
		cv::Mat mask;
		cv::inRange(mat, cv::Scalar(color[0], color[1], color[2]), cv::Scalar(color[0], color[1], color[2]), mask);

		auto bbox = cv::boundingRect(mask);
		result.bbox = { bbox.x, bbox.y, bbox.x + bbox.width, bbox.y + bbox.height };
		return result;
	}


	std::filesystem::path segmentation_benchmark::coco_id_to_img_path(int image_id) const
	{
		auto& data = *reinterpret_cast<coco_benchmark_data*>(bctx_.data.get());
		return data.images_dir / std::filesystem::path(fmt::format("{:012}.jpg", image_id));
	}

	std::filesystem::path segmentation_benchmark::davis2017_filename_to_img_path(const std::filesystem::path& filename) const
	{
		auto& data = *reinterpret_cast<davis2017_benchmark_data*>(bctx_.data.get());
		auto path = data.images_dir / filename;
		path.replace_extension(".png");
		if (!std::filesystem::is_regular_file(path))
		{
			path.replace_extension(".jpg");
		}
		return path;
	}

	std::filesystem::path segmentation_benchmark::davis2017_filename_to_annotation_path(const std::filesystem::path& filename) const
	{
		auto& data = *reinterpret_cast<davis2017_benchmark_data*>(bctx_.data.get());
		auto path = data.annotations_dir / filename;
		path.replace_extension(".png");
		if (!std::filesystem::is_regular_file(path))
		{
			path.replace_extension(".jpg");
		}
		return path;
	}

	std::string segmentation_benchmark::coco_benchmark_outfile_id(const std::string& method, int image_id, int annotation_id) const
	{
		return fmt::format("{:012}-{}-{}", image_id, annotation_id, method);
	}

	std::string segmentation_benchmark::davis2017_benchmark_outfile_id(const std::string& method, const std::filesystem::path& filename) const
	{
		auto id = filename.stem().u8string();
		return fmt::format("{:05}-{}", std::stoi(id), method);
	}

	bool segmentation_benchmark::coco_should_skip(const std::string& method, int image_id, int annotation_id) const
	{
		auto benchmark_path = get_benchmark_path(dataset_name());
		auto out_id = coco_benchmark_outfile_id(method, image_id, annotation_id);
		return std::filesystem::exists(benchmark_path / (out_id + ".json"));
	}

	bool segmentation_benchmark::davis2017_should_skip(const std::string& method, const std::filesystem::path& filename) const
	{
		auto benchmark_path = get_benchmark_path(dataset_name());
		auto out_id = davis2017_benchmark_outfile_id(method, filename);

		auto parent_dir = filename.parent_path();
		auto dir = benchmark_path / parent_dir;
		return std::filesystem::exists(dir / (out_id + ".json"));
	}

	image<image_pixel_format::rgb8> segmentation_benchmark::load_image(const std::filesystem::path& path)
	{
		image<image_pixel_format::rgb8> img;
		utils::vec2<int> size;
		int img_channels{};
		auto img_data = stbi_load(path.string().c_str(), &size[0], &size[1], &img_channels, 3);
		if (img_data == nullptr)
		{
			debug::error("Failed to load image {}: {}", path.string(), stbi_failure_reason());
			return img;
		}
		if (img_channels == 1)
		{
			image<image_pixel_format::gray8> gray_img(size);
			gray_img.set_data(reinterpret_cast<image_pixel_format::gray8*>(img_data));
			stbi_image_free(img_data);
			return gray_img.convert<image_pixel_format::rgb8>([](const image_pixel_format::gray8& pixel)
			{
				return image_pixel_format::rgb8{ pixel.value, pixel.value, pixel.value };
			});
		}

		if (img_channels != 3)
		{
			debug::error("Image {} has {} channels, expected 3", path.string(), img_channels);
			return img;
		}

		img.allocate(size);
		img.set_data(reinterpret_cast<image_pixel_format::rgb8*>(img_data));
		stbi_image_free(img_data);
		return img;
	}

	bool segmentation_benchmark::save_image(const image<image_pixel_format::rgb8>& img, const std::filesystem::path& path)
	{
		auto path_str = path.u8string();
		auto img_size = img.size();
		size_t channel_count = 3;
		return stbi_write_png(path_str.c_str(), img_size.x(), img_size.y(), channel_count, img.data(), img_size.x() * channel_count);
	}

	bool segmentation_benchmark::save_image(const image<image_pixel_format::gray8>& img, const std::filesystem::path& path)
	{
		auto path_str = path.u8string();
		auto img_size = img.size();
		size_t channel_count = 1;
		return stbi_write_png(path_str.c_str(), img_size.x(), img_size.y(), channel_count, img.data(), img_size.x() * channel_count);
	}
}
