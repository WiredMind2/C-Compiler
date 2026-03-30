test-all:
	python3 ifcc-test.py testfiles


test-arm-%:
	python3 ifcc-test.py --arch arm testfiles/$*_*
test-x86-%:
	python3 ifcc-test.py --arch x86 testfiles/$*_*

# Usage:
#   make test-09 -> run all tests matching testfiles/09_*
test-%:
	@if [ -d testfiles/$* ]; then \
		python3 ifcc-test.py testfiles/$*; \
	else \
		python3 ifcc-test.py testfiles/$*_*; \
	fi


# Compiles the tests in the directory testfiles/%* both with and without optimizations, and compares the assembly
optim-compares-%:
	make -C compiler ifcc
	@set -e; \
	if [ -d testfiles/$* ]; then \
		test_dirs="testfiles/$*"; \
	else \
		test_dirs=$$(ls -d testfiles/$*_* 2>/dev/null || true); \
	fi; \
	if [ -z "$$test_dirs" ]; then \
		echo "No matching test directory for pattern '$*'"; \
		exit 1; \
	fi; \
	out_root=ifcc-optim-compare/$*; \
	rm -rf "$$out_root"; \
	mkdir -p "$$out_root"; \
	fails=0; \
	for dir in $$test_dirs; do \
		echo "==> $$dir"; \
		for src in $$(find "$$dir" -type f -name '*.c' | sort); do \
			name=$$(basename "$$src" .c); \
			src_base=$$(basename "$$src"); \
			case_out="$$out_root/$${name}"; \
			mkdir -p "$$case_out"; \
			src_copy="$$case_out/$$src_base"; \
			cp "$$src" "$$src_copy"; \
			asm_opt="$$case_out/opt.s"; \
			asm_noopt="$$case_out/noopt.s"; \
			diff_out="$$case_out/$${name}.diff"; \
			if ! compiler/ifcc "$$src_copy" > "$$asm_opt" 2> "$$case_out/opt.compile.log"; then \
				echo "  FAIL $$name (compile opt)"; \
				fails=$$((fails + 1)); \
				continue; \
			fi; \
			if ! compiler/ifcc --nooptim "$$src_copy" > "$$asm_noopt" 2> "$$case_out/noopt.compile.log"; then \
				echo "  FAIL $$name (compile noopt)"; \
				fails=$$((fails + 1)); \
				continue; \
			fi; \
			diff -u "$$asm_noopt" "$$asm_opt" > "$$diff_out" || true; \
			echo "  OK  $$name -> $$diff_out"; \
		done; \
	done; \
	echo "Summary: $$fails compile failures"; \
	if [ "$$fails" -ne 0 ]; then \
		exit 1; \
	fi


# Renumber tests in all test directories
renumber:
	@echo "Renumbering all test files..."
	@find testfiles -type d | while read dir; do \
		if ls "$$dir"/*.c >/dev/null 2>&1; then \
			echo "Renumbering tests in $$dir..."; \
			cd "$$dir" && \
			i=0; \
			for file in $$(ls -1 *.c 2>/dev/null | sort -V); do \
				newname=$$(echo $$file | sed -E "s/^[0-9]+_/$$(printf "%02d" $$i)_/"); \
				if [ "$$file" != "$$newname" ]; then \
					echo "  $$file -> $$newname"; \
					mv "$$file" "$$newname"; \
				fi; \
				i=$$((i + 1)); \
			done; \
			cd - >/dev/null; \
		fi; \
	done
	@echo "Done renumbering all tests."

clean:
	rm -rf compiler/build generated
	rm -f compiler/ifcc
