#pragma once
#include <models/impl/model.hpp>

namespace vt
{
	class goturn_model : public impl::model
	{
	public:
		goturn_model();

	private:

	public:
		virtual void download(bool wait_for_download, const std::function<void()>& callback = nullptr) override;

	private:
		std::string download_url(size_t index) const;
		std::string prototxt_download_url() const;
	};
}
