local lunit = lunit

local RUN = lunit and function()end or function ()
  local res = lunit.run()
  if res.errors + res.failed > 0 then
    os.exit(-1)
  end
  return os.exit(0)
end

lunit = require "lunit"

local TEST_CASE  = assert(lunit.TEST_CASE)
local skip       = lunit.skip or function() end

local try   = require "try"
local coroutine = require "coroutine"

local function nreturn(...)
  return select("#", ...), ...
end

local pcall, error = pcall, error

local IS_LUA_51 = (_VERSION == "Lua 5.1")

local ENABLE = true

local _ENV = TEST_CASE'try.basic' if ENABLE then

local it = setmetatable(_ENV or _M, {__call = function(self, describe, fn)
  self["test " .. describe] = fn
end})

it("return value", function()
  local fn = assert_function(try.protect(function()
    return 1, nil, 3, nil
  end))
  local n, a, b, c, d = nreturn(fn())
  assert_equal(4, n)
  assert_equal(1, a)
  assert_nil(b)
  assert_equal(3, c)
  assert_nil(d)
end)

it("call finalizer", function()
  local n = 0
  local EVALUE = 'some value'
  local check = try.new(function() n = n + 1 end)

  local fn = assert_function(try.protect(function()
    check(nil, EVALUE)
  end))
  
  local _, b = assert_nil(fn())
  assert_equal(EVALUE, b)
  assert_equal(1, n)
end)

it("call finalizer and return nil", function()
  local n = 0
  local EVALUE = nil
  local check = try.new(function() n = n + 1 end)

  local fn = assert_function(try.protect(function()
    check(nil, EVALUE)
  end))
  
  local _, b = assert_nil(fn())
  assert_equal(EVALUE, b)
  assert_equal(1, n)
end)

it("raise error", function()
  local n = 0
  local EVALUE = "some value"
  local check = try.new(function() n = n + 1 end)

  local fn = assert_function(try.protect(function()
    error(EVALUE)
  end))

  local ok, err = pcall(fn)
  assert_false(ok)
  assert_match(EVALUE, err)
  assert_equal(0, n)
end)

it("raise error and return table", function()
  local n = 0
  local EVALUE = {"some value"}
  local check = try.new(function() n = n + 1 end)

  local fn = assert_function(try.protect(function()
    error(EVALUE)
  end))

  local ok, err = pcall(fn)
  assert_false(ok)
  assert_equal(EVALUE, err)
  assert_equal(0, n)
end)

end

local _ENV = TEST_CASE'try.assert' if ENABLE then

local it = setmetatable(_ENV or _M, {__call = function(self, describe, fn)
  self["test " .. describe] = fn
end})

it("call assert", function()
  local EVALUE = 'some value'

  local fn = assert_function(try.protect(function()
    try.assert(nil, EVALUE)
  end))
  
  local _, b = assert_nil(fn())
  assert_equal(EVALUE, b)
end)

it("call assert and return nil", function()
  local EVALUE = nil

  local fn = assert_function(try.protect(function()
    try.assert(nil, EVALUE)
  end))
  
  local _, b = assert_nil(fn())
  assert_equal(EVALUE, b)
end)

end

local _ENV = TEST_CASE'try.coro' if ENABLE then

local try = try

if IS_LUA_51 then try = require "try.co" end

local it = setmetatable(_ENV or _M, {__call = function(self, describe, fn)
  self["test " .. describe] = fn
end})

it("return value", function()
  local co = coroutine.create(try.protect(function(...)
    return ...
  end))

  local n, ok, a, b, c, d = nreturn(coroutine.resume(co, 1, nil, 3, nil))
  assert_equal(5, n)
  assert_true(ok)
  assert_equal(1, a)
  assert_nil(b)
  assert_equal(3, c)
  assert_nil(d)
end)

it("yeild inside protect", function()
  local co = coroutine.create(try.protect(function(...)
    return coroutine.yield(...)
  end))

  local n, ok, a, b, c, d = nreturn(coroutine.resume(co, 1, nil, 3, nil))
  assert_equal(5, n)
  assert_true(ok)
  assert_equal(1, a)
  assert_nil(b)
  assert_equal(3, c)
  assert_nil(d)
  
  assert_true(coroutine.resume(co))
end)

it("check inside coroutine", function()
  local n = 0
  local EVALUE = 'some value'
  local check = try.new(function() n = n + 1 end)

  local co = coroutine.create(try.protect(function(...)
    check(nil, EVALUE)
  end))

  local n, ok, a, b = nreturn(coroutine.resume(co, 1, nil, 3, nil))
  assert_equal(3, n)
  assert_true(ok)
  assert_nil(a)
  assert_equal(EVALUE, b)
end)

end

RUN()
