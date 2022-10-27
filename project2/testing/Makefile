CC = clang++
CPPFLAGS = -Wall -O2 -pedantic -std=c++11

NP_SHELL_CMD_SRC = $(wildcard src/cmd/*.cpp)
NP_SHELL_CMD = $(patsubst src/cmd/%.cpp,work_template/bin/%,$(NP_SHELL_CMD_SRC))
NP_SHELL_CMD_2 = $(patsubst src/cmd/%.cpp,work_template_2/bin/%,$(NP_SHELL_CMD_SRC))

FILE_NAME = $(wildcard src/file/*)
FILE = $(patsubst src/file/%,work_template/%,$(FILE_NAME))
FILE_2 = $(patsubst src/file/%,work_template_2/%,$(FILE_NAME))


BUILD_IN_CMD_NAME = ls cat wc
BUILD_IN_CMD = $(patsubst %,work_template/bin/%,$(BUILD_IN_CMD_NAME))
BUILD_IN_CMD_2 = $(patsubst %,work_template_2/bin/%,$(BUILD_IN_CMD_NAME))


.PHONY: work

all: work
	@rm -rf work_dir
	@rm -rf work_dir_2
	@cp -r work_template work_dir
	@cp -r work_template_2 work_dir_2


%: src/%.cpp
	$(CC) $< $(CPPFLAGS) -o $@

work: work_template/ $(FILE) $(NP_SHELL_CMD) $(BUILD_IN_CMD) work_template_2/ work_template_2/npbin/ $(FILE_2) $(NP_SHELL_CMD_2) $(BUILD_IN_CMD_2)

work_template/bin/%: src/cmd/%.cpp | work_template/bin/
	$(CC) $< $(CPPFLAGS) -o $@

work_template/bin/%: /bin/%
	cp -f $< $@

work_template/bin/:
	mkdir work_template/bin/

work_template/%: src/file/%
	cp $< $@

work_template/:
	mkdir work_template


work_template_2/bin/%: src/cmd/%.cpp | work_template_2/bin/
	$(CC) $< $(CPPFLAGS) -o $@

work_template_2/bin/%: /bin/%
	cp -f $< $@

work_template_2/bin/:
	mkdir work_template_2/bin/
	cp -r src/bins/bin/* work_template_2/bin/

work_template_2/npbin/:
	mkdir work_template_2/npbin/
	cp -r src/bins/npbin/* work_template_2/npbin/

work_template_2/%: src/file/%
	cp $< $@

work_template_2/:
	mkdir work_template_2

clean:
	rm -f $(NP_SHELL)
	rm -rf work_dir
	rm -rf work_template
	rm -rf work_template_2
