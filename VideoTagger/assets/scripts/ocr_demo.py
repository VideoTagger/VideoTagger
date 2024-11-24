from collections import defaultdict
import os
import sys
from typing import Any
from PIL import Image
import numpy as np
import vt
import time

import cv2
import tesserocr


class ocr_demo(vt.Script):
	def __init__(self):
		vt.Script.__init__(self)
		self.vid_path = "assets/samples/OCR Test.avi"
		self.group_name = "OCR Test"

	def has_progress(self: vt.Script) -> bool:
		return True

	def progress_callback(self: vt.Script, progress):
		self.progress_info = progress

	def get_video_text(self, video_path: str) -> list[dict[str, Any]]:
		cap = cv2.VideoCapture(video_path)
		if not cap.isOpened():
			raise ValueError(f"Unable to open video file: {video_path}")

		fps = cap.get(cv2.CAP_PROP_FPS)
		total_frames = cap.get(cv2.CAP_PROP_FRAME_COUNT)

		segments = []

		with tesserocr.PyTessBaseAPI(path=os.environ.get("TESSDATA_PREFIX")) as api:
			timestamp = 0
			while True:
				ret, frame = cap.read()
				if not ret:
					break

				frame_number = cap.get(cv2.CAP_PROP_POS_FRAMES)
				timestamp = frame_number * 1000.0 / fps

				gray_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
				pil_image = Image.fromarray(gray_frame)

				api.SetImage(pil_image)
				boxes = defaultdict(list[list[Any]])
				components = api.GetComponentImages(tesserocr.RIL.TEXTLINE, True)
				conf_avg = 0
				for img, box, _, _ in components:
					bounding_box = [box["x"], box["y"], box["w"], box["h"]]
					conf_avg += api.MeanTextConf()
					text = api.GetUTF8Text().strip()
					# print(
					# 	"Box: x={x}, y={y}, w={w}, h={h}, confidence: {0}, text: {1}".format(
					# 		conf, text, **box
					# 	)
					# )
					boxes[text].append(bounding_box)

				if len(components) == 0:
					conf_avg = 0

				segment = {
					"start": timestamp,
					"end": timestamp,
					"boxes": boxes,
					"confidence": conf_avg / 100,
				}

				progress = frame_number / total_frames
				self.progress = progress
				segments.append(segment)

			cap.release()
		return segments

	def merge_segments(self, segments: list[dict[str, Any]]) -> list[dict[str, Any]]:
		if not segments:
			return []

		merged_segments = []
		current_segment = segments[0]

		for next_segment in segments[1:]:
			if current_segment["boxes"] == next_segment["boxes"]:
				current_segment["end"] = next_segment["end"]
			else:
				merged_segments.append(current_segment)
				current_segment = next_segment

		merged_segments.append(current_segment)
		return merged_segments

	def on_run(self) -> None:
		# help(vt)
		project = vt.current_project()
		if project is None:
			return
		project.tags.clear()
		video = project.import_video(self.vid_path)

		if video is None:
			self.progress_info = "Import failed"
			print(f"Failed to import {self.vid_path} file", file=sys.stderr)
			return

		group = vt.VideoGroup(self.group_name)
		group.add_video(video, 0)
		project.add_group(group)

		tag = vt.Tag(f"Text", vt.to_abgr(0xDEA584FF))
		text_attribute_name = "text"
		box_attribute_name = "box"
		conf_attribute_name = "confidence"
		tag.add_attribute(text_attribute_name, vt.TagAttributeType.string)
		tag.add_attribute(box_attribute_name, vt.TagAttributeType.shape)
		tag.add_attribute(conf_attribute_name, vt.TagAttributeType.float)
		project.tags.add_tag(tag)

		self.progress_info = "Performing OCR"
		segments = self.merge_segments(self.get_video_text(video.path))

		for segment in segments:
			if len(segment["boxes"]) == 0:
				continue

			start = int(segment["start"])
			end = int(segment["end"])
			vt_segment = group.add_segment(tag, start, end)
			if vt_segment is not None:
				for text, boxes in segment["boxes"].items():
					# vt_segment.attributes[video.id][attribute_name].set_string(
					# 	str(segment["text"])
					# )
					vt_segment.get_attribute(video, text_attribute_name).set_string(
						text
					)

					# TODO: Add multiple rects when shapes start holding multiple regions
					x, y, w, h = boxes[0]
					vt_segment.get_attribute(video, box_attribute_name).set_rect(
						x, y, w, h
					)
					vt_segment.get_attribute(video, conf_attribute_name).set_float(
						segment["confidence"]
					)

		self.progress = 1.0
		self.progress_info = "Done!"
