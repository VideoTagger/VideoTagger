from sys import stderr
import vt
import time

This test shouldn't crash the whole application

class test_crash(vt.Script):
	def has_progress(self: vt.Script) -> bool:
		return False

	def on_run(self: vt.Script) -> None:
		pass
