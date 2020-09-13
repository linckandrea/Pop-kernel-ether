echo
echo "Setup"
echo
mkdir -p out
export ARCH=arm64
export SUBARCH=arm64
make O=out clean
make O=out mrproper

echo
echo "Issue Build Commands"
echo
export CROSS_COMPILE=/home/andrea/gcc10/arm64-gcc/bin/aarch64-elf-

echo
echo "Set DEFCONFIG"
echo 
make O=out lineageos_ether_defconfig

echo
echo "Build The Good Stuff"
echo 
make O=out -j$(nproc --all)
