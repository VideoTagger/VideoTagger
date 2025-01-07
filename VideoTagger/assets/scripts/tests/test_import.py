from vt import *


class test_import(Script):
	def __init__(self):
		Script.__init__(self)
		self.vid_path = "assets/samples/OCR Test.avi"

	def has_progress(self: Script) -> bool:
		return False

	def on_run(self) -> None:
		project = current_project()
		if project is None:
			error("Failed to get current project")
			return
		video = project.import_video(self.vid_path)
		if video is None:
			error("Failed to import video")
			return

		log(
			f"Video successfully imported from '{video.path}'\nVideo id: {video.id}\nVideo size: {video.size}"
		)
		if project.remove_video(video):
			log("Video successfully removed")
