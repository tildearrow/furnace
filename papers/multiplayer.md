# multiplayer protocol

this is a concept! it has not been implemented yet!

the Furnace protocol is described here.

# information

all numbers are little-endian.

the following fields may be found in "size":
- `f` indicates a floating point number.
- `STR` is a UTF-8 zero-terminated string.
- `CFG` is the same as STR, but contains a config.
- `???` is an array of variable size.
- `S??` is an array of `STR`s.
- `1??` is an array of bytes.
- `2??` is an array of shorts.
- `4??` is an array of ints.

two player IDs are reserved:
- 0: system
- 1: host (console)

two usernames are reserved:
- SYSTEM
- HOST

some characters are not allowed in usernames: 0x00-0x1f, `@`, 0x7f-0x9f, 0xd800-0xdfff, 0xfeff, 0xfffe and 0xffff.

# header
```
size | description
-----|------------------------------------
  3  | "fur" header
  1  | packet type
  4  | sequence number
```
the sequence number always starts at 0.

# client to server packets (init)

## 0x00: keep-alive

this packet keeps a connection alive.
the server shall respond with a packet of type 0x00 (keep-alive).
if the client does not receive any packets during 30 seconds, it will disconnect from the server.
likewise, if the server does not receive any packets during 30 seconds, it will disconnect the client.

## 0x01: start connection
```
size | description
-----|------------------------------------
  1  | reason
     | - 0: information
     | - 1: join
  3  | padding
  4  | client version
 STR | host name (may be blank)
```
after sending, you will receive a packet of type 0x01 (information), 0x02 (disconnect) or 0x03 (authenticate).

## 0x02: disconnect
```
size | description
-----|------------------------------------
 STR | reason
```
## 0x03: auth response
```
size | description
-----|------------------------------------
  1  | type
     | - 0: open
     | - 1: password
     | - 2: token
 --- | **open response**
 STR | username
 --- | **password response**
  1  | password type
     | - 0: plain text
     | - 1: SHA-512
 STR | account name
 ??? | password
 --- | **token response**
 STR | token
```
# server to client packets (init)

## 0x00: keep-alive

this packet keeps a connection alive. it is a response to a client's keep-alive packet.

## 0x01: information
```
size | description
-----|------------------------------------
  4  | server version
  2  | online players
     | - if it is 65535, this information is concealed.
  2  | maximum players
     | - 0 means unlimited.
 STR | server version (string)
 STR | server name
 STR | server description
 STR | project name
```
the client may send a 0x00 (keep-alive) packet after receiving this one within 5 seconds.
connection is then closed.

## 0x02: disconnect
```
size | description
-----|------------------------------------
 STR | reason
```
after being sent, the connection is closed.

## 0x03: authenticate
```
size | description
-----|------------------------------------
  1  | authentication type
     | - 0: open
     | - 1: password
     | - 2: token
```
## 0x04: authentication success
```
size | description
-----|------------------------------------
  4  | player ID
 STR | username
 CFG | properties
```
# client to server packets (session)

## 0x10: request project

the client may only send this once every minute.

## 0x11: participate
```
size | description
-----|------------------------------------
  1  | status
     | - 0: spectate
     | - 1: join
```
## 0x12: send chat message
```
size | description
-----|------------------------------------
 STR | message
```
## 0x13: send command
```
size | description
-----|------------------------------------
 STR | command
  2  | number of arguments
 S?? | arguments
```
## 0x14: get player list

no other information required.

## 0x15: project submission request

no other information required

## 0x16: project submission information
```
size | description
-----|------------------------------------
  4  | project size
 32  | SHA-256 sum of project
 STR | project name
```
this is followed by several 0x17 (project data) packets representing a Furnace song. see [format.md](format.md) for more information.

## 0x17: project submission data
```
size | description
-----|------------------------------------
  4  | offset
  4  | length
 ??? | data...
```
the client will send a packet with project size as offset and 0 as length to indicate end of data.
the server subsequently loads the project.

# server to client packets (session)

## 0x10: project information
```
size | description
-----|------------------------------------
  4  | project size
 32  | SHA-256 sum of project
 STR | project name
```
this is followed by several 0x13 (project data) packets representing a Furnace song. see [format.md](format.md) for more information.

## 0x11: project data
```
size | description
-----|------------------------------------
  4  | offset
  4  | length
 ??? | data...
```
the server will send a packet with project size as offset and 0 as length to indicate end of data.
the client subsequently loads the project.

## 0x12: participate status
```
size | description
-----|------------------------------------
  1  | status
     | - 0: denied
     | - 1: allowed
```
## 0x13: message
```
size | description
-----|------------------------------------
  4  | player ID
  8  | time (seconds)
  4  | time (nanoseconds)
  4  | message ID
  1  | type
     | - 0: chat, public
     | - 1: chat, private
     | - 2: notification, info
     | - 3: notification, warning
     | - 4: notification, urgent
 STR | message
```
## 0x14: system message
```
size | description
-----|------------------------------------
 STR | message
```
## 0x15: chat message edited
```
size | description
-----|------------------------------------
  4  | message ID
 STR | message
     | - an empty message means deleted.
```
## 0x16: player list
```
size | description
-----|------------------------------------
  2  | number of players
 --- | **player entry** (×players)
  4  | ID
  2  | latency (ms)
  1  | participating?
     | - 0: no
     | - 1: yes
  1  | status
     | - 0: normal
     | - 1: away
     | - 2: busy
 STR | name
 STR | IP address
     | - if empty, then server is not disclosing IP addresses.
```
this is sent after receiving 0x14 (get player list).

## 0x17: project submission request status
```
size | description
-----|------------------------------------
  1  | status
     | - 0: denied
     | - 1: allowed
```
this is sent after a project submission request is accepted.
if the status is 1, the client shall submit a project.

## 0x18: project submission complete
```
size | description
-----|------------------------------------
  1  | status
     | - 0: error
     | - 1: success
 STR | additional information
```
## 0x19: player joined
```
size | description
-----|------------------------------------
  4  | ID
 STR | name
```
## 0x1a: player left
```
size | description
-----|------------------------------------
  4  | ID
```
# client to server packets (project)

## 0x20: request orders

## 0x21: request instrument

## 0x22: request wavetable

## 0x23: request sample

## 0x24: request patterns

## 0x25: request sub-song

## 0x26: request song info

## 0x27: request asset list

## 0x28: request patchbay

## 0x29: request grooves

## 0x30: alter orders
```
size | description
-----|------------------------------------
  4  | transaction ID
```
## 0x31: alter instrument

## 0x32: alter wavetable

## 0x33: alter sample

## 0x34: alter pattern

## 0x35: alter sub-song

## 0x36: alter song info

## 0x37: alter asset list

## 0x38: alter patchbay

## 0x39: alter grooves

## 0x3a: alter chips

## 0x3b: alter chip settings

# server to client packets (project)

## 0x20: orders

## 0x21: instrument

## 0x22: wavetable

## 0x23: sample

## 0x24: pattern

## 0x25: sub-song

## 0x26: song info

## 0x27: asset list

## 0x28: patchbay

## 0x29: grooves

## 0x30: transaction response
```
size | description
-----|------------------------------------
  1  | status
     | - 0: error
     | - 1: success
     | - 2: success but request again
 STR | additional information
```
# client to server packets (interact)

## 0x40: engine command

## 0x41: playback

# server to client packets (interact)

## 0x40: engine command

## 0x41: playback

# client to server packets (extension)

# server to client packets (extension)
