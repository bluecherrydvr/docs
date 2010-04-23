include make.cfg
OUTPUT_DIR?=$(CURDIR)/bin/linux
export OUTPUT_DIR
DRIVER_DIR = ./src/driver/linux
SDK_DIR	= ./src/sdk
SDVR_DIAG_DIR = ./apps/sdvr_diag
DVRCP_DIR = ./apps/dvrcp
.PHONY: driver sdk sdvr_diag dvrcp

#all: driver sdk sdvr_diag dvrcp
all: driver sdvr_diag dvrcp

driver:
	make -C $(DRIVER_DIR)

#sdk:
#	make -C $(SDK_DIR)

sdvr_diag:
	make -C $(SDVR_DIAG_DIR)

dvrcp:
ifeq ($(SKIP_UI),y)
	@echo "skip build $@ for $(TARGET_SYS)"
else	
	make -C $(DVRCP_DIR)
endif

clean:
	make -C $(DRIVER_DIR) clean
#	make -C $(SDK_DIR) clean
	make -C $(SDVR_DIAG_DIR) clean
ifneq ($(SKIP_UI),y)
	make -C $(DVRCP_DIR) clean
endif	
	rm -rf $(OUTPUT_DIR)/$(TARGET_SYS)

run:
	$(OUTPUT_DIR)/$(TARGET_SYS)/bin/dvrcp

load::
	make -C $(DRIVER_DIR) load

unload::
	make -C $(DRIVER_DIR) unload

test::
	make -C $(DRIVER_DIR) test
