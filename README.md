lua-try
=======
[![Licence](http://img.shields.io/badge/Licence-MIT-brightgreen.svg)](LICENSE)
[![Build Status](https://travis-ci.org/moteus/lua-try.svg?branch=master)](https://travis-ci.org/moteus/lua-try)
[![Coverage Status](https://coveralls.io/repos/moteus/lua-try/badge.png?branch=master)](https://coveralls.io/r/moteus/lua-try?branch=master)

Simple exception support based on LuaSocket

You can read [this article](http://lua-users.org/wiki/FinalizedExceptions).

##Usage
```Lua
local try = require "try"

local protected_fun = try.protect(function(fname)
  local f -- local file handle
  
  -- define finalizer to close file
  local assert = try.new(function()
    if f then f:close() end
  end)

  -- now if `assert` raise error then
  -- `try` calls finalizer and protected function
  -- returns nil and error object

  f = assert(io.open(fname))

  assert(do_some_function(f))

  f:close()
end)

```
