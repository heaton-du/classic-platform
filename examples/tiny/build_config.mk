
-include ../config/*.mk
-include ../config/$(BOARDDIR)/*.mk

MOD_USE+=KERNEL MCU

SELECT_CONSOLE = RAMLOG
SELECT_OS_CONSOLE = RAMLOG

def-y += CFG_RAMLOG_SIZE=1024
#def-y += HEAPSIZE=400
