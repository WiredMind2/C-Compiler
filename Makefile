test-all:
	python ifcc-test.py testfiles


test-arm-%:
	python ifcc-test.py --arch arm testfiles/$*_*
test-x86-%:
	python ifcc-test.py --arch x86 testfiles/$*_*

# Usage: `make test-xx` where `xx` is the suite number (e.g. `01`, `02`, etc.)
test-%:
	python ifcc-test.py testfiles/$*_*


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
