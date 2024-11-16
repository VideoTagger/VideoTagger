import sys


def utf16_to_utf8(code_point):
	code_point_int = int(code_point, 16)
	utf16_char = chr(code_point_int)
	utf8_bytes = utf16_char.encode("utf-8")
	return "".join(f"\\x{byte:02x}" for byte in utf8_bytes)


if __name__ == "__main__":
	if len(sys.argv) == 1:
		print(
			f"Error: Not enough arguments\nUsage: {sys.argv[0]} [<icon_name>:<codepoint> | <codepoint>]...",
			file=sys.stderr,
		)
		exit(1)
	for i in range(1, len(sys.argv)):
		icon_info = sys.argv[i].split(":")
		if len(icon_info) > 2:
			print(
				"Error: Invalid format, expected <icon_name>:<codepoint> or <codepoint>",
				file=sys.stderr,
			)
		icon_name = icon_info[0] if len(icon_info) == 2 else "icon_name"
		print(f'inline constexpr auto {icon_name} = "{utf16_to_utf8(icon_info[-1])}";')
