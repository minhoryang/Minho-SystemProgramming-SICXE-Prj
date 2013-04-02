SRC_DIR = ..
TARGET = tokenizer memory directory history
PART_NAME = modules
TEST_INCLUDING = $(OBJS_DIR)/core/*.o

build: $(addsuffix .o, $(TARGET)) shell.o
tests: core_objs $(addsuffix .test, $(TARGET)) shell.test
core_objs:
	make -C ../core/

include $(SRC_DIR)/rules.mk 

# XXX : shell has tokenizer-dependency.
shell.o: shell.c shell.h .mkdir.o
	gcc -c shell.c $(CFLAGS) -o $(OBJS_DIR)/$(PART_NAME)/shell.o 
shell.test: shell.c shell.h tokenizer.o directory.o memory.o history.o
	gcc    shell.c $(CFLAGS) -o $(TESTS_DIR)/shell.test -Dshell_test $(TEST_INCLUDING) $(OBJS_DIR)/$(PART_NAME)/tokenizer.o $(OBJS_DIR)/$(PART_NAME)/directory.o $(OBJS_DIR)/$(PART_NAME)/memory.o $(OBJS_DIR)/$(PART_NAME)/history.o
