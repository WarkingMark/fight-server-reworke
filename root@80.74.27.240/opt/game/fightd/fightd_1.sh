#!/bin/sh
. ../../param.sh
while [ 1 ] ;
	do
  ulimit -c 100000
    ./fightd -h "127.0.0.1" -p ${FIGHTDP} -c ${FIGHTCP} -l "-1" -d _fightd.log -f ${HTTP}://${FIGHTDOMAIN}/private/fsfeedback.php
  done

# -d _fightd_5466.log
