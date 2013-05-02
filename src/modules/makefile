SRC_DIR = ..
TARGET = tokenizer memory directory history optab assembler shell
PART_NAME = modules
TEST_INCLUDING = $(OBJS_DIR)/core/*.o

build: $(addsuffix .o, $(TARGET))
tests: core_objs $(addsuffix .test, $(TARGET))
core_objs:
	make -C ../core/

include $(SRC_DIR)/rules.mk 
