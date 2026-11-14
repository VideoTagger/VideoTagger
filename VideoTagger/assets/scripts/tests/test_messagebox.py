import videotagger as vt

class test_messagebox(vt.Script):
	def has_progress(self: vt.Script) -> bool:
		return False

	async def on_run(self: vt.Script) -> None:
		await vt.sys.messagebox.show("Test Title", "Test Message", vt.sys.messagebox.MessageBoxIcon.Info, lambda: print("Callback executed"))
