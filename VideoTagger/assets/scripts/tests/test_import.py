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
			return
		video = project.import_video(self.vid_path)
		if video is None:
			return
		project.remove_video(video)
