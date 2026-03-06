
test-all:
	python ifcc-test.py testfiles

test-last:
	python ifcc-test.py -v -v testfiles/$(shell ls -A testfiles | sort -V | tail -n 1)
