ELFBIT ?= 64
TARGET := scl

CCWARN := -Wall -Wextra
CCFLAG := -std=gnu99 -m$(ELFBIT) -fno-stack-protector -z execstack

$(TARGET): main.c
	@echo "[$(CC)] $@"
	@$(CC) $(CCWARN) $(CCFLAG) -o $@ $<

setuid:
	@echo "[chown] root:root $(TARGET)"
	@chown root:root $(TARGET)
	@echo "[chmod] u+s $(TARGET)"
	@chmod u+s $(TARGET)

clean:
	@echo "[rm] $(TARGET)"
	@rm -f $(TARGET) || true

.PHONY: setuid clean
