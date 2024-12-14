from sys import stderr
import vt
import time


class test_console(vt.Script):
	def has_progress(self: vt.Script) -> bool:
		return False

	def on_run(self: vt.Script) -> None:
		project = vt.current_project()
		if project is None:
			return

		for _ in range(5):
			vt.log("Test infos")
			vt.error("Test error")
			vt.warn("Test multiline\nSecond Line\n\n4th Line")
