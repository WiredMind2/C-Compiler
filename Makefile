test-all:
	python3 ifcc-test.py testfiles


test-arm-%:
	python3 ifcc-test.py --arch arm testfiles/$*_*
test-x86-%:
	python3 ifcc-test.py --arch x86 testfiles/$*_*

# Usage:
#   make test-09              -> run all tests matching testfiles/09_*
#   make test-09_break_continue -> run tests in testfiles/09_break_continue exactly
test-%:
	@if [ -d testfiles/$* ]; then \
		python3 ifcc-test.py testfiles/$*; \
	else \
		python3 ifcc-test.py testfiles/$*_*; \
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
