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
CSRCS = pcmmio_shell.c mio_io.c mio_io_rtems.c multiio_pcmmio.c
COBJS = $(CSRCS:%.c=${ARCH}/%.o)

# C source names
#IGNORE mio_io_linux.c for RTEMS

## Older PCMMIO Specific Commands
##SHARED_CSRCS += main_pcmmio_din.c main_pcmmio_dout.c
##SHARED_CSRCS += main_pcmmio_adc.c main_pcmmio_dac.c
##SHARED_CSRCS += main_pcmmio_irq.c
##SHARED_CSRCS += main_pcmmio_benchmark.c
## Helper for Winsystems Mains.
##SHARED_CSRCS += kbhit.c 

SHARED_COBJS = $(SHARED_CSRCS:%.c=${ARCH}/%.o)
SHARED_CONFIG = ${ARCH}/rtems_config.o
DEFINES += -I../include -I../commands
LD_LIBS += ../commands/${ARCH}/libmiocmds.a

include $(RTEMS_MAKEFILE_PATH)/Makefile.inc
include $(RTEMS_CUSTOM)
include $(PROJECT_ROOT)/make/leaf.cfg

OBJS= $(COBJS) $(CXXOBJS) $(ASOBJS)

all:    ${ARCH} $(OBJS) $(PGM)

${PGM}: $(OBJS)
	$(make-exe)
