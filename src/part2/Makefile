ifeq ($(OS), Windows_NT)
	MAKEDIR = powershell IF(-not (Test-Path $(1))){mkdir $(1)}
	REMOVE = powershell IF(Test-Path $(1)){rm -r -fo $(1)}
	COPY = Copy
else
	MAKEDIR = mkdir -p $(1)
	REMOVE = rm -rf $(1)
	COPY = cp
endif


TARGET = ./cgi_server.exe

SRC_DIR = ./src
SRC_SUBDIR += . ConsoleCgi PanelCgi
INCLUDE_DIR = 
LIB_DIR = 
OBJ_DIR = ./src/obj


CC = g++
C_FLAGS = -g -Wall -std=c++17 -O3 -pedantic -pthread -lboost_system -lws2_32 -lwsock32
LD = $(CC)
INCLUDES = -I $(INCLUDE_DIR)
LIBS = -L $(LIB_DIR)
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
	@$(call MAKEDIR,$(@D))
	@echo "Linking" $@ "from" $^ "..."
	$(LD) -o $@ $^ $(LD_FLAGS) $(LD_LIBS) $(C_FLAGS)
	@echo "Link finished"

$(OBJS) : $(OBJ_DIR)/%.o:%.$(TYPE)
	@$(call MAKEDIR,$(@D))
	@echo "Compiling" $@ "from" $< "..."
	$(CC) -c -o $@ $< $(INCLUDES) $(LIBS) $(C_FLAGS)
	@echo "Compile finished"

.PHONY : clean cleanobj
clean : cleanobj
	@echo "Remove all executable files and output files"
	@$(call REMOVE,$(TARGET))
	

cleanobj :
	@echo "Remove object files"
	@$(call REMOVE,$(OBJ_DIR)/*.o)

run:
	@make
	@cp $(TARGET) ../../working_dir/
	@echo Running $(TARGET)
	@cd ../../working_dir/; $(TARGET) 7001