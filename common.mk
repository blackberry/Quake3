ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

USEFILE=

include $(MKFILES_ROOT)/qmacros.mk

LIBS+=$(if $(filter linux,$(OS)),stdc++)

ifdef LIBNAMES
LIBNAMES:=
endif

include $(MKFILES_ROOT)/qtargets.mk

OPTIMIZE_TYPE_g=none
OPTIMIZE_TYPE=$(OPTIMIZE_TYPE_$(filter g, $(VARIANTS)))
