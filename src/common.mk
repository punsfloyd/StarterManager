# This is an automatically generated record.
# The area between QNX Internal Start and QNX Internal End is controlled by
# the QNX IDE properties.

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

#USEFILE=

define PINFO
PINFO DESCRIPTION=Variant Detect Service - Control startup based on LUM
PINFO STATE=Release
PINFO VERSION= 3.7.0
endef

#===== NAME - name of the project (default - name of project directory).
NAME=startermanager

#===== CCFLAGS - add the flags to the C compiler command line. 
CCFLAGS+=-O3
CCFLAGS_g+=-O0

#===== LIBS - a space-separated list of library items to be included in the link.
LIBS+=-Bstatic pps -Bdynamic

#===== EXTRA_INCVPATH - a space-separated list of directories to search for include files.
EXTRA_INCVPATH+=$(PROJECT_ROOT)/../target/qnx6/usr/include

#===== EXTRA_LIBVPATH - a space-separated list of directories to search for library files.
EXTRA_LIBVPATH+=$(PROJECT_ROOT)/../target/qnx6/armle-v7/lib  \
	$(PROJECT_ROOT)/../target/qnx6/armle-v7/usr/lib

#===== USEFILE - the file containing the usage message for the application. 
USEFILE=

#===== POST_BUILD - extra steps to do after building the image.
define POST_BUILD
-cp -rvf $(BUILDNAME) $(PROJECT_ROOT)/../target/qnx6/armle-v7/bin/$(BUILDNAME)
endef

include $(MKFILES_ROOT)/qmacros.mk
ifndef QNX_INTERNAL
QNX_INTERNAL=$(PROJECT_ROOT)/.qnx_internal.mk
endif
include $(QNX_INTERNAL)

include $(MKFILES_ROOT)/qtargets.mk

OPTIMIZE_TYPE_g=none
OPTIMIZE_TYPE=$(OPTIMIZE_TYPE_$(filter g, $(VARIANTS)))

