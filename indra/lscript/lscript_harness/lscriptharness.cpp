#include "linden_common.h"
#include "llerrorcontrol.h"
#include "lluuid.h"

#include "../lscript_rt_interface.h"

int main(int argc, char **argv) {
	if (argc != 2)
		return 1;
	LLError::initForApplication(".", ".", true /* log to stderr */);
	LLUUID script_id;
	script_id.generate();
	if(!lscript_compile(argv[1], "/tmp/whatever.lso2", "/dev/stderr", 0, script_id.asString().c_str(), 0))
		return 1;
	lscript_run("/tmp/whatever.lso2", FALSE);
	return 0;
}

