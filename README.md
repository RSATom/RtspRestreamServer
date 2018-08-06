# RtspRestreamServer
RTSP restream server based on Gstreamer RTSP Server

## Demo

To try use following commands:

## Build

* `sudo apt update`
* `sudo apt install build-essential git cmake libspdlog-dev libgstrtspserver-1.0-dev libgstreamer1.0-dev`
* `git clone https://github.com/RSATom/RtspRestreamServer.git`
* `cd RtspRestreamServer && mkdir build && cd build && cmake .. && make -j4 && cd ..`

## Run

* `sudo apt install gstreamer1.0-rtsp gstreamer1.0-plugins-base gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-x gstreamer1.0-tools gstreamer1.0-plugins-base-apps`
* Record side:
`gst-launch-1.0 videotestsrc ! x264enc ! rtspclientsink location=rtsp://localhost:8001/test`
* Play side:
`vlc rtsp://localhost:8001/test`
