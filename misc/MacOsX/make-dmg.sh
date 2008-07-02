#!/bin/sh

# Create a read-only disk image of the contents of a folder
#
# Usage: make-diskimage <image_file> <src_folder> <volume_name>

DMG_DIRNAME=`dirname $1`
DMG_DIR=`cd $DMG_DIRNAME > /dev/null; pwd`
DMG_NAME=`basename $1`
DMG_TEMP_NAME=${DMG_DIR}/rw.${DMG_NAME}
SRC_FOLDER=`cd $2 > /dev/null; pwd`
VOLUME_NAME=$3


# Create the image
echo "Creating disk image..."
rm -f "${DMG_TEMP_NAME}"
hdiutil create -srcfolder "${SRC_FOLDER}" -volname "${VOLUME_NAME}" -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDRW "${DMG_TEMP_NAME}"

# mount it
echo "Mounting disk image..."
MOUNT_DIR="/Volumes/${VOLUME_NAME}"
DEV_NAME=`hdiutil attach -readwrite -noverify -noautoopen "${DMG_TEMP_NAME}" | egrep '^/dev/' | sed 1q | awk '{print $1}'`


# make sure it's not world writeable
echo "Fixing permissions..."
chmod -Rf go-w "${MOUNT_DIR}" || true

# unmount
echo "Unmounting disk image..."
hdiutil detach "${DEV_NAME}"

# compress image
echo "Compressing disk image..."
hdiutil convert "${DMG_TEMP_NAME}" -format UDBZ -o "${DMG_DIR}/${DMG_NAME}"
rm -f "${DMG_TEMP_NAME}"

# adding Mozart license
echo "Adding license file"
hdiutil unflatten "${DMG_DIR}/${DMG_NAME}"
/Developer/Tools/Rez Carbon.r license.r -a -o "${DMG_DIR}/${DMG_NAME}"
hdiutil flatten "${DMG_DIR}/${DMG_NAME}"

echo "Disk image done"
exit 0
