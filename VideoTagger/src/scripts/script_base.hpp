#pragma once

namespace vt
{
	struct script_base
	{
	public:
		script_base() = default;
		virtual ~script_base() = default;

	private:
		float progress_{};
		std::string progress_info_;

	public:
		virtual bool has_progress() const = 0;
		virtual void on_run() const {};

		void set_progress_info(const std::string& progress_info);
		void set_progress(float value);
		const std::string& progress_info() const;
		float progress() const;
	};
}
