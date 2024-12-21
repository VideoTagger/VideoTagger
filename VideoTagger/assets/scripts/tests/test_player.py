import vt


class test_player(vt.Script):
	def has_progress(self: vt.Script) -> bool:
		return False

	def on_run(self) -> None:
		if vt.player.is_playing:
			vt.player.pause()
			vt.player.seek(vt.Timestamp(0))
