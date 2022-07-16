#!/bin/bash
./comenzarSistema.sh
lxterminal --command="/bin/bash --init-file ./consolaBase1.sh" &
lxterminal --command="/bin/bash --init-file ./consolaBase2.sh" &
lxterminal --command="/bin/bash --init-file ./consolaBase2.sh" 


