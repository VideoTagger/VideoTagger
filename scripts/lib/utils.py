import os
import sys
import json
import shutil

WORK_DIR = os.path.dirname(os.path.realpath(sys.argv[0]))
ROOT = os.path.dirname(WORK_DIR)
CONFIG_PATH = os.path.join(ROOT, "config.json")


def get_config_var(var_name: str):
	if os.path.exists(CONFIG_PATH):
		with open(CONFIG_PATH, "r") as config_file:
			config_data = json.load(config_file)
			return config_data.get(var_name, "")
	return None


def get_version():
	return get_config_var("version")


def get_vt_version():
	vt_version = os.getenv("VT_VERSION")
	if not vt_version:
		vt_version = get_version()
	return vt_version


def tool_exists(tool_name: str) -> bool:
	tool_path = shutil.which(tool_name)
	return tool_path is not None


def get_platform():
	platform = sys.platform
	if platform.startswith("win"):
		return "windows"
	elif platform.startswith("linux"):
		return "linux"
	elif platform.startswith("darwin"):
		return "macos"
	return None


def get_exec_extension():
	platform = get_platform()
	if platform == "windows":
		return ".exe"
	return ""


def eprint(*args, **kwargs):
	print(*args, file=sys.stderr, **kwargs)
