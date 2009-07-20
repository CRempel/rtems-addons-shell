#
#  $Id$
#

RTEMS_MAKEFILE_PATH=/home/joel/rtems-4.10-work/bsp-install/i386-rtems4.10/pc386/
#
#  RTEMS_MAKEFILE_PATH is typically set in an environment variable
#

MAINS= buffered.c dacbuff.c dacout.c flash.c getall.c getvolt.c \
  poll.c repeat.c simple_rtems.c pcmmio_shell.c

PGMS=$(MAINS:%.c=${ARCH}/%.exe)

# optional managers required
MANAGERS=all
DEFINES += -DTESTING

# C source names
SHARED_CSRCS = mio_io.c kbhit.c main_pcmmio_din.c main_pcmmio_dout.c
#SHARED_CSRCS += mio_io_linux.c
SHARED_CSRCS += mio_io_rtems.c

SHARED_COBJS = $(SHARED_CSRCS:%.c=${ARCH}/%.o)
SHARED_CONFIG = ${ARCH}/rtems_config.o

include $(RTEMS_MAKEFILE_PATH)/Makefile.inc
include $(RTEMS_CUSTOM)
include $(PROJECT_ROOT)/make/leaf.cfg

OBJS= $(COBJS) $(CXXOBJS) $(ASOBJS) $(SHARED_COBJS) $(SHARED_CONFIG)

all:    ${ARCH} $(OBJS) $(PGMS)

${ARCH}/pcmmio_shell.exe: pcmmio_shell.c $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $< $(SHARED_COBJS)

${ARCH}/%.exe: %.c $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $< $(SHARED_COBJS) $(SHARED_CONFIG)
