#
#  $Id$
#

#
#  RTEMS_MAKEFILE_PATH is typically set in an environment variable
#


PGM=${ARCH}/pcmmio_shell.exe

# optional managers required
MANAGERS=all

#DEFINES += -DTESTING
CSRCS = pcmmio_shell.c mio_io.c mio_io_rtems.c multiio_pcmmio.c \
        FilesystemImage.c
COBJS = $(CSRCS:%.c=${ARCH}/%.o)

DEFINES += -I../include -I../commands
LD_LIBS += ../commands/${ARCH}/libmiocmds.a

CLEAN_ADDITIONS += FilesystemImage FilesystemImage.c FilesystemImage.h

include $(RTEMS_MAKEFILE_PATH)/Makefile.inc
include $(RTEMS_CUSTOM)
include $(PROJECT_ROOT)/make/leaf.cfg

OBJS= $(COBJS) $(CXXOBJS) $(ASOBJS)

all:    ${ARCH} FilesystemImage.c FilesystemImage.h $(OBJS) $(PGM)

${PGM}: $(OBJS)
	$(make-exe)

FilesystemImage: rootfs/test_adc rootfs/set_adc_modes
	(cd rootfs ; tar cf - --exclude=.svn \
	  --exclude=CVS --exclude=.cvsignore .) >$(@)

FilesystemImage.c FilesystemImage.h: FilesystemImage
	$(PROJECT_ROOT)/bin/rtems-bin2c FilesystemImage FilesystemImage

FilesystemImage.o: FilesystemImage.c
	$(CC) $(CFLAGS) $(CPU_CFLAGS) -c $<

