export ARCH=arm64 && export SUBARCH=arm64

make clean && make mrproper

export CROSS_COMPILE=/home/andrea/aarch64-linux-gnu/bin/aarch64-linux-gnu-

make lineageos_ether_defconfig

make -j4
