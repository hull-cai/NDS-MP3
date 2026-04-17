#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>devkitPro")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITPRO)/libnds/include/nds_rules

TARGET      := nds-mp3-player
BUILD       := build
SOURCES     := source
INCLUDES    := include

ARCH        := -mthumb -mthumb-interwork
CFLAGS      := -g -Wall -O2 $(ARCH)
CXXFLAGS    := $(CFLAGS) -fno-rtti -fno-exceptions
LDFLAGS     := -g $(ARCH) -Wl,-Map,$(notdir $*.map)

LIBS        := -lnds9 -lfat

#---------------------------------------------------------------------------------
# Automatically build a list of object files for our project
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------
export OUTPUT   := $(CURDIR)/$(TARGET)

export VPATH    := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR  := $(CURDIR)/$(BUILD)

CFILES      := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES    := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES      := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

export LD       := $(CC)
export OFILES   := $(addsuffix .o,$(CPPFILES:.cpp=)) \
                   $(addsuffix .o,$(CFILES:.c=)) \
                   $(addsuffix .o,$(SFILES:.s=))

export INCLUDE  := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                   -I$(CURDIR)/$(BUILD)

export LIBPATHS := -L$(LIBNDS)

.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).nds $(TARGET).map

#---------------------------------------------------------------------------------
else

DEPENDS := $(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).nds: $(OUTPUT).elf

$(OUTPUT).elf: $(OFILES)

#---------------------------------------------------------------------------------
-include $(DEPENDS)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
