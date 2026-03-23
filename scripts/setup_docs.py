import os
import sys

from lib.docs import (
	DOCS_OUT_DIR,
	create_docs_out_dir,
	create_docs_src_dir,
	get_all_doc_versions,
)
from lib.utils import get_vt_version, eprint, ROOT

INDEX_PATH = os.path.join(ROOT, "docs", "src", "index.html")
README_PATH = os.path.join(ROOT, "README.md")
DOCS_README_PATH = os.path.join(ROOT, "docs", "src", "README.autogen.md")
VERSION_SELECTOR_PATH = os.path.join(ROOT, "docs", "src", "version_selector.html")


def setup_readme():
	with open(README_PATH, "r") as readme_file:
		if not readme_file:
			eprint("Failed to read README.md")
			return False
		readme_content = readme_file.read()
	with open(DOCS_README_PATH, "w") as docs_readme_file:
		if not docs_readme_file:
			eprint("Failed to create README.autogen.md")
			return False
		docs_readme_file.write(readme_content)
		docs_readme_file.write("\n\n[TOC]\n")
		print(f"Copied README.md to {DOCS_README_PATH}")
	return True


def preprocess_index_html(version: str):
	index_content = ""
	with open(INDEX_PATH, "r") as index_file:
		if not index_file:
			eprint("Failed to read index.html")
			return False
		index_content = index_file.read()
		index_content = index_content.replace("${VT_VERSION}", version)

	if not index_content:
		return False

	if not os.path.exists(DOCS_OUT_DIR):
		os.makedirs(DOCS_OUT_DIR)

	with open(os.path.join(DOCS_OUT_DIR, "index.html"), "w") as index_file:
		index_file.write(index_content)
		print(f"Updated index.html with version {version} in {DOCS_OUT_DIR}")
	return True


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

	if not os.path.exists(DOCS_OUT_DIR):
		os.makedirs(DOCS_OUT_DIR)
	out_path = os.path.join(DOCS_OUT_DIR, "version_selector.html")

	with open(out_path, "w") as selector_file:
		selector_file.write(selector_content)
		print(f"Updated version selector with versions: {versions}")
	return True


def setup_docs(only_version_selector: bool = False):
	print("Setting up documentation...")
	version = get_vt_version()
	print(f"VT_VERSION: {version}")

	if not version:
		eprint("VERSION file doesn't exist or is empty.")
		exit(1)

	create_docs_out_dir(version)
	if not only_version_selector:
		create_docs_src_dir()
		if not setup_readme():
			eprint("Failed to set up README for docs.")
			exit(1)

		if not preprocess_index_html(version):
			eprint("Failed to create index.html.")
			exit(1)

	if not preprocess_version_selector_html(version, get_all_doc_versions()):
		eprint("Failed to create version selector HTML.")
		exit(1)
	print("Done!")


if __name__ == "__main__":
	if sys.argv and len(sys.argv) > 1 and sys.argv[1] == "--only-version-selector":
		setup_docs(True)
	else:
		setup_docs()
