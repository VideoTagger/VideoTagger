import os
import sys
import urllib.request
import zipfile, tarfile
from enum import Enum
from sys import platform

class UnpackAction(Enum):
	Nothing = 0,
	UnpackZip = 1,
	UnpackTar = 2


EXECUTABLE_EXT = ".exe" if platform == "win32" else ""
WORK_DIR = os.path.dirname(os.path.realpath(sys.argv[0]))
TOOL_DIR = os.path.join(WORK_DIR, os.pardir, "tools", "bin")

MDBOOK_VER = "0.4.37"
MDBOOK_WIN_URL = f"https://github.com/rust-lang/mdBook/releases/download/v{MDBOOK_VER}/mdbook-v{MDBOOK_VER}-x86_64-pc-windows-msvc.zip"
MDBOOK_LINUX_URL = f"https://github.com/rust-lang/mdBook/releases/download/v{MDBOOK_VER}/mdbook-v{MDBOOK_VER}-x86_64-unknown-linux-musl.tar.gz"
MDBOOK_MAC_URL = f"https://github.com/rust-lang/mdBook/releases/download/v{MDBOOK_VER}/mdbook-v{MDBOOK_VER}-x86_64-apple-darwin.tar.gz"

PREMAKE_VER = "5.0.0-beta2"
PREMAKE_WIN_URL = f"https://github.com/premake/premake-core/releases/download/v{PREMAKE_VER}/premake-{PREMAKE_VER}-windows.zip"
PREMAKE_LINUX_URL = f"https://github.com/premake/premake-core/releases/download/v{PREMAKE_VER}/premake-{PREMAKE_VER}-linux.tar.gz"
PREMAKE_MAC_URL = f"https://github.com/premake/premake-core/releases/download/v{PREMAKE_VER}/premake-{PREMAKE_VER}-macosx.tar.gz"

DOXYGEN_VER = "1.10.0"
DOXYGEN_SAFE_VER = DOXYGEN_VER.replace(".", "_")
DOXYGEN_WIN_URL = f"https://github.com/doxygen/doxygen/releases/download/Release_{DOXYGEN_SAFE_VER}/doxygen-{DOXYGEN_VER}.windows.x64.bin.zip"
DOXYGEN_LINUX_URL = f"https://github.com/doxygen/doxygen/releases/download/Release_{DOXYGEN_SAFE_VER}/doxygen-{DOXYGEN_VER}.linux.bin.tar.gz"
DOXYGEN_MAC_URL = f"https://github.com/doxygen/doxygen/releases/download/Release_{DOXYGEN_SAFE_VER}/doxygen-{DOXYGEN_VER}.dmg"


tools = {
	"mdbook": [
		{
			"url": MDBOOK_WIN_URL,
			"unpack-action": UnpackAction.UnpackZip
		},
		{
			"url": MDBOOK_LINUX_URL,
			"unpack-action": UnpackAction.UnpackTar
		},
		{
			"url": MDBOOK_MAC_URL,
			"unpack-action": UnpackAction.UnpackTar
		}
	],
	"premake5": [
		{
			"url": PREMAKE_WIN_URL,
			"unpack-action": UnpackAction.UnpackZip
		},
		{
			"url": PREMAKE_LINUX_URL,
			"unpack-action": UnpackAction.UnpackTar
		},
		{
			"url": PREMAKE_MAC_URL,
			"unpack-action": UnpackAction.UnpackTar
		}
	],
	"doxygen": [
		{
			"url": DOXYGEN_WIN_URL,
			"unpack-action": UnpackAction.UnpackZip
		},
		{
			"url": DOXYGEN_LINUX_URL,
			"unpack-action": UnpackAction.UnpackTar
		},
		{
			"url": DOXYGEN_MAC_URL,
			"unpack-action": UnpackAction.Nothing
		}
	]
}

def eprint(*args, **kwargs):
	print(*args, file=sys.stderr, **kwargs)

def download_file(url: str, path: str = ""):
	return urllib.request.urlretrieve(url, path)[0]

def zip_extract_all_files(path: str, target_dir: str):
	with zipfile.ZipFile(path, 'r') as zip_ref:
		zip_ref.extractall(target_dir)

def tar_extract_all_files(path: str, target_dir: str):
	tar_ref = tarfile.open(path, 'r')
	tar_ref.extractall(target_dir)

if __name__ == "__main__":
	try:
		if not WORK_DIR or not TOOL_DIR:
			eprint("Invalid script directories")
			exit(1)

		os.makedirs(TOOL_DIR, exist_ok=True)

		for tool_name in tools:
			print(f"Verifying if {tool_name} exists...")
			tool_path = os.path.join(TOOL_DIR, f"{tool_name}{EXECUTABLE_EXT}")
			if not os.path.exists(tool_path):
				tool = tools[tool_name]

				if platform == "win32":
					os_tool = tool[0]
				elif platform == "linux" or platform == "linux2":
					os_tool = tool[1]
				elif platform == "darwin":
					os_tool = tool[2]

				if os_tool is None:
					eprint("Tool not available for current platform")
					continue

				try:
					print(f"Downloading {tool_name}...")
					tmp_file = download_file(os_tool["url"])
					if os_tool["unpack-action"] == UnpackAction.UnpackZip:
						zip_extract_all_files(tmp_file, TOOL_DIR)
					elif os_tool["unpack-action"] == UnpackAction.UnpackTar:
						tar_extract_all_files(tmp_file, TOOL_DIR)
					print(f"Extracting files...")
				except Exception as ex:
					eprint(f"An error occurred with {tool_name}: {ex}")
					continue

	except Exception as ex:
		eprint(f"An error occurred: {ex}")
		exit(1)

