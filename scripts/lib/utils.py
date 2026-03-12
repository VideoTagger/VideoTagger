import os
import sys
import json

WORK_DIR = os.path.dirname(os.path.realpath(sys.argv[0]))
ROOT = os.path.dirname(WORK_DIR)
CONFIG_PATH = os.path.join(ROOT, "config.json")


def get_version():
	if os.path.exists(CONFIG_PATH):
		with open(CONFIG_PATH, "r") as config_file:
			config_data = json.load(config_file)
			return config_data.get("version", "")
	return None


def get_vt_version():
	vt_version = os.getenv("VT_VERSION")
	if not vt_version:
		vt_version = get_version()
	return vt_version


def eprint(*args, **kwargs):
	print(*args, file=sys.stderr, **kwargs)
