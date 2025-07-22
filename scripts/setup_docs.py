import os
import sys

from docs import *

INDEX_PATH = os.path.join(ROOT, "docs", "src", "index.html")


def preprocess_index_html(version: str):
	index_content = ""
	with open(INDEX_PATH, "r") as index_file:
		if not index_file:
			return False
		index_content = index_file.read()
		index_content = index_content.replace("${VT_VERSION}", version)

	if not index_content:
		return False

	if not os.path.exists(DOCS_PATH):
		os.makedirs(DOCS_PATH)

	with open(os.path.join(DOCS_PATH, "index.html"), "w") as index_file:
		index_file.write(index_content)
		print(f"Updated index.html with version {version} in {DOCS_PATH}")
	return True


if __name__ == "__main__":
	version = get_vt_version()
	print(f"VT_VERSION: {version}")

	if not version:
		print("VERSION file doesn't exist or is empty.")
		exit(1)

	create_docs_dir(version)
	if not preprocess_index_html(version):
		print("Failed to create index.html.")
		exit(1)
