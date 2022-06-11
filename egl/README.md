# practice-egl

https://qiita.com/y-tsutsu/items/1e88212b8532fc693c3c

https://forums.raspberrypi.com/viewtopic.php?t=281296
https://daily-tech.hatenablog.com/entry/2016/05/25/025616

```
sudo apt install libgles2-mesa-dev libegl1-mesa-dev xorg-dev
g++ main.cpp -o main -lGLESv2 -lEGL -lX11
```

v4l2

```
sudo modprobe v4l2loopback exclusive_caps=1
gst-launch-1.0 -v videotestsrc ! videoconvert ! video/x-raw,format=YUY2 ! v4l2sink device=/dev/video0
sudo modprobe -r v4l2loopback
```

info

```
v4l2-compliance

Buffer ioctls (Input 0):
        test VIDIOC_REQBUFS/CREATE_BUFS/QUERYBUF: OK
        test VIDIOC_EXPBUF: OK
        test Requests: OK (Not Supported)

4l2-ctl -d /dev/video0 --list-formats
ioctl: VIDIOC_ENUM_FMT
        Type: Video Capture

        [0]: 'YUYV' (YUYV 4:2:2)
        [1]: 'MJPG' (Motion-JPEG, compressed)
```