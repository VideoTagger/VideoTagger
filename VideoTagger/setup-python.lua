-- Original code https://github.com/DennyLindberg/basic-pybind11-cpp-module/blob/master/premake5.lua

function sys_invoke(command)
	local success, handle = pcall(io.popen, command)
	if not success then 
		return ""
	end

	result = handle:read("*a")
	handle:close()
	result = string.gsub(result, "\n$", "") -- remove trailing whitespace
	return result
end

function python_find_path()
	local path = sys_invoke('python -c "import sys; import os; print(os.path.dirname(sys.executable))"')
	
	path = string.gsub(path, "\\\\", "\\")
	path = string.gsub(path, "\\", "/")
	return path
end

function python_get_lib_name()
    return sys_invoke("python -c \"import sys; import os; import glob; path = os.path.dirname(sys.executable); libs = glob.glob(path + '/libs/python*'); print(os.path.splitext(os.path.basename(libs[-1]))[0]);\"")
end

PythonPath = python_find_path()
PythonIncludePath = PythonPath .. "/include"
PythonLibPath = PythonPath .. "/libs"
PythonLibName = python_get_lib_name()

if PythonPath == "" or PythonLibName == "" then
    error("Failed to find Python path")
else
    print("Python include path: " .. PythonIncludePath)
    print("Python lib path: " .. PythonLibPath)
    print("Python lib name: " .. PythonLibName)
end