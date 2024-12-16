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

function python_get_include_path()
	if os.host() == "windows" then
		return python_find_path() .. "/include"
	else
		return sys_invoke('python -c "from sysconfig import get_paths as gp; print(gp()[\'include\'])"')
	end
end

function python_get_lib_name_and_path()
	if os.host() == "windows" then
		local lib_name =  sys_invoke("python -c \"import sys; import os; import glob; path = os.path.dirname(sys.executable); libs = glob.glob(path + '/libs/python*'); print(os.path.splitext(os.path.basename(libs[-1]))[0]);\"")
		return lib_name, python_find_path() .. "/libs"
	else
		local lib_name = sys_invoke('python -c "import sysconfig; print(sysconfig.get_config_var(\'LDLIBRARY\'))"')
		local lib_path = sys_invoke('python -c "from sysconfig import get_config_var as gcv; print(gcv(\'LIBDIR\') or gcv(\'LIBPL\'))"')
		return lib_name, lib_path
	end
end

PythonPath = python_find_path()
PythonIncludePath = python_get_include_path()
PythonLibName, PythonLibPath = python_get_lib_name_and_path()
PythonDllPath = PythonPath .. "/" .. PythonLibName .. ".dll"

if PythonPath == "" then
    error("Failed to find Python path")
else
    print("Python include path: " .. PythonIncludePath)
    print("Python lib path: " .. PythonLibPath)
    print("Python lib name: " .. PythonLibName)
end
