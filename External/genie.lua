local action = _ACTION or ""

-- Default library prefixes.
EXTERNAL_PROJECT_PREFIX = "External_"
EXTERNAL_PROJECT_KIND = "StaticLib"

if _OPTIONS[ "toolchain" ] == "android-gcc-arm" then
	dofile ("gpg-cpp-sdk_genie.lua")
end

