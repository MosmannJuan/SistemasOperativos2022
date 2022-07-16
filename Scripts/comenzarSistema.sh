#!/bin/bash
lxterminal --command="/bin/bash --init-file ./iniciarMemoria.sh" &
sleep 1
lxterminal --command="/bin/bash --init-file ./iniciarKernel.sh" &
sleep 1
lxterminal --command="/bin/bash --init-file ./iniciarCpu.sh" &
sleep 1