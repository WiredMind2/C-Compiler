test-all:
	python ifcc-test.py testfiles

test-all-v:
	python ifcc-test.py -v testfiles

test-all-vv:
	python ifcc-test.py -v -v testfiles

test-00:
	python ifcc-test.py testfiles/00_base

test-00-v:
	python ifcc-test.py -v testfiles/00_base

test-00-vv:
	python ifcc-test.py -v -v testfiles/00_base

test-01:
	python ifcc-test.py testfiles/01_variables

test-01-v:
	python ifcc-test.py -v testfiles/01_variables

test-01-vv:
	python ifcc-test.py -v -v testfiles/01_variables

test-02:
	python ifcc-test.py testfiles/02_excepted_fail

test-02-v:
	python ifcc-test.py -v testfiles/02_excepted_fail

test-02-vv:
	python ifcc-test.py -v -v testfiles/02_excepted_fail

test-03:
	python ifcc-test.py testfiles/03_arithmetic_expr

test-03-v:
	python ifcc-test.py -v testfiles/03_arithmetic_expr

test-03-vv:
	python ifcc-test.py -v -v testfiles/03_arithmetic_expr

test-03-00:
	python ifcc-test.py testfiles/03_arithmetic_expr/mult_div_add_sub

test-03-00-v:
	python ifcc-test.py -v testfiles/03_arithmetic_expr/mult_div_add_sub

test-03-00-vv:
	python ifcc-test.py -v -v testfiles/03_arithmetic_expr/mult_div_add_sub

test-99:
	python ifcc-test.py testfiles/99_unimplemented

test-99-v:
	python ifcc-test.py -v testfiles/99_unimplemented

test-99-vv:
	python ifcc-test.py -v -v testfiles/99_unimplemented

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

