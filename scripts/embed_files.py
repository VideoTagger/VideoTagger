import os
from pathlib import Path
import re
from setup import eprint, zip_extract_all_files, WORK_DIR

SOURCE_DIR = os.path.join(WORK_DIR, "embeds")
EMBED_DIR = os.path.join(WORK_DIR, os.pardir, "VideoTagger", "src", "embeds")

MATERIAL_ICONS_URL = "https://fonts.google.com/download?family=Material+Symbols+Sharp"


def sanitize_path(path: str) -> str:
	return re.sub(r"[^a-zA-Z0-9_*]", "_", path)


def file_bytes(path: str) -> bytes:
	with open(path, "rb") as f:
		return f.read()


def embed_file(source: str, target: str) -> None:
	source_name = Path(source).stem
	source_name_safe = sanitize_path(source_name)

	target_name = Path(target).stem
	target_dir = os.path.dirname(target)
	namespace = "namespace vt::embed"

	with open(os.path.join(target_dir, f"{target_name}.hpp"), mode="w") as f:
		f.write("#pragma once\n")
		for include in ["<cstdint>", "<cstddef>"]:
			f.write(f"#include {include}\n")
		f.write(f"\n{namespace}\n{{\n")
		f.write(f"\textern const size_t {source_name_safe}_size;\n")
		f.write(f"\textern const uint8_t {source_name_safe}[];\n")
		f.write(f"}}\n")

	with open(os.path.join(target_dir, f"{target_name}.cpp"), mode="w") as f:
		for include in [f'"{source_name_safe}.hpp"']:
			f.write(f"#include {include}\n")
		bytes = file_bytes(source)

		f.write(f"\n{namespace}\n{{\n")
		f.write(f"\tconst size_t {source_name_safe}_size = {len(bytes)};\n")
		f.write(f"\textern const uint8_t {source_name_safe}[]\n\t{{\n")

		line_length = 16
		for i in range(0, len(bytes), line_length):
			chunk = bytes[i : i + line_length]
			f.write("\t\t" + ", ".join(f"0x{b:02X}" for b in chunk) + ",\n")

		f.write(f"\n\t}};\n")
		f.write(f"}}\n")


if __name__ == "__main__":
	if not SOURCE_DIR or not EMBED_DIR:
		eprint("Invalid embed directory")
		exit(1)

	for source_file in os.listdir(SOURCE_DIR):
		try:
			source_file_name_safe = os.path.join(
				EMBED_DIR, sanitize_path(Path(source_file).stem)
			)
			print(f"Embedding '{source_file}' into '{source_file_name_safe}'...")
			embed_file(
				os.path.join(SOURCE_DIR, source_file),
				source_file_name_safe,
			)

		except Exception as ex:
			eprint(f"An error occurred with '{source_file}': {ex}")
	print("Done!")
