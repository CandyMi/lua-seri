# lua-seri

  Lua serialization and deserialization library written by C.

## Usage

  `local xrpack = require "lxrpack"`

### 1. pack and unpack

  `xrpack.pack` and `xrpack.unpack` can pass data between different threads, the `pack` method is used to create memory and pack data, and the `unpack` method is used to unpack data and free memory.
  
### 2. encode and decode

  `xrpack.encode` and `xrpack.decode` can pack Lua data structures into characters for network transmission.
  

## Note

  `encode` and `decode` may crash between multiple threads, `pack` and `unpack` will inevitably cause `double free` if used between multiple threads.
  
  If you want to use them safely, remember the following rules:

  * `Thread-1` uses `pack` to generate `data1`, then `Thread-2` can only be allowed to call `unpack(data1)` once to get the actual data.

  * `encode` and `decode` must appear in pairs; the sender calls `encode` to generate, and the receiver calls `decode` to decode.

  For more usage, please leave a message to communicate with the author.
