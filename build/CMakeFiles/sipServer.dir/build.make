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
CMAKE_SOURCE_DIR = /home/wjc/project/sipserver

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/wjc/project/sipserver/build

# Include any dependencies generated for this target.
include CMakeFiles/sipServer.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/sipServer.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/sipServer.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/sipServer.dir/flags.make

CMakeFiles/sipServer.dir/utils/MD5.c.o: CMakeFiles/sipServer.dir/flags.make
CMakeFiles/sipServer.dir/utils/MD5.c.o: ../utils/MD5.c
CMakeFiles/sipServer.dir/utils/MD5.c.o: CMakeFiles/sipServer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/wjc/project/sipserver/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/sipServer.dir/utils/MD5.c.o"
	/usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/sipServer.dir/utils/MD5.c.o -MF CMakeFiles/sipServer.dir/utils/MD5.c.o.d -o CMakeFiles/sipServer.dir/utils/MD5.c.o -c /home/wjc/project/sipserver/utils/MD5.c

CMakeFiles/sipServer.dir/utils/MD5.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/sipServer.dir/utils/MD5.c.i"
	/usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/wjc/project/sipserver/utils/MD5.c > CMakeFiles/sipServer.dir/utils/MD5.c.i

CMakeFiles/sipServer.dir/utils/MD5.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/sipServer.dir/utils/MD5.c.s"
	/usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/wjc/project/sipserver/utils/MD5.c -o CMakeFiles/sipServer.dir/utils/MD5.c.s

CMakeFiles/sipServer.dir/utils/HTTPDigest.c.o: CMakeFiles/sipServer.dir/flags.make
CMakeFiles/sipServer.dir/utils/HTTPDigest.c.o: ../utils/HTTPDigest.c
CMakeFiles/sipServer.dir/utils/HTTPDigest.c.o: CMakeFiles/sipServer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/wjc/project/sipserver/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/sipServer.dir/utils/HTTPDigest.c.o"
	/usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/sipServer.dir/utils/HTTPDigest.c.o -MF CMakeFiles/sipServer.dir/utils/HTTPDigest.c.o.d -o CMakeFiles/sipServer.dir/utils/HTTPDigest.c.o -c /home/wjc/project/sipserver/utils/HTTPDigest.c

CMakeFiles/sipServer.dir/utils/HTTPDigest.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/sipServer.dir/utils/HTTPDigest.c.i"
	/usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/wjc/project/sipserver/utils/HTTPDigest.c > CMakeFiles/sipServer.dir/utils/HTTPDigest.c.i

CMakeFiles/sipServer.dir/utils/HTTPDigest.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/sipServer.dir/utils/HTTPDigest.c.s"
	/usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/wjc/project/sipserver/utils/HTTPDigest.c -o CMakeFiles/sipServer.dir/utils/HTTPDigest.c.s

CMakeFiles/sipServer.dir/utils/func.cpp.o: CMakeFiles/sipServer.dir/flags.make
CMakeFiles/sipServer.dir/utils/func.cpp.o: ../utils/func.cpp
CMakeFiles/sipServer.dir/utils/func.cpp.o: CMakeFiles/sipServer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/wjc/project/sipserver/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/sipServer.dir/utils/func.cpp.o"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/sipServer.dir/utils/func.cpp.o -MF CMakeFiles/sipServer.dir/utils/func.cpp.o.d -o CMakeFiles/sipServer.dir/utils/func.cpp.o -c /home/wjc/project/sipserver/utils/func.cpp

CMakeFiles/sipServer.dir/utils/func.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sipServer.dir/utils/func.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/wjc/project/sipserver/utils/func.cpp > CMakeFiles/sipServer.dir/utils/func.cpp.i

CMakeFiles/sipServer.dir/utils/func.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sipServer.dir/utils/func.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/wjc/project/sipserver/utils/func.cpp -o CMakeFiles/sipServer.dir/utils/func.cpp.s

CMakeFiles/sipServer.dir/utils/mylog.cpp.o: CMakeFiles/sipServer.dir/flags.make
CMakeFiles/sipServer.dir/utils/mylog.cpp.o: ../utils/mylog.cpp
CMakeFiles/sipServer.dir/utils/mylog.cpp.o: CMakeFiles/sipServer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/wjc/project/sipserver/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/sipServer.dir/utils/mylog.cpp.o"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/sipServer.dir/utils/mylog.cpp.o -MF CMakeFiles/sipServer.dir/utils/mylog.cpp.o.d -o CMakeFiles/sipServer.dir/utils/mylog.cpp.o -c /home/wjc/project/sipserver/utils/mylog.cpp

CMakeFiles/sipServer.dir/utils/mylog.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sipServer.dir/utils/mylog.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/wjc/project/sipserver/utils/mylog.cpp > CMakeFiles/sipServer.dir/utils/mylog.cpp.i

CMakeFiles/sipServer.dir/utils/mylog.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sipServer.dir/utils/mylog.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/wjc/project/sipserver/utils/mylog.cpp -o CMakeFiles/sipServer.dir/utils/mylog.cpp.s

