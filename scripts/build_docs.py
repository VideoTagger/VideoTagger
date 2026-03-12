import os
import subprocess

import setup_tools
import setup_docs

from lib.utils import get_vt_version, eprint, ROOT
from lib.docs import DOCS_DIR

DOXYGEN_PATH = os.path.join(ROOT, "tools", "bin", "doxygen")


def main():
	vt_version = get_vt_version()
	if not vt_version:
		eprint("Error: version in config.json is empty or not found.")
		exit(1)

	os.environ["VT_VERSION"] = vt_version

	setup_tools.setup_tools()
	setup_docs.setup_docs()

	if not os.path.exists(DOXYGEN_PATH):
		eprint(
			f"Error: Doxygen not found at {DOXYGEN_PATH}. Please ensure Doxygen is installed and the path is correct."
		)
		exit(1)

	result = subprocess.run([DOXYGEN_PATH, "Doxyfile"], cwd=DOCS_DIR, env=os.environ)
	if result.returncode != 0:
		exit(result.returncode)


if __name__ == "__main__":
	main()
