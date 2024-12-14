from collections import defaultdict
import os
import sys
from typing import Any
from PIL import Image
import numpy as np
from vt import *

import cv2
import tesserocr


class SegmentData:
	def __init__(self):
		self.keyframes = defaultdict(list[Rectangle])

	keyframes: dict[Timestamp, list[Rectangle]]
	start: Timestamp
	end: Timestamp


class ocr_demo(Script):
	def __init__(self):
		Script.__init__(self)
		self.vid_path = "assets/samples/OCR Test.avi"
		self.group_name = "OCR Test"

	def has_progress(self: Script) -> bool:
		return True

	def progress_callback(self: Script, progress):
		self.progress_info = progress

	def get_video_text(
		self, video_path: str
	) -> tuple[dict[str, list[SegmentData]], float]:
		cap = cv2.VideoCapture(video_path)
		if not cap.isOpened():
			raise ValueError(f"Unable to open video file: {video_path}")

		fps = cap.get(cv2.CAP_PROP_FPS)
		total_frames = cap.get(cv2.CAP_PROP_FRAME_COUNT)

		segments: dict[str, list[SegmentData]] = defaultdict(list[SegmentData])

		with tesserocr.PyTessBaseAPI(path=os.environ.get("TESSDATA_PREFIX")) as api:
			timestamp = 0
			while True:
				ret, frame = cap.read()
				if not ret:
					break

				frame_number = cap.get(cv2.CAP_PROP_POS_FRAMES)
				timestamp = Timestamp(int(frame_number * 1000.0 / fps))

				gray_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
				pil_image = Image.fromarray(gray_frame)

				api.SetImage(pil_image)
				components = api.GetComponentImages(tesserocr.RIL.TEXTLINE, True)
				for img, box, _, _ in components:
					x, y, w, h = [box["x"], box["y"], box["w"], box["h"]]
					text = api.GetUTF8Text().strip()

					segment_data = SegmentData()
					segment_data.keyframes[timestamp].append(
						Rectangle(Vec2(x, y), Vec2(x + w, y + h))
					)
					segment_data.start = timestamp
					segment_data.end = timestamp
					segments[text].append(segment_data)
				progress = frame_number / total_frames
				self.progress = progress

			cap.release()
		return segments, 1000.0 / fps

	def merge_segments(
		self, segments: dict[str, list[SegmentData]], frame_offset: int
	) -> dict[str, list[SegmentData]]:
		merged_segments: dict[str, list[SegmentData]] = defaultdict(list[SegmentData])
		for text, text_segments in segments.items():
			current_segment = text_segments[0]
			for next_segment in text_segments[1:]:
				should_merge = (
					next_segment.start.total_milliseconds
					- current_segment.end.total_milliseconds
				) <= frame_offset
				if should_merge:
					new_keyframes = current_segment.keyframes | next_segment.keyframes
					current_segment.end = max(list(new_keyframes.keys()))
					last_rects: list[Rectangle] = []
					current_segment.keyframes.clear()
					for keyframe, rects in new_keyframes.items():
						if rects != last_rects:
							current_segment.keyframes[keyframe] = rects
							last_rects = rects

					merged_segments[text].append(current_segment)
				else:
					merged_segments[text].append(current_segment)
					current_segment = next_segment

		return merged_segments

	def on_run(self) -> None:
		# help(vt)
		project = current_project()
		if project is None:
			return
		project.tags.clear()
		video = project.import_video(self.vid_path)

		if video is None:
			self.progress_info = "Import failed"
			print(f"Failed to import {self.vid_path} file", file=sys.stderr)
			return

		group = VideoGroup(self.group_name)
		group.add_video(video, Timestamp(0))

		any_text_tag = Tag("Any Text", to_abgr(0x9FAFAFFF))
		text_attribute_name = "text"
		box_attribute_name = "box"
		project.tags.add_tag(any_text_tag)

		self.progress_info = "Performing OCR"
		raw_segments, fps_offset = self.get_video_text(video.path)
		self.progress_info = "Meging segments"
		text_segments = self.merge_segments(raw_segments, int(round(fps_offset)) + 1)

		tags: dict[str, Tag] = dict()
		for name in text_segments.keys():
			tag = Tag(f"Text: {name}", random_color())
			tag.add_attribute(box_attribute_name, TagAttributeType.shape)
			tag.add_attribute(text_attribute_name, TagAttributeType.string)
			project.tags.add_tag(tag)
			tags[name] = tag

		for text, segments in text_segments.items():
			for segment in segments:
				if len(segments) == 0:
					continue

				start, end = segment.start, segment.end
				group.add_segment(any_text_tag, start, end)

				vt_segment = group.add_segment(tags[text], start, end)
				if vt_segment is not None:
					vt_segment.get_attribute(video, text_attribute_name).set_string(
						text
					)

					vt_segment.get_attribute(
						video, box_attribute_name
					).set_rectangle_regions(segment.keyframes)

		project.add_group(group)
		project.group_queue.add_group(group)
		self.progress = 1.0
		self.progress_info = "Done!"
