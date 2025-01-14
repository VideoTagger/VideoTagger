from vt import *

This test may crash the whole application, if the exception is not handled properly.

class test_crash(Script):
	def has_progress(self: Script) -> bool:
		return False

	def on_run(self) -> None:
		pass
