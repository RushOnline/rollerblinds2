# KEYES IR remote

```
lg, nbits: 32

'^'  0x00FF629D
'<'  0x00FF22DD
'OK' 0x00FF02FD
'>'  0x00FFC23D
'V'  0x00FFA857
'1'  0x00FF6897
'2'  0x00FF9867
'3'  0x00FFB04F
'4'  0x00FF30CF
'5'  0x00FF18E7
'6'  0x00FF7A85
'7'  0x00FF10EF
'8'  0x00FF38C7
'9'  0x00FF5AA5
'0'  0x00FF4AB5
'*'  0x00FF42BD
'#'  0x00FF52AD
```


  - platform: remote_receiver
    id: remote_up
    lg:
      data: 0x00FF629D
      nbits: 32

  - platform: remote_receiver
    id: remote_down
    lg:
      data: 0x00FFA857
      nbits: 32

  - platform: remote_receiver
    id: remote_left
    lg:
      data: 0x00FF22DD
      nbits: 32

  - platform: remote_receiver
    id: remote_ok
    lg:
      data: 0x00FF02FD
      nbits: 32

  - platform: remote_receiver
    id: remote_right
    lg:
      data: 0x00FFC23D
      nbits: 32

  - platform: remote_receiver
    id: remote_1
    lg:
      data: 0x00FF6897
      nbits: 32

  - platform: remote_receiver
    id: remote_2
    lg:
      data: 0x00FF9867
      nbits: 32

  - platform: remote_receiver
    id: remote_3
    lg:
      data: 0x00FFB04F
      nbits: 32

  - platform: remote_receiver
    id: remote_4
    lg:
      data: 0x00FF30CF
      nbits: 32

  - platform: remote_receiver
    id: remote_5
    lg:
      data: 0x00FF18E7
      nbits: 32

  - platform: remote_receiver
    id: remote_6
    lg:
      data: 0x00FF7A85
      nbits: 32

  - platform: remote_receiver
    id: remote_7
    lg:
      data: 0x00FF10EF
      nbits: 32

  - platform: remote_receiver
    id: remote_8
    lg:
      data: 0x00FF38C7
      nbits: 32

  - platform: remote_receiver
    id: remote_9
    lg:
      data: 0x00FF5AA5
      nbits: 32

  - platform: remote_receiver
    id: remote_0
    lg:
      data: 0x00FF4AB5
      nbits: 32

  - platform: remote_receiver
    id: remote_asterisk
    lg:
      data: 0x00FF42BD
      nbits: 32

  - platform: remote_receiver
    id: remote_hash
    lg:
      data: 0x00FF52AD
      nbits: 32

