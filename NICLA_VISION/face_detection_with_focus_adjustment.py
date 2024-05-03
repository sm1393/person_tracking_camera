# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# QRCode Example
#
# This example shows the power of the OpenMV Cam to detect QR Codes
# using lens correction (see the qrcodes_with_lens_corr.py script for higher performance).

import sensor
import time
import image

sensor.reset()

# Sensor settings
sensor.set_contrast(3)
sensor.set_gainceiling(16)
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.HQVGA)
sensor.skip_frames(time=2000)
sensor.set_auto_gain(False)  # must turn this off to prevent image washout...
sensor.set_vflip(True)

face_cascade = image.HaarCascade("frontalface")

clock = time.clock()

referenceFaceArea = 12900

while True:
    clock.tick()
    img = sensor.snapshot()
    for code in img.find_qrcodes():
        img.draw_rectangle(code.rect(), color=(255, 0, 0))
        print(code.payload())

    objects = img.find_features(face_cascade, threshold=0.75, scale_factor=1.25)

    if len(objects)!=0:
        center_x,center_y= str(objects[0][0] + objects[0][2]//2), str(objects[0][1] + objects[0][3]//2)
        area_face=objects[0][2]*objects[0][3]

        print("Face Center=", center_x+','+center_y, "| Face area:",objects[0][2], "*", objects[0][3], "=" , area_face)

        if (area_face) > referenceFaceArea:
            sensor.set_framesize(sensor.QVGA)
        else:
            sensor.set_framesize(sensor.HQVGA)


    # Draw objects
    for r in objects:
        img.draw_rectangle(r)
