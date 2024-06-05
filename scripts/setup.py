import os
import sys
import urllib.request
import zipfile
from sys import platform

EXECUTABLE_EXT = ".exe" if platform == "win32" else ""
WORK_DIR = os.path.dirname(os.path.realpath(sys.argv[0]))
TOOL_DIR = os.path.join(WORK_DIR, os.pardir, "tools", "bin")

MDBOOK_VER = "0.4.37"
MDBOOK_WIN_URL = f"https://github.com/rust-lang/mdBook/releases/download/v{MDBOOK_VER}/mdbook-v{MDBOOK_VER}-x86_64-pc-windows-msvc.zip"
MDBOOK_LINUX_URL = f"https://github.com/rust-lang/mdBook/releases/download/v{MDBOOK_VER}/mdbook-v{MDBOOK_VER}-x86_64-unknown-linux-musl.tar.gz"

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
	"mdbook": [MDBOOK_WIN_URL, MDBOOK_LINUX_URL, ""],
	"premake5": [PREMAKE_WIN_URL, PREMAKE_LINUX_URL, PREMAKE_MAC_URL],
    "doxygen": [DOXYGEN_WIN_URL, DOXYGEN_LINUX_URL, DOXYGEN_MAC_URL]
}

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)
    
def download_file(url: str, path: str = ""):
    return urllib.request.urlretrieve(url, path)[0]

def extract_all_files(path: str, target_dir: str):
    with zipfile.ZipFile(path, 'r') as zip_ref:
    	zip_ref.extractall(target_dir)

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
                tool_urls = tools[tool_name]
                
                if platform == "win32":
                    tool_url = tool_urls[0]
                elif platform == "linux" or platform == "linux2":
                    tool_url = tool_urls[1]
                elif platform == "darwin":
                    tool_url = tool_urls[2]
                
                if not tool_url:
                    eprint("Tool not available for current platform")
                    continue
                
                print(f"Downloading {tool_name}...")
                tmp_zip = download_file(tool_url)
                extract_all_files(tmp_zip, TOOL_DIR)
                print(f"Extracting files...")
    except Exception as ex:
        eprint(f"An error occurred: {ex}")
        exit(1)
