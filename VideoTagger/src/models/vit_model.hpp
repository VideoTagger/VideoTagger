#pragma once
#include <models/impl/model.hpp>

namespace vt
{
	class vit_model : public impl::model
	{
	public:
		vit_model();

	private:

	public:
		virtual void download(bool wait_for_download, const std::function<void()>& callback = nullptr) override;

	private:
		std::string download_url() const;
	};
}
