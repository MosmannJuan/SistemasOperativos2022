#!/bin/bash
./comenzarSistema.sh
lxterminal --command="/bin/bash --init-file ./suspe1.sh" &
lxterminal --command="/bin/bash --init-file ./suspe2.sh" &
lxterminal --command="/bin/bash --init-file ./suspe3.sh" 


