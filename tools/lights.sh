#!/bin/sh
# Monitor LED strip (driven by Dexi) from the terminal.
#   tools/lights.sh                    status
#   tools/lights.sh screen | on        follow the monitor (HyperHDR)
#   tools/lights.sh off                everything dark
#   tools/lights.sh rainbow|chase|sparkle|candle|solid|mood [rrggbb]   effect (+ colour); stops following the screen
#   tools/lights.sh bri 0-255          brightness of effects
#   tools/lights.sh restart            restart HyperHDR (sync stuck)
HOST=${DEXI:-dexi.local}
get() { curl -s -m 5 "http://$HOST/light$1"; echo; }
case "$1" in
  "") get "" ;;
  on|screen) get "?sync=on&mode=mood" ;;
  off) get "?mode=off" ;;
  bri) get "?bri=$2" ;;
  restart)
    pkill -f hyperhdr.app; sleep 3; open -a hyperhdr; sleep 10; get "" ;;
  mood|rainbow|solid|chase|sparkle|candle)
    get "?sync=off&mode=$1${2:+&color=$2}" ;;
  *) sed -n '2,8p' "$0"; exit 1 ;;
esac