CMakeFiles/sipServer.dir/utils/myconfig.cpp.o: CMakeFiles/sipServer.dir/flags.make
CMakeFiles/sipServer.dir/utils/myconfig.cpp.o: ../utils/myconfig.cpp
CMakeFiles/sipServer.dir/utils/myconfig.cpp.o: CMakeFiles/sipServer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/wjc/project/sipserver/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/sipServer.dir/utils/myconfig.cpp.o"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/sipServer.dir/utils/myconfig.cpp.o -MF CMakeFiles/sipServer.dir/utils/myconfig.cpp.o.d -o CMakeFiles/sipServer.dir/utils/myconfig.cpp.o -c /home/wjc/project/sipserver/utils/myconfig.cpp

CMakeFiles/sipServer.dir/utils/myconfig.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sipServer.dir/utils/myconfig.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/wjc/project/sipserver/utils/myconfig.cpp > CMakeFiles/sipServer.dir/utils/myconfig.cpp.i

CMakeFiles/sipServer.dir/utils/myconfig.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sipServer.dir/utils/myconfig.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/wjc/project/sipserver/utils/myconfig.cpp -o CMakeFiles/sipServer.dir/utils/myconfig.cpp.s

CMakeFiles/sipServer.dir/SipServer.cpp.o: CMakeFiles/sipServer.dir/flags.make
CMakeFiles/sipServer.dir/SipServer.cpp.o: ../SipServer.cpp
CMakeFiles/sipServer.dir/SipServer.cpp.o: CMakeFiles/sipServer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/wjc/project/sipserver/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/sipServer.dir/SipServer.cpp.o"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/sipServer.dir/SipServer.cpp.o -MF CMakeFiles/sipServer.dir/SipServer.cpp.o.d -o CMakeFiles/sipServer.dir/SipServer.cpp.o -c /home/wjc/project/sipserver/SipServer.cpp

CMakeFiles/sipServer.dir/SipServer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sipServer.dir/SipServer.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/wjc/project/sipserver/SipServer.cpp > CMakeFiles/sipServer.dir/SipServer.cpp.i

CMakeFiles/sipServer.dir/SipServer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sipServer.dir/SipServer.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/wjc/project/sipserver/SipServer.cpp -o CMakeFiles/sipServer.dir/SipServer.cpp.s

CMakeFiles/sipServer.dir/main.cpp.o: CMakeFiles/sipServer.dir/flags.make
CMakeFiles/sipServer.dir/main.cpp.o: ../main.cpp
CMakeFiles/sipServer.dir/main.cpp.o: CMakeFiles/sipServer.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/wjc/project/sipserver/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/sipServer.dir/main.cpp.o"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/sipServer.dir/main.cpp.o -MF CMakeFiles/sipServer.dir/main.cpp.o.d -o CMakeFiles/sipServer.dir/main.cpp.o -c /home/wjc/project/sipserver/main.cpp

CMakeFiles/sipServer.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sipServer.dir/main.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/wjc/project/sipserver/main.cpp > CMakeFiles/sipServer.dir/main.cpp.i

CMakeFiles/sipServer.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sipServer.dir/main.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/wjc/project/sipserver/main.cpp -o CMakeFiles/sipServer.dir/main.cpp.s

# Object files for target sipServer
sipServer_OBJECTS = \
"CMakeFiles/sipServer.dir/utils/MD5.c.o" \
"CMakeFiles/sipServer.dir/utils/HTTPDigest.c.o" \
"CMakeFiles/sipServer.dir/utils/func.cpp.o" \
"CMakeFiles/sipServer.dir/utils/mylog.cpp.o" \
"CMakeFiles/sipServer.dir/utils/myconfig.cpp.o" \
"CMakeFiles/sipServer.dir/SipServer.cpp.o" \
"CMakeFiles/sipServer.dir/main.cpp.o"

# External object files for target sipServer
sipServer_EXTERNAL_OBJECTS =

sipServer: CMakeFiles/sipServer.dir/utils/MD5.c.o
sipServer: CMakeFiles/sipServer.dir/utils/HTTPDigest.c.o
sipServer: CMakeFiles/sipServer.dir/utils/func.cpp.o
sipServer: CMakeFiles/sipServer.dir/utils/mylog.cpp.o
sipServer: CMakeFiles/sipServer.dir/utils/myconfig.cpp.o
sipServer: CMakeFiles/sipServer.dir/SipServer.cpp.o
sipServer: CMakeFiles/sipServer.dir/main.cpp.o
sipServer: CMakeFiles/sipServer.dir/build.make
sipServer: CMakeFiles/sipServer.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/wjc/project/sipserver/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Linking CXX executable sipServer"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/sipServer.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/sipServer.dir/build: sipServer
.PHONY : CMakeFiles/sipServer.dir/build

CMakeFiles/sipServer.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/sipServer.dir/cmake_clean.cmake
.PHONY : CMakeFiles/sipServer.dir/clean

CMakeFiles/sipServer.dir/depend:
	cd /home/wjc/project/sipserver/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/wjc/project/sipserver /home/wjc/project/sipserver /home/wjc/project/sipserver/build /home/wjc/project/sipserver/build /home/wjc/project/sipserver/build/CMakeFiles/sipServer.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/sipServer.dir/depend
