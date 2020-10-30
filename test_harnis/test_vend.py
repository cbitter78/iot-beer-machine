#! /usr/bin/env python

import random
import json
import re
import sys
from Adafruit_IO import Client, Feed

ADAFRUIT_IO_USERNAME = ""
ADAFRUIT_IO_KEY = ""

# Get the adafruit creds from the arduino code. 
with open('../iot-beer-machine/secrets.h', 'r') as reader:
    d = reader.read()
    ADAFRUIT_IO_USERNAME = re.search('AIO_USERNAME\W+"(\w+)"', d).groups(0)[0]
    ADAFRUIT_IO_KEY      = re.search('AIO_KEY\W+"(\w+)"', d).groups(0)[0]

aio = Client(ADAFRUIT_IO_USERNAME, ADAFRUIT_IO_KEY)
aio_cmd = Feed
try: # if we have a feed, then fetch it.
    aio_cmd = aio.feeds('iot-beer-command')
except RequestError: # else create it
    aio_cmd = aio.create_feed(Feed(name="iot-beer-command"))

d = json.loads('{"name":"vend","id":"1","slot":0,"args":["Charles"]}')
d['slot'] = int(sys.argv[1])
d['id'] = F"{random.randint(10, 99)}"
payload = json.dumps(d)
print(F"Sending payload {payload}")

r = aio.send(aio_cmd.name, payload)

