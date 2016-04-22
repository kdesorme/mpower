#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""
 Copyright ⓒ 2014 CNRS/LAAS

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.

 THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
"""


import sys
import json
import pycurl
import time

class mPower:
    def __init__(self, host):
        self.contents = ''
        self.host = host
        self.handle = pycurl.Curl()
        self.handle.setopt(self.handle.AUTOREFERER, 1)
        self.handle.setopt(self.handle.FOLLOWLOCATION, 1)
        self.handle.setopt(self.handle.COOKIEFILE, "")
        self.handle.setopt(self.handle.WRITEFUNCTION, self.body_callback)

    def __del__(self):
        self.handle.close()
        print("bye\n")

    def body_callback(self, buf):
        self.contents = self.contents + buf

    def PrintMemory(self):
        print self.contents

    def Login(self, user, password):
        self.contents = ''
        self.handle.setopt(self.handle.URL,
                           "http://" + self.host + "/index.cgi")
        self.handle.perform()
        self.handle.setopt(self.handle.POSTFIELDS,
                           "username="+user +"&password="+password)
        self.handle.setopt(self.handle.URL,
                           "http://" + self.host + "/login.cgi")
        self.handle.perform()
        if (self.handle.getinfo(self.handle.RESPONSE_CODE) != 200):
            print("login page error\n")
        if (str.find(self.contents, "Invalid credentials.") > 0):
            print("Invalid credentials\n")
            return -1
        return 0
 
    def SetOutput(self, output, value):
        self.contents = ''
        self.handle.setopt(self.handle.URL,
                           "http://" + self.host + "/sensors/%d/" % output)
        self.handle.setopt(self.handle.POSTFIELDS, "output=%d" % value)
        self.handle.setopt(self.handle.CUSTOMREQUEST, "PUT")
        self.handle.perform()
        decoded = json.loads(self.contents);
        if (decoded['status'] != "success"):
            print decoded['status']
            return -1
        return 0

    def QueryOutputs(self):
        self.contents = ''
        self.handle.setopt(self.handle.URL,
                           "http://"+self.host+"/mfi/sensors.cgi")
        self.handle.setopt(self.handle.CUSTOMREQUEST, "GET")
        self.handle.setopt(self.handle.HTTPGET, 1)
        self.handle.perform()
        decoded = json.loads(self.contents);
        if (decoded['status'] != "success"):
            return -1
        self.sensors = decoded['sensors']
        return 0

    def PrintOutputs(self):
        for sensor in self.sensors:
            print "%d: %f W %f A %f V %f %d" % (
                sensor['port'], sensor['power'], sensor['current'], 
                sensor['voltage'], sensor['powerfactor'], sensor['relay'])

t = mPower("mpower")
if (t.Login("ubnt", "ubnt") != 0):
    exit(1)
print("login ok")
t.SetOutput(3, 1)
time.sleep(5)
t.QueryOutputs()
t.PrintOutputs()
t.SetOutput(3, 0)
