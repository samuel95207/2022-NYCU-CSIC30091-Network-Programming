

all: part1 part2 part3


part1:
	@$(MAKE) -C src/np_simple
	@cp src/np_simple/np_simple ./np_simple

part2:
	@$(MAKE) -C src/np_single_proc
	@cp src/np_single_proc/np_single_proc ./np_single_proc

part3:
	@$(MAKE) -C src/np_multi_proc
	@cp src/np_multi_proc/np_multi_proc ./np_multi_proc

working_dir: create_working_dir noop number removetag removetag0 ls cat

clean_command:
	rm working_dir/bin/*

clean_working_dir:
	rm -f working_dir/np_simple
	rm -f working_dir/np_single_proc
	rm -f working_dir/np_multi_proc
	rm -f working_dir/*.txt

create_working_dir:
	mkdir -p working_dir/bin
	mkdir -p working_dir/user_pipe

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


clean1:
	@$(MAKE) clean -C src/np_simple

clean2:
	@$(MAKE) clean -C src/np_single_proc

clean3:
	@$(MAKE) clean -C src/np_multi_proc

clean: clean1 clean2 clean3
	rm -f working_dir/np_simple
	rm -f working_dir/np_single_proc
	rm -f working_dir/np_multi_proc
	rm -f np_simple
	rm -f np_single_proc
	rm -f np_multi_proc


run1: part1
	@$(MAKE) run -C src/np_simple

run2: part2
	@$(MAKE) run -C src/np_single_proc

run3: part3
	@$(MAKE) run -C src/np_multi_proc


test1: part1
	@ - cd testing; \
	./demo.sh ../np_simple 7002

test2: part2
	@ - cd testing; \
	./demo.sh ../np_single_proc 7002

test3: part3
	@ - cd testing; \
	./demo.sh ../np_multi_proc 7002

testall: np_simple np_single_proc np_multi_proc
	@ - cd testing; \
	./demo.sh ../ 7002 7003 7004


zip:
	@make clean
	@rm -f 311511034.zip
	@rm -rf 311511034
	@mkdir 311511034
	@mkdir 311511034/src/np_simple/src
	@mkdir 311511034/src/np_single_proc/src
	@mkdir 311511034/src/np_multi_proc/src
	@cp src/np_simple/src/*.cpp 311511034/src/np_simple/src
	@cp src/np_simple/src/*.h 311511034/src/np_simple/src
	@cp src/np_single_proc/src/*.cpp 311511034/src/np_single_proc/src
	@cp src/np_single_proc/src/*.h 311511034/src/np_single_proc/src
	@cp src/np_multi_proc/src/*.cpp 311511034/src/np_multi_proc/src
	@cp src/np_multi_proc/src/*.h 311511034/src/np_multi_proc/src
	@cp Makefile 311511034
	@zip -r 311511034.zip 311511034
	@rm -rf 311511034