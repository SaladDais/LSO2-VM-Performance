#include "linden_common.h"
#include "../lscript_rt_interface.h"

int main(int argc, char **argv) {
	if (argc != 2)
		return 1;
	return !lscript_compile(argv[1], "/dev/stdout", "/dev/stderr", 0, "foo", 0);
}

