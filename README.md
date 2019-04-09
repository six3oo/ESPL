This repository contains the Demo project for the ESPL lib hosted by the TUM RCS chair.
Please contact Alex Hoffman (alex.hoffman@tum.de)) for further information or visit
the course web site: http://www.rcs.ei.tum.de/lehre/praktika/pr-espl/


HINT on setting up the debugger:
 - Tab "Main": choose your projects .elf file
 - Tab "Debugger":
        . Check "Start OpenOCD locally"
        . for Executable give path or alias of your openocd and arm-none-eabi-gdb binary respecively
        . GDB port: 3333
        . Telnet port: 4444
        . config options: "-s path/to/your/openocd/configs -f board/stm32f429discovery.cfg"
 - Tab "Startup": Choose your .elf file to use explicitly
