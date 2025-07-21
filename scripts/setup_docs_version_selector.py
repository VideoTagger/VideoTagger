import os
import sys

from docs import *

INDEX_PATH = os.path.join(ROOT, "docs", "src", "index.html")
VERSION_SELECTOR_PATH = os.path.join(ROOT, "docs", "src", "version_selector.html")


def preprocess_version_selector_html(vt_version: str, versions: list):
	if not versions:
		return False

	selector_content = ""
	with open(VERSION_SELECTOR_PATH, "r") as selector_file:
		selector_content = selector_file.read()
	if not selector_content:
		return False

	options_str = ""
	for i, version in enumerate(versions):
		if version == vt_version:
			options_str += f'\t<option value="{version}" selected>{version}</option>'
		else:
			options_str += f'\t<option value="{version}">{version}</option>'
		if i < len(versions) - 1:
			options_str += "\n"

	selector_content = selector_content.replace(
		"${VERSION_SELECTOR_OPTIONS}", options_str
	)

	if not os.path.exists(DOCS_PATH):
		os.makedirs(DOCS_PATH)
	out_path = os.path.join(DOCS_PATH, "version_selector.html")

	with open(out_path, "w") as selector_file:
		selector_file.write(selector_content)
		print(f"Updated version selector with versions: {versions}")
	return True


if __name__ == "__main__":
	version = get_vt_version()
	print(f"VT_VERSION: {version}")

	if not version:
		print("VERSION file doesn't exist or is empty.")
		exit(1)

	create_docs_dir(version)

	if not preprocess_version_selector_html(version, get_all_doc_versions()):
		print("Failed to create version selector HTML.")
		exit(1)
