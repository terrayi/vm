
TARGET= mingw32
PREFIX=
STRIP_OPTION= -s
COMPILER_TARGET_NAME= Compiler
VM_TARGET_NAME= Vm
COMPILER_TARGET= $(COMPILER_TARGET_NAME) 
VM_TARGET= $(VM_TARGET_NAME)

ifeq ($(TARGET), mingw32)
PREFIX= i386-mingw32-
COMPILER_TARGET=$(COMPILER_TARGET_NAME).exe
VM_TARGET=$(VM_TARGET_NAME).exe
else ifeq ($(TARGET), mac)
STRIP_OPTION= -S
endif

ifeq ($(DEBUG), 1)
	OPTION= -DDEBUG
else
	OPTION=
endif

CC= $(PREFIX)gcc
STRIP= $(PREFIX)strip $(STRIP_OPTION)

ifeq ($(PLATFORM), win32)
COMPILER_TARGET=$(COMPILER_TARGET_NAME).exe
VM_TARGET=$(VM_TARGET_NAME).exe
RM= del
else
RM= rm -f
endif

VM_SRC= \
	vm_test.c \
	vm.c

VM_HEADERS= \
	vm.h \
	vm_code_enums.h \
	vm_defines.h \
	vm_error_defines.h \
	vm_functions.h \
	vm_includes.h \
	vm_ms_struct.h \
	vm_ret_enums.h \
	vm_stack_struct.h \
	vm_state_enums.h \
	vm_typedefs.h \
	vm_vm_struct.h
	
COMPILER_SRC= \
	compiler_main.cpp \
	compiler.cpp

COMPILER_HEADERS = \
	ascii.h \
	compiler_includes.h \
	compiler_typedefs.h \
	compiler_structs.h \
	compiler_enums.h \
	compiler_mnemonics.h \
	compiler.h

VM_OBJ= \
	vm_test.o \
	vm.o

COMPILER_OBJ= $(COMPILER_SRC:.cpp=.o)

all: Compiler VM

VM: $(VM_OBJ)
	$(CC) -o $(VM_TARGET) $(VM_OBJ) $(OPTION)
	$(STRIP) $(VM_TARGET)

Compiler: $(COMPILER_OBJ) $(COMPILER_HEADERS)
	$(CC) -o $(COMPILER_TARGET) $(COMPILER_OBJ) -lstdc++ $(OPTION)
	$(STRIP) $(COMPILER_TARGET)

%.o: %.cpp
	$(CC) -c -o $@ $< $(OPTION)

%.o: %.c
	$(CC) -c -o $@ $< $(OPTION)

vm.o: vm.c $(VM_HEADERS)
compiler.o: compiler.cpp $(COMPILER_HEADER)

.PHONY: clean
clean:
	$(RM) *.o *.d

.PHONY: allclean
allclean: clean
	$(RM) $(COMPILER_TARGET) $(VM_TARGET)

