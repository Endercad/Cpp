# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/cyh/Cpp/basic/Coroutine

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/cyh/Cpp/basic/Coroutine/build

# Include any dependencies generated for this target.
include CMakeFiles/task_example.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/task_example.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/task_example.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/task_example.dir/flags.make

CMakeFiles/task_example.dir/examples/03_task.cpp.o: CMakeFiles/task_example.dir/flags.make
CMakeFiles/task_example.dir/examples/03_task.cpp.o: ../examples/03_task.cpp
CMakeFiles/task_example.dir/examples/03_task.cpp.o: CMakeFiles/task_example.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/cyh/Cpp/basic/Coroutine/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/task_example.dir/examples/03_task.cpp.o"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/task_example.dir/examples/03_task.cpp.o -MF CMakeFiles/task_example.dir/examples/03_task.cpp.o.d -o CMakeFiles/task_example.dir/examples/03_task.cpp.o -c /home/cyh/Cpp/basic/Coroutine/examples/03_task.cpp

CMakeFiles/task_example.dir/examples/03_task.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/task_example.dir/examples/03_task.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/cyh/Cpp/basic/Coroutine/examples/03_task.cpp > CMakeFiles/task_example.dir/examples/03_task.cpp.i

CMakeFiles/task_example.dir/examples/03_task.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/task_example.dir/examples/03_task.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/cyh/Cpp/basic/Coroutine/examples/03_task.cpp -o CMakeFiles/task_example.dir/examples/03_task.cpp.s

# Object files for target task_example
task_example_OBJECTS = \
"CMakeFiles/task_example.dir/examples/03_task.cpp.o"

# External object files for target task_example
task_example_EXTERNAL_OBJECTS =

task_example: CMakeFiles/task_example.dir/examples/03_task.cpp.o
task_example: CMakeFiles/task_example.dir/build.make
task_example: CMakeFiles/task_example.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/cyh/Cpp/basic/Coroutine/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable task_example"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/task_example.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/task_example.dir/build: task_example
.PHONY : CMakeFiles/task_example.dir/build

CMakeFiles/task_example.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/task_example.dir/cmake_clean.cmake
.PHONY : CMakeFiles/task_example.dir/clean

CMakeFiles/task_example.dir/depend:
	cd /home/cyh/Cpp/basic/Coroutine/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/cyh/Cpp/basic/Coroutine /home/cyh/Cpp/basic/Coroutine /home/cyh/Cpp/basic/Coroutine/build /home/cyh/Cpp/basic/Coroutine/build /home/cyh/Cpp/basic/Coroutine/build/CMakeFiles/task_example.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/task_example.dir/depend

