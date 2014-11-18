#!/bin/bash

cd $(dirname `ls -ls $0 | awk '{print $NF;}'`)/..

WK_DIR=`pwd`
EXE_NAME=service
PID_FILE=$WK_DIR/$EXE_NAME.pid
MAIL_SCRIPT=$WK_DIR/script/mailto.py
PYTHON=`which python`
SLEEP_TIME=10

# 查看当前程序是否存活，如果存活返回1，否则返回0
function _status()
{
  if [ -f $PID_FILE ]; then
    RETURN_CODE=1
    for PID in `cat $PID_FILE`; do
      RUN=`ps -eo pid,cmd | grep $PID | grep $EXE_NAME`
      if [ "$RUN" == "" ]; then
        RETURN_CODE=0
      fi
    done
    return $RETURN_CODE
  else
    return 0
  fi
}

# 查看当前程序的error log 是否有更新，如果存在更新返回1，否则返回0
function _error()
{
  HAS_ERROR=0
  for ERROR_LOG in `find $WK_DIR/logs/ -name "$EXE_NAME*ERROR*" -type f` ; do
    HAS_ERROR=1
    cat $ERROR_LOG | $PYTHON $MAIL_SCRIPT error_log
    rm -rf $ERROR_LOG | true
  done
  return $HAS_ERROR
}

##
# main

_status
STATUS=`echo $?`
_error
ERROR_LOG=`echo $?`

if [ $STATUS -eq 0 ]; then
  LIVE_STATUS=death
else
  LIVE_STATUS=running
fi
if [ $ERROR_LOG -eq 1 ]; then
  ERROR_STATUS=error
else
  ERROR_STATUS=common
fi
if [ $STATUS -eq 0 -o $ERROR_LOG -eq 1 ]; then
  echo "live status is $LIVE_STATUS, error status is $ERROR_STATUS\nI will restart the $EXE_NAME" | $PYTHON $MAIL_SCRIPT restart_report
  sh $WK_DIR/script/startup.sh restart
fi
