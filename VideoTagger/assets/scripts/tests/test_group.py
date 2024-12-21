from vt import *


class test_group(Script):
	def __init__(self):
		Script.__init__(self)
		self.vid_path = "assets/samples/OCR Test.avi"

	def has_progress(self: Script) -> bool:
		return False

	def on_run(self) -> None:
		project = current_project()
		if project is None:
			return
		video = project.import_video(self.vid_path)
		if video is None:
			return
		group = VideoGroup("New Group")
		group.add_video(video, Timestamp(0))
		project.add_group(group)
		for info in group.video_infos:
			log(f"id: {info.id} offset: {info.offset}")
			video = project.get_video(info.id)
			if video is not None:
				log(f"path: {video.path} size: {video.size}")
