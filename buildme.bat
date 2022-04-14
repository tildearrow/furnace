@echo off
git pull
git submodule update --init --recursive
cmake -Bbuild
cmake --build build --target ALL_BUILD --config Release --parallel 12
copy build\Release\furnace.exe \Apps\Audio\Furnace\
