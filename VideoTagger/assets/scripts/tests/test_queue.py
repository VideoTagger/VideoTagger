from vt import *


class test_queue(Script):
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
		if video is not None:
			group = VideoGroup("New Group")
			group.add_video(video, Timestamp(0))
			project.add_group(group)
			project.group_queue.add_group(group)
		group = project.group_queue.current_group()
		if group is not None:
			log(f"name: {group.name} videos: {len(group.video_infos)}")

		log("All groups:")
		for group in project.group_queue.groups:
			log(f"name: {group.name} videos: {len(group.video_infos)}")
