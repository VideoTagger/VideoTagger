import videotagger as vt
import os
import sys
import pybind11_stubgen


class generate_stubs(vt.Script):
	def has_progress(self: vt.Script) -> bool:
		return False

	def on_run(self: vt.Script) -> None:
		out_dir = os.path.abspath("./stubs")
		# TODO: Change output directory to something better
		sys.argv = ["pybind11-stubgen", "videotagger", "-o", out_dir]
		print(f"Output directory: {out_dir}")
		pybind11_stubgen.main()

		vt.fs.open_in_explorer(out_dir)
