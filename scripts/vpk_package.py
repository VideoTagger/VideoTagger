import os
import subprocess

import setup_tools
import setup_docs

from lib.utils import (
	get_config_var,
	get_exec_extension,
	get_platform,
	get_vt_version,
	tool_exists,
	eprint,
	ROOT,
)

RELEASE_DIR = os.path.join(ROOT, "build", "release")
PACK_ID = "VideoTagger"
ICON_PATH = os.path.join(ROOT, "VideoTagger", "src", "resources", "icon_max.ico")
BUILD_CONFIG = "Debug"
BUILD_DIR = os.path.join(ROOT, "build")
ARCH = "x86_64"
BRANCH = "dev"
VPK_VERSION = "0.0.1298"


def build_path(platform: str, arch: str, config: str):
	return os.path.abspath(
		os.path.join(
			BUILD_DIR,
			"cmake",
			"install",
			f"{platform.capitalize()}-{arch.lower()}",
			config.capitalize(),
		)
	)


def build_channel(platform: str, arch: str, branch: str):
	new_arch = "x64" if arch == "x86_64" else arch
	return f"{platform.lower()}-{new_arch.lower()}-{branch.lower()}"


def main():
	platform = get_platform()
	if not platform:
		eprint("Error: Unsupported platform.")
		exit(1)

	vt_version = get_vt_version()
	if not vt_version:
		eprint("Error: version in config.json is empty or not found.")
		exit(1)
	channel = build_channel(platform, ARCH, BRANCH)
	print(f"Using channel: {channel}")

	os.environ["VT_VERSION"] = vt_version

	setup_tools.setup_tools()
	setup_docs.setup_docs()

	if not tool_exists("vpk"):
		eprint(
			"Error: vpk tool not found. Please ensure it is installed and in the system PATH."
		)
		exit(1)

	exe_name = f"VideoTagger-{BUILD_CONFIG}" + get_exec_extension()

	package_dir = build_path(platform, ARCH, BUILD_CONFIG)
	print(f"Packaging from directory: {package_dir}")
	if not os.path.isdir(package_dir):
		eprint(
			f"Error: Package directory {package_dir} does not exist. Please build the project first."
		)
		exit(1)

	args = [
		"dnx",
		"vpk",
		"--version",
		VPK_VERSION,
		"--",
		"pack",
		"-o",
		RELEASE_DIR,
		"--packId",
		PACK_ID,
		"-v",
		vt_version,
		"-c",
		channel,
		"-p",
		f'"{package_dir}"',
	]

	# if platform == "windows":
	# 	args.extend(["--msiDeploymentTool"])

	args.extend(["-e", exe_name])

	if os.path.exists(ICON_PATH):
		args.extend(["-i", f'"{ICON_PATH}"'])

	name = get_config_var("name")

	command = " ".join(args)
	print(f"Running command: {command}")

	result = subprocess.run(args, cwd=ROOT, env=os.environ)
	if result.returncode != 0:
		exit(result.returncode)


if __name__ == "__main__":
	main()
