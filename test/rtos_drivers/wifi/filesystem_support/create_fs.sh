set -x

# discern repository root
REPO_ROOT=`git rev-parse --show-toplevel`

# Copy files into filesystem
mkdir -p fat_mnt
mkdir fat_mnt/firmware
mkdir fat_mnt/wifi
cp $REPO_ROOT/modules/drivers/wifi/sl_wf200/thirdparty/wfx-firmware/wfm_wf200_C0.sec fat_mnt/firmware/wf200.sec

# Collect network information
if [ ! -f networks.dat ]; then
    ./wifi_profile.py
fi
cp networks.dat fat_mnt/wifi

# create filesystem
${REPO_ROOT}/dist_host/fatfs_mkimage -o fat.fs -i fat_mnt

# Cleanup
rm -rf fat_mnt
