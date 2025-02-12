TARGET_EXEC ?= myprogram
TARGET_TEST ?= test-lab

BUILD_DIR ?= build
TEST_DIR ?= tests
SRC_DIR ?= src
EXE_DIR ?= app

SRCS := $(shell find $(SRC_DIR) -name *.c)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

TEST_SRCS := $(shell find $(TEST_DIR) -name *.c)
TEST_OBJS := $(TEST_SRCS:%=$(BUILD_DIR)/%.o)
TEST_DEPS := $(TEST_OBJS:.o=.d)

EXE_SRCS := $(shell find $(EXE_DIR) -name *.c)
EXE_OBJS := $(EXE_SRCS:%=$(BUILD_DIR)/%.o)
EXE_DEPS := $(EXE_OBJS:.o=.d)

CFLAGS ?= -Wall -Wextra  -MMD -MP
DEBUG ?= -g
SANATIZE ?= -fno-omit-frame-pointer -fsanitize=address

#If you need to link against a library uncomment the line below and add the library name
#LDFLAGS ?= -pthread -lreadline

#Default to building without debug flags
all: $(TARGET_EXEC) $(TARGET_TEST)

#Build with debug flags and address sanitizer
#https://www.gnu.org/software/make/manual/make.html#Target_002dspecific
debug: CFLAGS += $(SANATIZE)
debug: CFLAGS += $(DEBUG)
debug: $(TARGET_EXEC) $(TARGET_TEST)

$(TARGET_EXEC): $(OBJS) $(EXE_OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(EXE_OBJS) -o $@ $(LDFLAGS)

$(TARGET_TEST): $(OBJS) $(TEST_OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(TEST_OBJS)  -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

check: $(TARGET_TEST)
	ASAN_OPTIONS=detect_leaks=1 ./$<

.PHONY: clean
clean:
	$(RM) -rf $(BUILD_DIR) $(TARGET_EXEC) $(TARGET_TEST)

# Install the libs needed to use git send-email on codespaces
.PHONY: install-deps
install-deps:
	sudo apt-get update -y
	sudo apt-get install -y libio-socket-ssl-perl libmime-tools-perl


-include $(DEPS) $(TEST_DEPS) $(EXE_DEPS)
