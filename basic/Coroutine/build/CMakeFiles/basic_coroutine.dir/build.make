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
include CMakeFiles/basic_coroutine.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/basic_coroutine.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/basic_coroutine.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/basic_coroutine.dir/flags.make

CMakeFiles/basic_coroutine.dir/examples/01_basic_coroutine.cpp.o: CMakeFiles/basic_coroutine.dir/flags.make
CMakeFiles/basic_coroutine.dir/examples/01_basic_coroutine.cpp.o: ../examples/01_basic_coroutine.cpp
CMakeFiles/basic_coroutine.dir/examples/01_basic_coroutine.cpp.o: CMakeFiles/basic_coroutine.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/cyh/Cpp/basic/Coroutine/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/basic_coroutine.dir/examples/01_basic_coroutine.cpp.o"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/basic_coroutine.dir/examples/01_basic_coroutine.cpp.o -MF CMakeFiles/basic_coroutine.dir/examples/01_basic_coroutine.cpp.o.d -o CMakeFiles/basic_coroutine.dir/examples/01_basic_coroutine.cpp.o -c /home/cyh/Cpp/basic/Coroutine/examples/01_basic_coroutine.cpp

CMakeFiles/basic_coroutine.dir/examples/01_basic_coroutine.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/basic_coroutine.dir/examples/01_basic_coroutine.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/cyh/Cpp/basic/Coroutine/examples/01_basic_coroutine.cpp > CMakeFiles/basic_coroutine.dir/examples/01_basic_coroutine.cpp.i

CMakeFiles/basic_coroutine.dir/examples/01_basic_coroutine.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/basic_coroutine.dir/examples/01_basic_coroutine.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/cyh/Cpp/basic/Coroutine/examples/01_basic_coroutine.cpp -o CMakeFiles/basic_coroutine.dir/examples/01_basic_coroutine.cpp.s

# Object files for target basic_coroutine
basic_coroutine_OBJECTS = \
"CMakeFiles/basic_coroutine.dir/examples/01_basic_coroutine.cpp.o"

# External object files for target basic_coroutine
basic_coroutine_EXTERNAL_OBJECTS =

basic_coroutine: CMakeFiles/basic_coroutine.dir/examples/01_basic_coroutine.cpp.o
basic_coroutine: CMakeFiles/basic_coroutine.dir/build.make
basic_coroutine: CMakeFiles/basic_coroutine.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/cyh/Cpp/basic/Coroutine/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable basic_coroutine"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/basic_coroutine.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/basic_coroutine.dir/build: basic_coroutine
.PHONY : CMakeFiles/basic_coroutine.dir/build

CMakeFiles/basic_coroutine.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/basic_coroutine.dir/cmake_clean.cmake
.PHONY : CMakeFiles/basic_coroutine.dir/clean

CMakeFiles/basic_coroutine.dir/depend:
	cd /home/cyh/Cpp/basic/Coroutine/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/cyh/Cpp/basic/Coroutine /home/cyh/Cpp/basic/Coroutine /home/cyh/Cpp/basic/Coroutine/build /home/cyh/Cpp/basic/Coroutine/build /home/cyh/Cpp/basic/Coroutine/build/CMakeFiles/basic_coroutine.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/basic_coroutine.dir/depend

