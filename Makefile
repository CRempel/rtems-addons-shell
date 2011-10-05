#
#  $Id$
#

include $(RTEMS_MAKEFILE_PATH)/Makefile.inc
include $(RTEMS_CUSTOM)
include $(RTEMS_SHARE)/make/directory.cfg

SUBDIRS  = commands
SUBDIRS += pcmmio
SUBDIRS += rtd6425 rtd6425_shell
