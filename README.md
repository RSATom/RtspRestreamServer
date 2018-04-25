# RtspRestreamServer
RTSP restream server based on Gstreamer RTSP Server

## Demo

Demo available at `rtsp://restream-demo.northeurope.cloudapp.azure.com:8001/`

To try use following commands:
* Record side:  
`gst-launch-1.0 videotestsrc ! x264enc ! rtspclientsink location=rtsp://restream-demo.northeurope.cloudapp.azure.com:8001/test`
* Play side:  
`vlc rtsp://restream-demo.northeurope.cloudapp.azure.com:8001/test`

## Build

* `sudo add-apt-repository ppa:rsatom/gst-rtsp-server-1.0`
* `sudo apt update`
* `sudo apt install build-essential git cmake libspdlog-dev libgstrtspserver-1.0-dev libgstreamer1.0-dev`
* `git clone https://github.com/RSATom/RtspRestreamServer.git`
* `cd RtspRestreamServer && mkdir build && cd build && cmake .. && make -j4 && cd ..`
