#
#  $Id$
#

RTEMS_MAKEFILE_PATH=${HOME}/centaur/install/i386-rtems4.10/pc386/
#
#  RTEMS_MAKEFILE_PATH is typically set in an environment variable
#

#MAINS= buffered.c dacbuff.c dacout.c flash.c getall.c getvolt.c \
#  poll.c repeat.c simple_rtems.c pcmmio_shell.c

MAINS= pcmmio_shell.c

PGMS=$(MAINS:%.c=${ARCH}/%.exe)

# optional managers required
MANAGERS=all
#DEFINES += -DTESTING

# C source names
#SHARED_CSRCS += mio_io_linux.c
SHARED_CSRCS = mio_io.c kbhit.c 
SHARED_CSRCS += main_pcmmio_adc_mode.c
SHARED_CSRCS += main_pcmmio_din.c main_pcmmio_dout.c
SHARED_CSRCS += main_pcmmio_adc.c main_pcmmio_dac.c
SHARED_CSRCS += main_pcmmio_irq.c
SHARED_CSRCS += main_pcmmio_benchmark.c
SHARED_CSRCS += mio_io_rtems.c
SHARED_CSRCS += FilesystemImage.c

SHARED_COBJS = $(SHARED_CSRCS:%.c=${ARCH}/%.o)
SHARED_CONFIG = ${ARCH}/rtems_config.o

CLEAN_ADDITIONS += FilesystemImage FilesystemImage.c FilesystemImage.h

include $(RTEMS_MAKEFILE_PATH)/Makefile.inc
include $(RTEMS_CUSTOM)
include $(PROJECT_ROOT)/make/leaf.cfg

LDFLAGS += -g
OBJS= $(COBJS) $(CXXOBJS) $(ASOBJS) $(SHARED_COBJS) $(SHARED_CONFIG)

all:    ${ARCH} FilesystemImage.c FilesystemImage.h $(OBJS) $(PGMS)

${ARCH}/pcmmio_shell.exe: pcmmio_shell.c $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $< $(SHARED_COBJS)

${ARCH}/%.exe: %.c $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $< $(SHARED_COBJS) $(SHARED_CONFIG)

FilesystemImage: rootfs/test_adc rootfs/set_adc_modes
	(cd rootfs ; tar cf - --exclude=.svn \
	  --exclude=CVS --exclude=.cvsignore .) >$(@)

FilesystemImage.c FilesystemImage.h: FilesystemImage
	$(PROJECT_ROOT)/bin/rtems-bin2c FilesystemImage FilesystemImage

FilesystemImage.o: FilesystemImage.c
	$(CC) $(CFLAGS) $(CPU_CFLAGS) -c $<

