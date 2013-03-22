DEST_DIR = $(SRC_DIR)/../build
OBJS_DIR = $(DEST_DIR)/objs
TESTS_DIR = $(DEST_DIR)/tests
CFLAGS = -I$(SRC_DIR) -Wall

.PHONY: clean
clean:
	rm -rf $(DEST_DIR)

$(addsuffix .o, $(TARGET)): %.o: %.c %.h .mkdir.o
	gcc -c $(basename $@).c $(CFLAGS) -o $(OBJS_DIR)/$(basename $@).o

$(addsuffix .test, $(TARGET)): %.test: %.c %.h .mkdir.test
	gcc    $(basename $@).c $(CFLAGS) -o $(TESTS_DIR)/$(basename $@).test -D$(basename $@)_test

.PHONY: .mkdir.o
.mkdir.o:
	@mkdir -p $(OBJS_DIR)

.PHONY: .mkdir.test
.mkdir.test:
	@mkdir -p $(TESTS_DIR)
