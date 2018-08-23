VSIM_DETECTED_PATH=$(dir $(shell which vsim))
BUILD_DIR=build

CFLAGS += -std=gnu++11 -MMD -MP -O3 -g

CFLAGS += -I$(PULP_SDK_WS_INSTALL)/include -fPIC
LDFLAGS += -L$(PULP_SDK_WS_INSTALL)/lib -fPIC -shared -O3 -g -ljson

DPI_CFLAGS += $(CFLAGS) -DUSE_DPI
DPI_LDFLAGS += $(LDFLAGS)  -Wl,-export-dynamic -ldl -rdynamic -lpulpperiph

ifdef VSIM_INCLUDE
DPI_CFLAGS += -Iext/sv/include -I$(VSIM_INCLUDE) -DVSIM_INCLUDE=1
else
DPI_CFLAGS += -Iext/sv/include -Iext/nosv
endif

PERIPH_CFLAGS += $(CFLAGS) $(DPI_CFLAGS)
PERIPH_LDFLAGS += $(LDFLAGS)  -Wl,-export-dynamic -ldl -rdynamic

DPI_SRCS = src/dpi.cpp src/qspim.cpp src/jtag.cpp src/ctrl.cpp src/uart.cpp src/cpi.cpp
PERIPH_SRCS = src/models.cpp src/qspim.cpp src/jtag.cpp src/ctrl.cpp src/uart.cpp src/cpi.cpp

DPI_OBJS = $(patsubst %.cpp,$(BUILD_DIR)/dpi/%.o,$(patsubst %.c,$(BUILD_DIR)/dpi/%.o,$(DPI_SRCS)))
PERIPH_OBJS = $(patsubst %.cpp,$(BUILD_DIR)/periph/%.o,$(patsubst %.c,$(BUILD_DIR)/periph/%.o,$(PERIPH_SRCS)))

-include $(DPI_OBJS:.o=.d)
-include $(PERIPH_OBJS:.o=.d)

$(BUILD_DIR)/dpi/%.o: %.cpp
	@mkdir -p $(basename $@)
	g++ $(DPI_CFLAGS) -o $@ -c $<

$(BUILD_DIR)/dpi/%.o: %.c
	@mkdir -p $(basename $@)
	g++ $(DPI_CFLAGS) -o $@ -c $<

$(BUILD_DIR)/periph/%.o: %.cpp
	@mkdir -p $(basename $@)
	g++ $(PERIPH_CFLAGS) -o $@ -c $<

$(BUILD_DIR)/periph/%.o: %.c
	@mkdir -p $(basename $@)
	g++ $(PERIPH_CFLAGS) -o $@ -c $<

$(BUILD_DIR)/libpulpdpi.so: $(DPI_OBJS)
	@mkdir -p $(basename $@)
	g++ -o $@ $^ $(DPI_LDFLAGS)

$(BUILD_DIR)/libpulpperiph.so: $(PERIPH_OBJS)
	@mkdir -p $(basename $@)
	g++ -o $@ $^ $(PERIPH_LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)
	make -C models clean


$(PULP_SDK_WS_INSTALL)/lib/libpulpdpi.so: $(BUILD_DIR)/libpulpdpi.so
	install -D $< $@

$(PULP_SDK_WS_INSTALL)/lib/libpulpperiph.so: $(BUILD_DIR)/libpulpperiph.so
	install -D $< $@

$(PULP_SDK_INSTALL)/rules/dpi_rules.mk: dpi_rules.mk
	install -D $< $@


INSTALL_TARGETS += $(PULP_SDK_WS_INSTALL)/lib/libpulpperiph.so
INSTALL_TARGETS += $(PULP_SDK_WS_INSTALL)/lib/libpulpdpi.so

HEADER_FILES += $(shell find include -name *.hpp)
HEADER_FILES += $(shell find include -name *.h)


define declareInstallFile

$(PULP_SDK_WS_INSTALL)/$(1): $(1)
	install -D $(1) $$@

INSTALL_HEADERS += $(PULP_SDK_WS_INSTALL)/$(1)

endef


$(foreach file, $(HEADER_FILES), $(eval $(call declareInstallFile,$(file))))


build: $(INSTALL_HEADERS) $(INSTALL_TARGETS) $(PULP_SDK_INSTALL)/rules/dpi_rules.mk
	make -C models build


checkout:
	git submodule update --init

.PHONY: checkout build install
