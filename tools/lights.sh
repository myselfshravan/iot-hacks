#!/bin/sh
# Monitor LED strip (driven by Dexi) from the terminal.
#   tools/lights.sh                    status
#   tools/lights.sh screen | on        follow the monitor (HyperHDR)
#   tools/lights.sh off                everything dark
#   tools/lights.sh rainbow|chase|sparkle|candle|solid|mood [rrggbb]   effect (+ colour); stops following the screen
#   tools/lights.sh bri 0-255          brightness of effects
#   tools/lights.sh start              open HyperHDR and follow the screen
#   tools/lights.sh stop               quit HyperHDR (saves ~30% CPU); strip falls back to its effect
#   tools/lights.sh restart            restart HyperHDR (sync stuck)
HOST=${DEXI:-dexi.local}
get() { curl -s -m 10 "http://$HOST/light$1"; echo; }
case "$1" in
  "") get "" ;;
  on|screen) get "?sync=on&mode=mood" ;;
  off) get "?mode=off" ;;
  bri) get "?bri=$2" ;;
  start)
    open -a hyperhdr; get "?sync=on&mode=mood" ;;
  stop)
    pkill -f hyperhdr.app; while pgrep -qf hyperhdr.app; do sleep 1; done; echo "HyperHDR stopped" ;;
  restart)
    pkill -f hyperhdr.app; sleep 3; open -a hyperhdr; sleep 10; get "" ;;
  mood|rainbow|solid|chase|sparkle|candle)
    get "?sync=off&mode=$1${2:+&color=$2}" ;;
  *) sed -n '2,10p' "$0"; exit 1 ;;
esac
