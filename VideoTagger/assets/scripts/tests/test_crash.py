import vt

This test may crash the whole application, if the exception is not handled properly.

class test_crash(vt.Script):
	def has_progress(self: vt.Script) -> bool:
		return False

	def on_run(self) -> None:
		pass
