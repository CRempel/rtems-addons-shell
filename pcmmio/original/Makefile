#
#  $Id$
#

RTEMS_MAKEFILE_PATH=/home/joel/rtems-4.10-work/bsp-install/i386-rtems4.10/pc386/
#
#  RTEMS_MAKEFILE_PATH is typically set in an environment variable
#

MAINS= buffered.c # dacbuff.c dacout.c flash.c getall.c getvolt.c poll.c repeat.c

PGMS=$(MAINS:%.c=${ARCH}/%.exe)

# optional managers required
MANAGERS=all

# C source names
SHARED_CSRCS = rtems_config.c mio_io.c kbhit.c
SHARED_CSRCS += mio_io_linux.c

SHARED_COBJS = $(SHARED_CSRCS:%.c=${ARCH}/%.o)

include $(RTEMS_MAKEFILE_PATH)/Makefile.inc
include $(RTEMS_CUSTOM)
include $(PROJECT_ROOT)/make/leaf.cfg

OBJS= $(COBJS) $(CXXOBJS) $(ASOBJS)

all:    ${ARCH} $(SHARED_COBJS) $(PGMS)

${ARCH}/%.exe: %.c
	$(CC) -o $@ $< $(SHARED_COBJS)
