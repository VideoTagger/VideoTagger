from collections import defaultdict
import os
from pathlib import Path
import re
import filecmp
import requests
from setup import eprint, WORK_DIR

SOURCE_DIR = os.path.join(WORK_DIR, os.pardir, "VideoTagger", "licenses")
EMBED_DIR = os.path.join(WORK_DIR, os.pardir, "VideoTagger", "src", "embeds")

REPO_API_URL = "https://api.github.com/repos/VideoTagger/VideoTagger"


def get_app_description() -> str:
	response = requests.get(REPO_API_URL)
	response.raise_for_status()
	return response.json()["description"]


def into_str(path: str) -> str:
	with open(path, "r") as f:
		return f.read()


def embed_about(
	target_dir: str,
	target: str,
	licenses: dict[str, str],
) -> None:
	target_name = Path(target).stem
	namespace = "namespace vt::embed"

	target_hpp = os.path.join(target_dir, f"{target_name}.hpp")
	temp_hpp = target_hpp + ".tmp"

	with open(temp_hpp, mode="w") as f:
		f.write("#pragma once\n")
		for include in ["<string>", "<map>"]:
			f.write(f"#include {include}\n")
		f.write(f"\n{namespace}\n{{\n")
		f.write(f"\textern const char* const app_description;\n")
		f.write(
			"\textern const std::map<std::string, std::string> third_party_licenses;\n"
		)
		f.write(f"}}\n")

	if os.path.exists(target_hpp):
		if not filecmp.cmp(target_hpp, temp_hpp):
			os.remove(target_hpp)
			os.rename(temp_hpp, target_hpp)
		else:
			os.remove(temp_hpp)
	else:
		os.rename(temp_hpp, target_hpp)

	target_cpp = os.path.join(target_dir, f"{target_name}.cpp")
	temp_cpp = target_cpp + ".tmp"

	with open(temp_cpp, mode="w") as f:
		for include in [f'"{target_name}.hpp"']:
			f.write(f"#include {include}\n")
		app_description = get_app_description()

		f.write(f"\n{namespace}\n{{\n")
		f.write(f'\tconst char* const app_description = "{app_description}";\n')
		f.write(
			f"\tconst std::map<std::string, std::string> third_party_licenses =\n\t{{\n"
		)
		for license in licenses:
			f.write(f'\t\t{{"{license}", R"({licenses[license].strip()})"}},\n')

		f.write(f"\t}};\n")
		f.write(f"}}\n")

	if os.path.exists(target_cpp):
		if not filecmp.cmp(target_cpp, temp_cpp):
			os.remove(target_cpp)
			os.rename(temp_cpp, target_cpp)
		else:
			os.remove(temp_cpp)
	else:
		os.rename(temp_cpp, target_cpp)


def gen_about() -> None:
	if not SOURCE_DIR or not EMBED_DIR:
		eprint("Invalid license directory")
		exit(1)

	licenses: dict[str, str] = defaultdict(str)
	for license_file in os.listdir(SOURCE_DIR):
		try:
			license = into_str(os.path.join(SOURCE_DIR, license_file))
			licenses[Path(license_file).stem] = license.strip()
			print(f"Reading '{license_file}' license...")

		except Exception as ex:
			eprint(f"An error occurred with '{license_file}': {ex}")

	print("Embedding...")
	embed_about(EMBED_DIR, "about", licenses)
	print("Done!")


if __name__ == "__main__":
	gen_about()
