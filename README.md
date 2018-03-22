# RtspRestreamServer
RTSP restream server based on Gstreamer RTSP Server

## Demo

Demo available at `rtsp://restream-demo.northeurope.cloudapp.azure.com:8001/`

To try use following commands:
* Record side:  
`gst-launch-1.0 videotestsrc ! x264enc ! rtspclientsink location=rtsp://restream-demo.northeurope.cloudapp.azure.com:8001/test`
* Play side:  
`vlc rtsp://restream-demo.northeurope.cloudapp.azure.com:8001/test`
