#!/usr/bin/python

import os, sys, time

ROOT = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, ROOT)

from mailbox import MailBox


if __name__ == "__main__" :
  title = sys.argv[1]
  message = ""
  for lines in sys.stdin :
	message += lines + "<br>"
  mailbox = MailBox()
  daystr = time.strftime("%Y-%m-%d %H:%M:%S")
  mailbox.set_head('[SYSTEM] %s[%s]'%(title, daystr), ["xxx@xxx.com"])])
  mailbox.set_content(message)
  mailbox.send()
