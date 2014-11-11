package = "try"
version = "scm-0"

source = {
  url = "https://github.com/moteus/lua-try/archive/master.zip",
  dir = "lua-try-master",
}

description = {
  summary    = "Simple exception support based on LuaSocket",
  homepage   = "https://github.com/moteus/lua-try",
  license    = "MIT/X11",
  maintainer = "Alexey Melnichuk",
  detailed   = [[
  ]],
}

dependencies = {
  "lua >= 5.1, < 5.4"
}

build = {
  copy_directories = {'test'},

  type = "builtin",

  modules = {
    try = {
      sources = { "src/try.c" },
    },
    ["try.co"] =  "src/lua/try/co.lua",
  }
}
