#pragma once
#include <models/impl/model.hpp>

namespace vt
{
	class da_siam_rpn_model : public impl::model
	{
	public:
		da_siam_rpn_model();

	private:

	public:
		virtual void download(bool wait_for_download, const std::function<void()>& callback = nullptr) override;

	private:
		std::string model_download_url() const;
		std::string kernel_r1_download_url() const;
		std::string kernel_cls1_download_url() const;
	};
}
