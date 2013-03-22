SRC_DIR = .
ARGUMENTS = $(filter-out b.c build.core b.m build.modules, $(MAKECMDGOALS))

.PHONY: build tests
build tests: build.core build.modules

b.c build.core:
	make -C $(SRC_DIR)/core $(ARGUMENTS)

b.m build.modules: core
	make -C $(SRC_DIR)/modules $(ARGUMENTS)


include $(SRC_DIR)/rules.mk 
