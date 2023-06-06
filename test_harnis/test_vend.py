#! /usr/bin/env python

import random
import string
import json
import re
import sys
from Adafruit_IO import Client, Feed, AdafruitIOError, errors

ADAFRUIT_IO_USERNAME = ""
ADAFRUIT_IO_KEY      = ""

BEERS = [
    'Dog Fish 60 Minute',
    'Bells two hearted',
    'Freshly Squeezed',
    'All Day IPA',
    'Guiness',
    'Ballast Point Sculpin IPA'
]

# Get the adafruit creds from the arduino code. 
with open('../src/secrets.h', 'r') as reader:
    d = reader.read()
    ADAFRUIT_IO_USERNAME = re.search('AIO_USERNAME\W+"(\w+)"', d).groups(0)[0]
    ADAFRUIT_IO_KEY      = re.search('AIO_KEY\W+"(\w+)"', d).groups(0)[0]


def get_id():
    return ''.join(random.choice(string.ascii_letters) for i in range(4))



def get_feed(feed_name, aio):
    try: # if we have a feed, then fetch it.
        return aio.feeds(feed_name)
    except errors.RequestError: # else create it
        return aio.create_feed(Feed(feed_name))

def vend():
    aio = Client(ADAFRUIT_IO_USERNAME, ADAFRUIT_IO_KEY)
    aio_cmd    = get_feed('cmd', aio)
    aio_cmd_rx = get_feed('cmd-rx', aio)



    d = json.loads('{"cmd":"vend","id":"1","args":[1, "Charles", "Ballast Point Grapefruit Sculpin"]}')
    d['id'] = get_id()
    d['args'][0] = 1 #random.randint(0,5)  # Assign the slot
    d['args'][2] = BEERS[d['args'][0]]
    payload = json.dumps(d)
    
    
    if len(payload) > 106:
        print(f"Cant send payload,  its too big.  Max len = 100,  current len = {len(payload)}")
        return

    print(f"Sending payload {payload} Len: {len(payload)}")

    r = aio.send(aio_cmd.name, payload)
    print("data sent")

    


if __name__ == "__main__":
    vend()