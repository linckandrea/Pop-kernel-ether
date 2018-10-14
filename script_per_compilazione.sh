export ARCH=arm64 && export SUBARCH=arm64
make clean && make mrproper
export CROSS_COMPILE=/home/andrea/aarch64-linux-android/bin/aarch64-opt-linux-android-
make lineageos_ether_defconfig
make -j4
