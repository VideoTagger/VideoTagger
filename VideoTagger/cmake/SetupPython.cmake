if (WIN32)
    set(_PYTHON_EXECUTABLE python)
else()
    set(_PYTHON_EXECUTABLE python3)
endif()

function(run_python_command OUTPUT_VAR PY_COMMAND)
    execute_process(
        COMMAND ${_PYTHON_EXECUTABLE} -c "${PY_COMMAND}"
        RESULT_VARIABLE _res
        OUTPUT_VARIABLE _out
        ERROR_VARIABLE _err
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT _res EQUAL 0)
        message(FATAL_ERROR "Failed to run Python command: ${PY_COMMAND}\nError: ${_err}")
    endif()
    set(${OUTPUT_VAR} "${_out}" PARENT_SCOPE)
endfunction()

run_python_command(PythonPath "import sys, os; print(os.path.dirname(sys.executable))")
file(TO_CMAKE_PATH "${PythonPath}" PythonPath)

if (WIN32)
    set(PythonIncludePath "${PythonPath}/include")
else()
    run_python_command(PythonIncludePath "from sysconfig import get_paths as gp; print(gp()['include'])")
endif()

if (WIN32)
    run_python_command(PythonLibName "import sys, os, glob; path = os.path.dirname(sys.executable); libs = glob.glob(path + '/libs/python*'); import os.path; print(os.path.splitext(os.path.basename(libs[-1]))[0])")
    set(PythonLibPath "${PythonPath}/libs")
else()
    run_python_command(PythonLibName "import sysconfig; print(sysconfig.get_config_var('LDLIBRARY'))")
    run_python_command(PythonLibPath "from sysconfig import get_config_var as gcv; print(gcv('LIBDIR') or gcv('LIBPL'))")
endif()

message(STATUS "Python Path        = ${PythonPath}")
message(STATUS "Python Include     = ${PythonIncludePath}")
message(STATUS "Python Lib Name    = ${PythonLibName}")
message(STATUS "Python Lib Path    = ${PythonLibPath}")
