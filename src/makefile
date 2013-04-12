SRC_DIR = .
TARGET = 20091631
ARGUMENTS = $(filter-out b.c build.core b.m build.modules, $(MAKECMDGOALS))

.PHONY: build
build: build.core build.modules 20091631.o
	gcc -Wall -o $(DEST_DIR)/20091631.out $(OBJS_DIR)/*/*.o $(OBJS_DIR)/20091631.o
	@cp $(DEST_DIR)/20091631.out $(DEST_DIR)/../20091631.out

.PHONY: tests
tests: build.core build.modules

b.c build.core:
	make -C $(SRC_DIR)/core $(ARGUMENTS)

b.m build.modules: core
	make -C $(SRC_DIR)/modules $(ARGUMENTS)


include $(SRC_DIR)/rules.mk 
#CFLAGS += -DDEBUG
