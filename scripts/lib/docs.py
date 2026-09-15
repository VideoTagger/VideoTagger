import os
import sys

from lib.utils import ROOT

DOCS_OUT_DIR = os.path.join(ROOT, "build", "web", "public", "docs")
DOCS_DIR = os.path.join(ROOT, "docs")
DOCS_SRC_DIR = os.path.join(ROOT, "docs", "src")


def create_docs_src_dir():
	src_dir = os.path.join(DOCS_SRC_DIR)
	if not os.path.exists(src_dir):
		os.makedirs(src_dir)


def create_docs_out_dir(version: str):
	out_dir = os.path.join(DOCS_OUT_DIR, version)
	if not os.path.exists(out_dir):
		os.makedirs(out_dir)


def get_all_doc_versions():
	versions = []
	if not os.path.exists(DOCS_OUT_DIR):
		return versions
	for item in os.listdir(DOCS_OUT_DIR):
		item_path = os.path.join(DOCS_OUT_DIR, item)
		if os.path.isdir(item_path):
			versions.append(item.strip())
	return versions
