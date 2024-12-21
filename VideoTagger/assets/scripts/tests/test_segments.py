from vt import *


class test_segments(Script):
	def __init__(self):
		Script.__init__(self)
		self.vid_path = "assets/samples/OCR Test.avi"

	def has_progress(self: Script) -> bool:
		return False

	def on_run(self) -> None:
		project = current_project()
		if project is None:
			return

		group = project.group_queue.current_group()
		if group is not None:
			for tag in project.tags.list:
				log(f"tag: {tag.name} color: {tag.color}")
				for segment in group.get_segments(tag):
					log(f"start: {segment.start} end: {segment.end}")
