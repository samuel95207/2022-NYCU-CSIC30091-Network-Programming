

all: np_simple np_single_proc np_multi_proc


np_simple:
	@$(MAKE) -C src/np_simple
	@cp src/np_simple/np_simple ./np_simple

np_single_proc:
	@$(MAKE) -C src/np_single_proc
	@cp src/np_single_proc/np_single_proc ./np_single_proc

np_multi_proc:

commands: create_working_dir noop number removetag removetag0 ls cat

clean_command:
	rm working_dir/bin/*

clean_working_dir:
	rm -f working_dir/$(TARGET)
	rm -f working_dir/*.txt

create_working_dir:
	mkdir -p working_dir/bin

noop:
	g++ src/commands/noop.cpp -o working_dir/bin/noop
number:
	g++ src/commands/number.cpp -o working_dir/bin/number
removetag:
	g++ src/commands/removetag.cpp -o working_dir/bin/removetag
removetag0:
	g++ src/commands/removetag0.cpp -o working_dir/bin/removetag0
ls:
	cp /usr/bin/ls working_dir/bin/ls
cat:
	cp /usr/bin/cat working_dir/bin/cat


test1: np_simple
	@ - cd testing; \
	./demo.sh ../np_simple 7002

test2: np_single_proc
	@ - cd testing; \
	./demo.sh ../np_single_proc 7002