export ARCH=arm64 && export SUBARCH=arm64

export CROSS_COMPILE=/home/andrea/aarch64-linux-android/bin/aarch64-opt-linux-android-

make clean && make mrproper

make lineageos_ether_defconfig

make -j4
