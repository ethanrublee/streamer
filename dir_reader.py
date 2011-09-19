#!/usr/bin/env python
import ecto
from ecto_opencv.highgui import imshow, ImageReader
import os
import mjpeg_server

#this will read all images on the user's Desktop
images = ImageReader(path=os.path.expanduser('~/Desktop'), loop=True)

server = mjpeg_server.server(address='0.0.0.0', port='9090', nthreads=8, doc_root='./')
streamer1 = mjpeg_server.Streamer(server=server, path="/stream_0")


plasm = ecto.Plasm()
plasm.connect(images['image'] >> (streamer1['image']))
if __name__ == '__main__':
    server.start()
    from ecto.opts import doit
    doit(plasm, description='Stream images from the user\'s Desktop to the browser using mjpeg.')
    server.stop()
