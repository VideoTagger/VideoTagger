import argparse
import os
import subprocess

import setup_tools
import setup_docs

from lib.utils import (
	get_config_var,
	get_exec_extension,
	get_platform,
	get_vt_version,
	tool_path,
	eprint,
	ROOT,
)

RELEASE_DIR = os.path.join(ROOT, "build", "release")
PACK_ID = "VideoTagger"
ICON_PATH = os.path.join(ROOT, "VideoTagger", "src", "resources", "icon_max.ico")
BUILD_DIR = os.path.join(ROOT, "build")
DEFAULT_ARCH = "x86_64"
DEFAULT_BRANCH = "dev"
DEFAULT_BUILD_CONFIG = "Shipping"
DEFAULT_VPK_VERSION = "0.0.1298"


def normalize_path(path: str):
	return path.replace("\\", "/")


def build_path(platform: str, arch: str, config: str):
	return normalize_path(
		os.path.abspath(
			os.path.join(
				BUILD_DIR,
				"cmake",
				"install",
				f"{platform.capitalize()}-{arch.lower()}",
				config.capitalize(),
			)
		)
	)


def build_channel(platform: str, arch: str, branch: str):
	new_arch = "x64" if arch == "x86_64" else arch
	return f"{platform.lower()}-{new_arch.lower()}-{branch.lower()}"


def parse_args():
	parser = argparse.ArgumentParser(description="Package VideoTagger with Velopack")
	parser.add_argument("--arch", default=DEFAULT_ARCH, help="Target architecture")
	parser.add_argument(
		"--branch", default=DEFAULT_BRANCH, help="Release branch/channel suffix"
	)
	parser.add_argument(
		"--build-config",
		default=DEFAULT_BUILD_CONFIG,
		help="Build configuration to package",
	)
	parser.add_argument(
		"--vpk-version", default=DEFAULT_VPK_VERSION, help="Version of the vpk tool"
	)
	return parser.parse_args()


def main():
	cli_args = parse_args()
	platform = get_platform()
	if not platform:
		eprint("Error: Unsupported platform.")
		exit(1)

	vt_version = get_vt_version()
	if not vt_version:
		eprint("Error: version in config.json is empty or not found.")
		exit(1)
	channel = build_channel(platform, cli_args.arch, cli_args.branch)
	print(f"Using channel: {channel}")

	os.environ["VT_VERSION"] = vt_version

	setup_tools.setup_tools()
	setup_docs.setup_docs()

	dnx_path = tool_path("dnx")

	exe_name = f"VideoTagger-{cli_args.build_config}" + get_exec_extension()

	package_dir = build_path(platform, cli_args.arch, cli_args.build_config)
	print(f"Packaging from directory: {package_dir}")
	if not os.path.isdir(package_dir):
		eprint(
			f"Error: Package directory {package_dir} does not exist. Please build the project first."
		)
		exit(1)

	vpk_args = [
		dnx_path,
		"vpk",
		"-y",
		"--version",
		cli_args.vpk_version,
		"--",
		"pack",
		"-o",
		normalize_path(RELEASE_DIR),
		"--packId",
		PACK_ID,
		"-v",
		vt_version,
		"-c",
		channel,
		"-p",
		normalize_path(package_dir),
	]

	# if platform == "windows":
	# 	args.extend(["--msiDeploymentTool"])

	vpk_args.extend(["-e", exe_name])

	if os.path.exists(ICON_PATH):
		vpk_args.extend(["-i", normalize_path(ICON_PATH)])

	name = get_config_var("name")

	command = " ".join(vpk_args)
	print(f"Running command: {command}")

	result = subprocess.run(vpk_args, cwd=normalize_path(ROOT), env=os.environ)
	if result.returncode != 0:
		exit(result.returncode)


if __name__ == "__main__":
	main()
