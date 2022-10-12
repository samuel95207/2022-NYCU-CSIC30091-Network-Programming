TARGET = ./npshell

SRC_DIR = ./src
SRC_SUBDIR += . 
INCLUDE_DIR += ./src
OBJ_DIR = ./src/obj

CC = g++
C_FLAGS = -g -Wall -std=c++17 -O3
LD = $(CC)
INCLUDES += -I$(INCLUDE_DIR)
LD_FLAFS += 
LD_LIBS =

ifeq ($(CC), g++)
	TYPE = cpp
else
	TYPE = c
endif

SRCS += ${foreach subdir, $(SRC_SUBDIR), ${wildcard $(SRC_DIR)/$(subdir)/*.$(TYPE)}}
OBJS += ${foreach src, $(notdir $(SRCS)), ${patsubst %.$(TYPE), $(OBJ_DIR)/%.o, $(src)}}

vpath %.$(TYPE) $(sort $(dir $(SRCS)))

all : $(TARGET)
	@echo "Builded target:" $^
	@echo "Done"

$(TARGET) : $(OBJS)
	@mkdir -p $(@D)
	@echo "Linking" $@ "from" $^ "..."
	$(LD) -o $@ $^ $(LD_FLAGS) $(LD_LIBS)
	@echo "Link finished"

$(OBJS) : $(OBJ_DIR)/%.o:%.$(TYPE)
	@mkdir -p $(@D)
	@echo "Compiling" $@ "from" $< "..."
	$(CC) -c -o $@ $< $(C_FLAGS) $(INCLUDES)
	@echo "Compile finished"

.PHONY : clean cleanobj
clean : cleanobj clean_working_dir clean_case
	@echo "Remove all executable files and output files"
	rm -f $(TARGET)

cleanobj :
	@echo "Remove object files"
	rm -rf $(OBJ_DIR)/*.o

test:
	@make
	@cp $(TARGET) working_dir/


testall:
	@make
	@cd testing; \
	make clean; \
	make ;\
	./demo.sh ../npshell



zip:
	@make clean
	@rm -f 311511034.zip
	@rm -rf 311511034
	@mkdir 311511034
	@mkdir 311511034/src
	@cp src/*.cpp 311511034/src
	@cp src/*.h 311511034/src
	@cp Makefile 311511034
	@zip -r 311511034.zip 311511034
	@rm -rf 311511034




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

