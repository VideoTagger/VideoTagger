import os
import sys

WORK_DIR = os.path.dirname(os.path.realpath(sys.argv[0]))
ROOT = os.path.dirname(WORK_DIR)
DOCS_PATH = os.path.join(ROOT, "build", "docs", "public", "docs")
VERSION_PATH = os.path.join(ROOT, "VERSION")


def get_version():
	if os.path.exists(VERSION_PATH):
		with open(VERSION_PATH, "r") as version_file:
			return version_file.read().strip()


def get_vt_version():
	vt_version = os.getenv("VT_VERSION")
	if not vt_version:
		vt_version = get_version()
	return vt_version


def create_docs_dir(version: str):
	out_dir = os.path.join(DOCS_PATH, version)
	if not os.path.exists(out_dir):
		os.makedirs(out_dir)


def get_all_doc_versions():
	versions = []
	if not os.path.exists(DOCS_PATH):
		return versions
	for item in os.listdir(DOCS_PATH):
		item_path = os.path.join(DOCS_PATH, item)
		if os.path.isdir(item_path):
			versions.append(item.strip())
	return versions
