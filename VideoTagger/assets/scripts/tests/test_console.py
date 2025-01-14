import sys
from vt import *


class test_console(Script):
	def has_progress(self: Script) -> bool:
		return False

	def on_run(self) -> None:
		project = current_project()
		if project is None:
			return
		print("Testing stdout redirection")
		print("Testing stderr redirection", file=sys.stderr)

		for _ in range(5):
			log("Test info")
			error("Test error")
			warn("Test multiline\nSecond Line\n\n4th Line")
