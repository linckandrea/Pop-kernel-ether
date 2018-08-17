export ARCH=arm64 && export SUBARCH=arm64

export CROSS_COMPILE=/home/andrea/a/bin/aarch64-linux-android-

make lineageos_ether_defconfig

make -j4
