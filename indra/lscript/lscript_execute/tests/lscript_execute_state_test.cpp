#include "linden_common.h"
#include "llfile.h"
#include "lltimer.h"
#include "lscript_execute.h"
#include "stringize.h"

#include "../../lscript_rt_interface.h"
#include "../../test/lltut.h"
#include <memory>
#include <vector>

namespace tut
{
	struct lscript_execute
	{
		std::string mTestScriptDir;
		std::string mTestScriptFile;
		std::set<std::string> mCleanups;
		lscript_execute()
		{
			LLUUID random;
			random.generate();
			// generate temp dir
			mTestScriptDir = STRINGIZE(LLFile::tmpdir() << "lscript-test-" << random << "/");
			LLFile::mkdir(mTestScriptDir);
		}
		~lscript_execute()
		{
			//Remove test files
			for (auto filename : mCleanups)
			{
				LLFile::remove(filename);
			}
			LLFile::rmdir(mTestScriptDir);
		}

		std::unique_ptr<LLScriptExecuteLSL2> loadScript(const std::string& script_path)
		{
			std::string lso_file = mTestScriptDir + "compiled.lso";
			std::string error_file = mTestScriptDir + "compiled.errors";
			mCleanups.insert(lso_file);
			mCleanups.insert(error_file);
			ensure(lscript_compile(script_path.c_str(), lso_file.c_str(), error_file.c_str(), 0, "foo", 0));

			LLFILE* file = LLFile::fopen(lso_file.c_str(), "rb");  /* Flawfinder: ignore */
			return std::unique_ptr<LLScriptExecuteLSL2>(new LLScriptExecuteLSL2(file));
		}
	};

	typedef test_group<lscript_execute> lscript_execute_test;
	typedef lscript_execute_test::object lscript_execute_t;
	lscript_execute_test tut_lscript_execute("lscript_execute");

	template<> template<>
	void lscript_execute_t::test<1>()
	{
		std::unique_ptr<LLScriptExecuteLSL2> execute(loadScript("tests/lsl_conformance.lsl"));
		F32 time_slice = 3600.0f; // 1 hr.
		U32 events_processed = 0;
		do {
			const char *error;
			LLTimer timer;
			execute->runQuanta(FALSE, LLUUID::null, &error,
							   time_slice, events_processed, timer);
		} while (!execute->isFinished());
		ensure(!execute->getFaults());
		ensure_equals("IP register zeroed", get_register(execute->mBuffer, LREG_IP), 0x0);
		ensure_equals("SP register", get_register(execute->mBuffer, LREG_SP), 0x3FFF);
		ensure_equals("BP register", get_register(execute->mBuffer, LREG_BP), 0x3FFB);
		ensure_equals("HR register", get_register(execute->mBuffer, LREG_HR), 0x3C15);
		ensure_equals("HP register", get_register(execute->mBuffer, LREG_HP), 0x3E9F);
	}

	template<> template<>
	void lscript_execute_t::test<2>()
	{
		std::unique_ptr<LLScriptExecuteLSL2> execute(loadScript("tests/lsl_conformance.lsl"));
		F32 time_slice = 3600.0f; // 1 hr.
		U32 events_processed = 0;
		const char *error;
		LLTimer timer;
		// run for the smallest possible time slice
		execute->runQuanta(FALSE, LLUUID::null, &error,
						   0.0, events_processed, timer);
		// should not be completely done yet
		ensure(!execute->isFinished());

		// store the original state of the registers before state save / reload
		size_t regs_size = get_register(execute->mBuffer, LREG_GFR);
		std::unique_ptr<U8[]> orig_regs = std::unique_ptr<U8[]>(new U8[regs_size]);
		memcpy(orig_regs.get(), execute->mBuffer, regs_size);

		// can't be unique_ptr, writeState() will realloc.
		U8 *script_state = new U8[TOP_OF_MEMORY];
		execute->writeState(&script_state, 0, 0);
		execute = loadScript("tests/lsl_conformance.lsl");
		ensure("Read state succeeded", execute->readState(script_state) != -1);

		ensure("Register state equivalent", !memcmp(orig_regs.get(), execute->mBuffer, regs_size));
		do {
			execute->runQuanta(FALSE, LLUUID::null, &error,
							   time_slice, events_processed, timer);
		} while (!execute->isFinished());
		ensure("No faults", !execute->getFaults());
		ensure_equals("IP register zeroed", get_register(execute->mBuffer, LREG_IP), 0x0);
		ensure_equals("SP register", get_register(execute->mBuffer, LREG_SP), 0x3FFF);
		ensure_equals("BP register", get_register(execute->mBuffer, LREG_BP), 0x3FFB);
		ensure_equals("HR register", get_register(execute->mBuffer, LREG_HR), 0x3C15);
		ensure_equals("HP register", get_register(execute->mBuffer, LREG_HP), 0x3E9F);
	}
}
