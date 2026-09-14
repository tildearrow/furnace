# Building for Android

Assuming the Android Ndk is installed at location `/path/to/toolchain`, building
libsndfile for Android (arm-linux-androideabi) should be as simple as:
```
autoreconf -vif
export ANDROID_TOOLCHAIN_HOME=/path/to/android/toolchain
./Scripts/android-configure.sh
make
```
The `Scripts/android-configure.sh` contains four of variables; `ANDROID_NDK_VER`,
`ANDROID_GCC_VER`, `ANDROID_API_VER` and `ANDROID_TARGET` that can be overridden
by setting them before the script is run.

Since I (erikd), do almost zero Android development, I am happy accept patches
for this documentation and script to improve its utility for real Android
developers.

---

## Using CMake

(Tested on Linux)

For convenience, export the following variables:

```
export ANDROID_ABI=arm64-v8a
export ANDROID_PLATFORM_API_LEVEL=29
export NDK_ROOT=/path/to/android/ndk
```

Set `ANDROID_ABI`,  `ANDROID_PLATFORM_API_LEVEL`  according to your target system. Now cd into the libsndfile root directory, and run

```
cmake -S . -B build  -DCMAKE_TOOLCHAIN_FILE=$NDK_ROOT/build/cmake/android.toolchain.cmake -DANDROID_ABI=$ANDROID_ABI -DANDROID_PLATFORM=$ANDROID_PLATFORM_API_LEVEL
```

cd into `build` and run make

```
cd build
make [-j <number of parallel jobs>]
```

This will build libsndfile for android.
