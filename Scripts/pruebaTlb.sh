#!/bin/bash
./comenzarSistema.sh
lxterminal --command="/bin/bash --init-file ./tlb1.sh" &
lxterminal --command="/bin/bash --init-file ./tlb2.sh" 


