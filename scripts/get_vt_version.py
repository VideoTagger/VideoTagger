from lib.utils import get_vt_version, eprint

if __name__ == "__main__":
	version = get_vt_version()
	if version:
		print(version, end="")
	else:
		eprint("Failed to get VT_VERSION.")
