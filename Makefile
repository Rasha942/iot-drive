TARGET = none

# I - Additional Includes' folder
# LF - Additional Linker's flags

LF =
I = /usr/include/simpleini

WORKSPACE = $(CURDIR)/
LIBS = $(WORKSPACE)libs/
OBJS = $(WORKSPACE)objs/
PROGS = $(WORKSPACE)progs/


SRC = $(WORKSPACE)src/
INC = $(WORKSPACE)include/
TST = $(WORKSPACE)test/
STD = -std=c++20

CC ?= g++
CFLAGS ?= $(STD) -g -pedantic-errors -Wall -Wextra  -fPIC -I $(INC) -I ${I}
LDFLAGS ?= -L $(LIBS) -l$(TARGET) $(LF) -Wl,-rpath=$(LIBS) -lm -lstdc++ -lmysqlcppconn
VLGFLAGS = valgrind --leak-check=yes --track-origins=yes


ifneq ($(TARGET),none)
DEPENDENCIES = $(shell $(CC) -MM $(SRC)$(TARGET).cpp $(CFLAGS) | sed '/SimpleIni.h/d')
DEPENDENCIES_SRC = $(shell echo $(subst $(TARGET).o: ,,$(patsubst %.hpp, %.cpp,$(subst inc,src,$(DEPENDENCIES)))))
DEPENDENCIES_OBJ = $(shell basename -a $(patsubst %.cpp, %.o,$(DEPENDENCIES_SRC)))
endif

DEBUG = 1
SUBLIB = 0

.PHONY: default clean cleano cleanall release debug all test help run vlg

ifeq ($(TARGET),none)
default:
	@echo invalid option
	@echo try \'make help\' for more information.
endif

$(PROGS)$(TARGET).out: $(LIBS)lib$(TARGET).so $(OBJS)$(TARGET)_test.o
	@$(CC)  $(OBJS)$(TARGET)_test.o -o $@ $(LDFLAGS)
	@echo $@ Compiled Successfuly!
#endif
$(LIBS)lib$(TARGET).so: $(addprefix $(OBJS), $(DEPENDENCIES_OBJ))
	$(CC) -shared -o $@ $^ -fPIC
	@echo $@ Library File Created!

	
$(OBJS)%.o: $(SRC)%.cpp
ifeq ($(DEBUG), 1)
	$(CC) -c $< -g $(CFLAGS) -o $@ 
else
	$(CC) -c $< -g $(CFLAGS) -o $@ 
endif
	@echo $@ Object File Created!
		

$(OBJS)$(TARGET)_test.o: $(TST)$(TARGET)_test.cpp
	@$(CC) -c $< -g $(CFLAGS) -o $@
	@echo $@ Object File Created!
	


cleanall:
	@rm $(addprefix $(OBJS), $(DEPENDENCIES_OBJ)) $(LIBS)lib$(TARGET).so $(PROGS)$(TARGET).out -f
	@echo All files Removed Successfuly!
	
clean:
	@rm *.o *.so -f 
	@echo Object files and libraries Removed Successfuly!

cleano:
	@rm *.o -f
	@echo All Object files Removed Successfuly!

release: 
	@make -s TARGET=$(TARGET) DEBUG=0

debug: $(PROG)$(TARGET).out

run: $(PROGS)$(TARGET).out
	@$< $(ARGS)

vlg: $(PROGS)$(TARGET).out
	$(VLGFLAGS) $(PROGS)$(TARGET).out $(ARGS)
	
all: release lib$(TARGET).so
	@make -s clean

help:
	@echo "IOT Drive build system"
	@echo ""
	@echo "Usage: make TARGET=<master|minion> [DEBUG=0|1]"
	@echo ""
	@echo "Targets:"
	@echo "  make TARGET=master    Build the master node (progs/master.out)"
	@echo "  make TARGET=minion    Build a minion node (progs/minion.out)"
	@echo "  make TARGET=<t> run   Build and run target t"
	@echo "  make TARGET=<t> vlg   Build and run target t under valgrind"
	@echo "  make TARGET=<t> clean Remove object files and libraries"
	@echo ""
	@echo "Requires: g++ (C++20), libmysqlcppconn-dev, simpleini headers."

