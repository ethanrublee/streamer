#!/usr/bin/env python
import ecto
from ecto_opencv.highgui import VideoCapture, imshow, FPSDrawer
import mjpeg_server

video_cap = VideoCapture(video_device=0, width=640, height=480)
fps = FPSDrawer()
server = mjpeg_server.server(address='0.0.0.0', port='9090', nthreads=8, doc_root='./')
streamer1 = mjpeg_server.Streamer(server=server, path="/video",quality=60)

plasm = ecto.Plasm()
plasm.connect(video_cap['image'] >> fps['image'],
              fps['image'] >> streamer1['image'],
              )

if __name__ == '__main__':
    server.start()
    from ecto.opts import doit
    doit(plasm, description='Capture a video from the device and display it.')
    server.stop()
