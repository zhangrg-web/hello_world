#
# Component Makefile
#
# This Makefile should, at the very least, just include $(SDK_PATH)/make/component.mk. By default,
# this will take the sources in this directory, compile them and link them into
# lib(subdirectory_name).a in the build directory. This behaviour is entirely configurable,
# please read the SDK documents if you need to do this.
#
COMPONENT_ADD_INCLUDEDIRS := .

COMPONENT_SRCDIRS := .

ifndef CONFIG_AIRKISS_CRYPT
LIBS := airkiss
else
LIBS := airkiss_aes
endif

#COMPONENT_ADD_LDFLAGS += -lmain -L . $(addprefix -l,$(LIBS))
COMPONENT_ADD_LDFLAGS += $(COMPONENT_PATH)/libairkiss.a

#COMPONENT_ADD_LINKER_DEPS += $(patsubst %,./lib%.a,$(LIBS))
COMPONENT_ADD_LINKER_DEPS := $(patsubst %,$(COMPONENT_PATH)/lib%.a,$(LIBS))
