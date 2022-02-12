#include "linden_common.h"
#include "llerrorcontrol.h"

#include "../lscript_rt_interface.h"

int main(int argc, char **argv) {
	if (argc != 2)
		return 1;
	LLError::initForApplication(".", ".", true /* log to stderr */);
	if(!lscript_compile(argv[1], "/tmp/whatever.lso2", "/dev/stderr", 0, "foo", 0))
		return 1;
	lscript_run("/tmp/whatever.lso2", FALSE);
	return 0;
}

