DEST_DIR = $(SRC_DIR)/built
OBJS_DIR = $(DEST_DIR)/objs
TESTS_DIR = $(DEST_DIR)/tests
CFLAGS = -I$(SRC_DIR) -Wall

.PHONY: clean
clean:
	@rm -rf $(DEST_DIR)/../20091631.out $(DEST_DIR)/../output.txt $(DEST_DIR)/../*.lst $(DEST_DIR)/../*dlt $(DEST_DIR)
	@echo "Cleaned."

$(addsuffix .o, $(TARGET)): %.o: %.c %.h .mkdir.o
	gcc -g -c $(basename $@).c $(CFLAGS) -o $(OBJS_DIR)/$(PART_NAME)/$(basename $@).o

$(addsuffix .test, $(TARGET)): %.test: %.c %.h .mkdir.test
	gcc -g   $(basename $@).c $(CFLAGS) -o $(TESTS_DIR)/$(basename $@).test -D$(basename $@)_test $(TEST_INCLUDING)

.PHONY: .mkdir.o
.mkdir.o:
	@mkdir -p $(OBJS_DIR)/$(PART_NAME)

.PHONY: .mkdir.test
.mkdir.test:
	@mkdir -p $(TESTS_DIR)
