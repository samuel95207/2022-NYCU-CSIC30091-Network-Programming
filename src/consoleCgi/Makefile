TARGET = ./hw4.cgi

SRC_DIR = ./src
SRC_SUBDIR += . 
INCLUDE_DIR += ./src:/usr/local/include
LIB_DIR = /usr/local/lib
OBJ_DIR = ./src/obj

CC = g++
C_FLAGS = -g -Wall -std=c++17 -O3 -pedantic -pthread -lboost_system
LD = $(CC)
INCLUDES += -I $(INCLUDE_DIR)
LIBS += -L $(LIB_DIR)
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
	$(LD) -o $@ $^ $(LD_FLAGS) $(LD_LIBS) $(C_FLAGS)
	@echo "Link finished"

$(OBJS) : $(OBJ_DIR)/%.o:%.$(TYPE)
	@mkdir -p $(@D)
	@echo "Compiling" $@ "from" $< "..."
	$(CC) -c -o $@ $< $(INCLUDES) $(LIBS) $(C_FLAGS)
	@echo "Compile finished"

.PHONY : clean cleanobj
clean : cleanobj
	@echo "Remove all executable files and output files"
	rm -f $(TARGET)

cleanobj :
	@echo "Remove object files"
	rm -rf $(OBJ_DIR)/*.o

run:
	@make
	@cp $(TARGET) ../../working_dir/