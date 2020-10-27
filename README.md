# Lua AMQP client

![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)


> Lua shared memory queue for producers-consumers

A Lua shared memory queue desgined for multiple producers-consumers on different processes

![](header.png)

## Installation

```sh
luarocks install lua-shared-queue
```

## Build from source

It can be built using luarocks builtin method or with Makefile.

Using make - just run make and copy the library (bin/shared_queue.so) to the wanted location:

```sh
make
```

## Quick Start

Below is a small snippet that shows a simple flow of produce-consumer

producer - first process:
``` lua
local shared_queue = require('shared_queue').init(true) -- it should be called with 'true' only once, for the init process of the shared objects.

shared_queue:publish("msg1")
shared_queue:publish("msg2")
```

consumer - second process:
``` lua
local shared_queue = require('shared_queue').init()

while true do 
  print("message is " .. shared_queue:consume())
end
```

consumer output:
``` lua
message is msg1
message is msg2
```

## Contributing

1. Fork it (<https://github.com/galtet/lua-shared-queue/fork>)
2. Create your feature branch (`git checkout -b feature/fooBar`)
3. Commit your changes (`git commit -am 'Add some fooBar'`)
4. Push to the branch (`git push origin feature/fooBar`)
5. Create a new Pull Request
