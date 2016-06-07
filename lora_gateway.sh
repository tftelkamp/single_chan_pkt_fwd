#! /bin/sh
### BEGIN INIT INFO
# Provides:          Lora gateway
# Required-Start:    $all
# Required-Stop:     
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Manage my cool stuff
### END INIT INFO

PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/opt/bin

. /lib/init/vars.sh
. /lib/lsb/init-functions
# If you need to source some other scripts, do it here

case "$1" in
  start)
    log_begin_msg "Starting the lora gateway"
    mv /var/log/lora_gateway /var/log/lora_gateway.bkp
    /home/pi/dual_chan_pkt_fwd/lora_gw_startup.sh&
    log_end_msg $?
    exit 0
    ;;
  stop)
    log_begin_msg "Stopping the lora gateway"

    kill `ps -ef|grep dual_chan_pkt|grep -v grep| awk '{print $2}'`

    log_end_msg $?
    exit 0
    ;;
  *)
    echo "Usage: /etc/init.d/<your script> {start|stop}"
    exit 1
    ;;
esac
