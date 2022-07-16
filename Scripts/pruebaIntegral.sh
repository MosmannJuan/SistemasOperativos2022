#!/bin/bash
./comenzarSistema.sh
lxterminal --command="/bin/bash --init-file ./integral1.sh" &
lxterminal --command="/bin/bash --init-file ./integral2.sh" &
lxterminal --command="/bin/bash --init-file ./integral3.sh" &
lxterminal --command="/bin/bash --init-file ./integral4.sh" & 
lxterminal --command="/bin/bash --init-file ./integral5.sh" 
 


