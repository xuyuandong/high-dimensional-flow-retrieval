#!/usr/bin/env python

import os, sys, time
import smtplib, mimetypes
from email.MIMEText import MIMEText
from email.MIMEImage import MIMEImage
from email.MIMEMultipart import MIMEMultipart

class MailBox(object):
  def __init__(self):
    self.mail_user = ''
    self.mail_pass = ''
    self.mail_from = ''
    self.message = MIMEMultipart()

  def set_head(self, subject, mail_list):
    self.mail_to = mail_list
    self.message['From'] = self.mail_from
    self.message['To'] = ','.join(mail_list)
    self.message['Subject'] = subject

  def set_content(self, content):
    self.message.attach(MIMEText(content, 'html', 'utf-8'))

  def set_attachment(self, filename):
    content = MIMEText(open(filename).read())
    content["Content-Disposition"] = 'attachment;filename="%s"' %filename
    self.message.attach(content)

  def set_image(self, filename, imgname):
    fp = open(filename, "rb")
    image = MIMEImage(fp.read())
    fp.close()
    image.add_header("Content-ID", imgname)
    self.message.attach(image)

  def send(self):
    smtp = smtplib.SMTP()
    smtp.connect("mail.staff.sina.com.cn")
    smtp.login(self.mail_user, self.mail_pass)
    smtp.sendmail(self.mail_from, self.mail_to, self.message.as_string())
    smtp.quit()
