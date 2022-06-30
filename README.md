# GSTREAMER_RTSP_ANDROID
运行在安卓平台上的rtsp server demo，
基于GSTREAMER，
使用mk文件而不是cmake文件，是因为官方提供的预编译包中是mk文件，
mk中使用GSTREAMER_EXTRA_DEPS可以快速添加如gstreamer-rtsp-server-1.0的依赖库，
最初因为gstreamer-rtsp-server仓库已经移至因为gstreamer子仓库中开发，
gstreamer1.6版本后已经使用G_IO_MODULES := openssl

# WARN
必须替换下面为你的目录
GSTREAMER_ROOT        := Z:/gst/arm64

# CODE
使用gst_rtsp_media_factory_set_launch和launch指定参数方法来启动rtsp server，这很快

# TODO
本地测试ok，其他设备播放延迟很大

## deps

- gstreamer-1.0-android-universal-1.20.3 for android(https://gstreamer.freedesktop.org/data/pkg/android/)
- ndk@r20/21

## license

This is based on [android-tutorial-2 of gst-docs](https://github.com/GStreamer/gst-docs/tree/master/examples/tutorials/android/android-tutorial-2) 
and [test-appsrc of gst-rtsp-server](https://github.com/GStreamer/gst-rtsp-server/blob/master/examples/test-appsrc.c), 
and the license of this repo is under the license of them.

## help
mk文件中引用的含义：
GSTREAMER_PLUGINS_CORE := coreelements coretracers adder app audioconvert audiomixer audiorate audioresample audiotestsrc compositor gio overlaycomposition pango rawparse typefindfunctions videoconvert videorate videoscale videotestsrc volume autodetect videofilter
GSTREAMER_PLUGINS_CODECS := subparse ogg theora vorbis opus ivorbisdec alaw apetag audioparsers auparse avi dv flac flv flxdec icydemux id3demux isomp4 jpeg lame matroska mpg123 mulaw multipart png speex taglib vpx wavenc wavpack wavparse y4menc adpcmdec adpcmenc bz2 dashdemux dvbsuboverlay dvdspu hls id3tag kate midi mxf openh264 opusparse pcapparse pnm rfbsrc siren smoothstreaming subenc videoparsersbad y4mdec jpegformat gdp rsvg openjpeg spandsp sbc androidmedia
GSTREAMER_PLUGINS_ENCODING := encoding
GSTREAMER_PLUGINS_NET := tcp rtsp rtp rtpmanager soup udp dtls netsim sctp sdpelem srtp srt webrtc nice rtspclientsink
GSTREAMER_PLUGINS_PLAYBACK := playback
GSTREAMER_PLUGINS_VIS := libvisual goom goom2k1 audiovisualizers
GSTREAMER_PLUGINS_SYS := opengl ipcpipeline opensles
GSTREAMER_PLUGINS_EFFECTS := alpha alphacolor audiofx cairo cutter debug deinterlace dtmf effectv equalizer gdkpixbuf imagefreeze interleave level multifile replaygain shapewipe smpte spectrum videobox videocrop videomixer accurip aiff audiobuffersplit audiofxbad audiolatency audiomixmatrix autoconvert bayer coloreffects closedcaption debugutilsbad fieldanalysis freeverb frei0r gaudieffects geometrictransform inter interlace ivtc legacyrawparse proxy removesilence segmentclip smooth speed soundtouch timecode videofiltersbad videoframe_audiolevel webrtcdsp ladspa
GSTREAMER_PLUGINS_CAPTURE := camerabin
GSTREAMER_PLUGINS_CODECS_GPL := assrender
GSTREAMER_PLUGINS_CODECS_RESTRICTED := asfmux dtsdec mpegpsdemux mpegpsmux mpegtsdemux mpegtsmux voaacenc a52dec amrnb amrwbdec asf dvdsub dvdlpcmdec xingmux realmedia x264 libav
GSTREAMER_PLUGINS_NET_RESTRICTED := mms rtmp
GSTREAMER_PLUGINS_GES := nle ges
