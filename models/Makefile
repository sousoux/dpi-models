ROOT_DPI_BUILD_DIR ?= $(CURDIR)/build

DPI_DIRS=test/spim_verif jtag/proxy uart/uart camera


-include $(PULP_SDK_HOME)/install/rules/dpi_rules.mk


build: dpi_build

clean: dpi_clean

.PHONY: clean build
