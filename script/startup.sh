#!/bin/bash

cd $(dirname `ls -ls $0 | awk '{print $NF;}'`)/..
WK_DIR=`pwd`

EXE_NAME=service

PROCESS_NUM=1

function _start() 
{
  cd $WK_DIR/bin
  for ((i = 0; i < $PROCESS_NUM; ++i)) ; do
    nohup ./$EXE_NAME --flagfile=../conf/$EXE_NAME.conf & echo $! >>../$EXE_NAME.pid &
  done
  echo "$EXE_NAME start on pid of `awk 'BEGIN {line = ""}{if(length(line) == 0) {line = $0} else {line = line","$0}} END {print line}' ../$EXE_NAME.pid`"
}

function _stop()
{
  if [ -f $WK_DIR/$EXE_NAME.pid ]; then
    for PID in `cat ${WK_DIR}/${EXE_NAME}.pid`; do
      RUN=`ps -eo pid,cmd | grep $PID | grep $EXE_NAME`
      if [ "$RUN" != "" ]; then
        echo "$PID is running!"
        kill $PID
        echo "$PID has been killed!"
      fi
    done
    rm -rf $WK_DIR/$EXE_NAME.pid
  else
    echo "pid file is not exist!"
  fi
}

function _restart() 
{
  _stop
  _start
}

function _show_useage()
{
  echo "useage: $0 {start|stop|restart}"
}

##
# main

case $1 in 
  start|stop|restart)
  _${1}
  ;;
  *)
  _show_useage
  ;;
esac

