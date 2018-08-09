export ARCH=arm64 && export SUBARCH=arm64

export CROSS_COMPILE=/home/andrea/aarch64-linux-android-4.9/bin/aarch64-linux-android-

make lineageos_ether_defconfig

make -j4
