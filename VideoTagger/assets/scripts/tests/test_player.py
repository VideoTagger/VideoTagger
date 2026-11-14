import videotagger as vt

class test_player(vt.Script):
	def has_progress(self: vt.Script) -> bool:
		return False

	def on_run(self) -> None:
		seek_pos = vt.Timestamp(0)
		vt.log(f"Player is {'' if vt.windows.player.is_playing else 'not '}playing")
		if vt.windows.player.is_playing:
			vt.log(f"Current timestamp: {vt.windows.player.current_timestamp}")
			vt.log("Pausing...")
			vt.windows.player.pause()
			vt.log(f"Seeking to: {seek_pos}")
			vt.windows.player.seek(seek_pos)
		else:
			vt.log("Unpausing...")
			vt.windows.player.play()
