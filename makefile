SRC_DIR = src
ARGUMENTS = $(filter-out build, $(MAKECMDGOALS))

build tests:
	make -C $(SRC_DIR) $(ARGUMENTS)
	@# ignore for passing ARGUMENTS!

include $(SRC_DIR)/rules.mk 
