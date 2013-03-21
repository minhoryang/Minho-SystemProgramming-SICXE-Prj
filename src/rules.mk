DEST_DIR = $(SRC_DIR)/../build
OBJS_DIR = $(DEST_DIR)/objs
TESTS_DIR = $(DEST_DIR)/tests
CFLAGS = -I$(SRC_DIR) -Wall

clean:
	rm -rf $(DEST_DIR)

$(TARGET).o: $(TARGET).c $(TARGET).h .mkdir.o
	gcc -c $(TARGET).c $(CFLAGS) -o $(OBJS_DIR)/$(TARGET).o

$(TARGET).test: $(TARGET).c $(TARGET).h .mkdir.test
	gcc    $(TARGET).c $(CFLAGS) -D$(TARGET)_test -o $(TESTS_DIR)/$(TARGET).test

.mkdir.o:
	@mkdir -p $(OBJS_DIR)

.mkdir.test:
	@mkdir -p $(TESTS_DIR)
