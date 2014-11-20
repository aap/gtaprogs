Each directory contains a Makefile, first alter them to reflect your
system configuration. Mainly change the variable RWDIR to the directory
of the rwtools directory.

Then run `make` in `engine` and `misctest`.
On Windows use MinGW/MSYS and run `make -f Makefile.win32`.

Misctest expects a config file as its first argument (see the `conf` directory)
and to be run from its directory (otherwise it won't find the shaders).

Pedtest has no Windows Makefile yet and the paths are still hardcoded, sorry.

Subdirectories:
- engine:
	contains reusable parts
- pedtest:
	test for peds and animation
- misctest:
	test for misc stuff
- shaders:
	shaders for the programs

Dependencies:
- rwtools
- external: glew, glfw3, jsoncpp
