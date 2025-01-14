from vt import *


class test_player(Script):
	def has_progress(self: Script) -> bool:
		return False

	def on_run(self) -> None:
		seek_pos = Timestamp(0)
		log(f"Player is {'' if player.is_playing else 'not '}playing")
		if player.is_playing:
			log(f"Current timestamp: {player.current_timestamp}")
			log("Pausing...")
			player.pause()
			log(f"Seeking to: {seek_pos}")
			player.seek(seek_pos)
		else:
			log("Unpausing...")
			player.play()
