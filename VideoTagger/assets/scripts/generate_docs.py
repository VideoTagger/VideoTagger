from collections import defaultdict
import os
import sys
import vt
from mypy.stubgen import generate_stubs, Options


class generate_docs(vt.Script):
	def __init__(self):
		vt.Script.__init__(self)
		self.module_name = "vt"

	def has_progress(self: vt.Script) -> bool:
		return False

	def progress_callback(self: vt.Script, progress):
		self.progress_info = progress

	def generate_stub_with_stubgen(self):
		output_dir = "./stubs"
		os.makedirs(output_dir, exist_ok=True)

		options = Options(
			pyversion=(3, 12),  # Target Python version, e.g., 3.8
			no_import=True,  # Don't use `import` when parsing
			inspect=False,  # Use runtime introspection for types
			doc_dir="",  # Directory for docstring parsing (None if not needed)
			search_path=[],  # Extra search paths for modules
			interpreter=sys.executable,  # Path to the Python interpreter
			parse_only=False,  # Only parse files, don't introspect
			ignore_errors=False,  # Ignore errors during stub generation
			include_private=False,  # Include private members
			output_dir=output_dir,  # Directory to output the .pyi files
			modules=[self.module_name],  # List of modules to process
			packages=[],  # List of packages to process
			files=[],  # List of individual files to process
			verbose=True,  # Enable verbose output
			quiet=False,  # Suppress warnings and output if True
			export_less=False,  # Don't restrict exported members
			include_docstrings=True,  # Include docstrings in stubs
		)

		generate_stubs(options)
		print(f"Generated .pyi file in {output_dir}")

	def on_run(self) -> None:
		self.progress_info = "Generating docs"
		self.generate_stub_with_stubgen()
		self.progress_info = "Done!"
