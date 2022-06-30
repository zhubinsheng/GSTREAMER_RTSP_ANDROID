# GSTREAMER_ RTSP_ ANDROID

	-RTSP server demo running on Android platform,
	-Based on GStreamer,
	-The MK file is used instead of the cmake file because the official precompiled package contains the MK file,
	-Using GStreamer in MK_ EXTRA_ DEPs can quickly add dependent libraries such as gstreamer-rtsp-server-1.0,
	-Initially, the GStreamer RTSP server warehouse has been moved to the GStreamer sub warehouse for development,
	-this has been used since gstreamer1.6_ IO_ MODULES := openssl


# WARN

The following must be replaced with your directory
GSTREAMER_ ROOT := Z:/gst/arm64


# CODE

Using GST_ rtsp_ media_ factory_ set_ Launch and launch specify parameter methods to start RTSP server, which is very fast


# TODO

The local test is OK, but the playback delay of other devices is very large


## deps

- gstreamer-1.0-android-universal-1.20.3 for android( https://gstreamer.freedesktop.org/data/pkg/android/ )

- ndk@r20 /21


## license


This is based on [android-tutorial-2 of gst-docs]( https://github.com/GStreamer/gst-docs/tree/master/examples/tutorials/android/android-tutorial-2 )

and [test-appsrc of gst-rtsp-server]( https://github.com/GStreamer/gst-rtsp-server/blob/master/examples/test-appsrc.c ),

and the license of this repo is under the license of them.


## help

Meaning of reference in MK file:

	GSTREAMER_ PLUGINS_ CORE := coreelements coretracers adder app audioconvert audiomixer audiorate audioresample audiotestsrc compositor gio overlaycomposition pango rawparse typefindfunctions videoconvert videorate videoscale videotestsrc volume autodetect videofilter

	GSTREAMER_ PLUGINS_ CODECS := subparse ogg theora vorbis opus ivorbisdec alaw apetag audioparsers auparse avi dv flac flv flxdec icydemux id3demux isomp4 jpeg lame matroska mpg123 mulaw multipart png speex taglib vpx wavenc wavpack wavparse y4menc adpcmdec adpcmenc bz2 dashdemux dvbsuboverlay dvdspu hls id3tag kate midi mxf openh264 opusparse pcapparse pnm rfbsrc siren smoothstreaming subenc videoparsersbad y4mdec j pegformat gdp rsvg openjpeg spandsp sbc androidmedia

	GSTREAMER_ PLUGINS_ ENCODING := encoding

	GSTREAMER_ PLUGINS_ NET := tcp rtsp rtp rtpmanager soup udp dtls netsim sctp sdpelem srtp srt webrtc nice rtspclientsink

	GSTREAMER_ PLUGINS_ PLAYBACK := playback

	GSTREAMER_ PLUGINS_ VIS := libvisual goom goom2k1 audiovisualizers

	GSTREAMER_ PLUGINS_ SYS := opengl ipcpipeline opensles

	GSTREAMER_ PLUGINS_ EFFECTS := alpha alphacolor audiofx cairo cutter debug deinterlace dtmf effectv equalizer gdkpixbuf imagefreeze interleave level multifile replaygain shapewipe smpte spectrum videobox videocrop videomixer accurip aiff audiobuffersplit audiofxbad audiolatency audiomixmatrix autoconvert bayer coloreffects closedcaption debugutilsbad fieldanalysis freeverb frei0r gaudieffects geometrictransform inte r interlace ivtc legacyrawparse proxy removesilence segmentclip smooth speed soundtouch timecode videofiltersbad videoframe_ audiolevel webrtcdsp ladspa

	GSTREAMER_ PLUGINS_ CAPTURE := camerabin

	GSTREAMER_ PLUGINS_ CODECS_ GPL := assrender

	GSTREAMER_ PLUGINS_ CODECS_ RESTRICTED := asfmux dtsdec mpegpsdemux mpegpsmux mpegtsdemux mpegtsmux voaacenc a52dec amrnb amrwbdec asf dvdsub dvdlpcmdec xingmux realmedia x264 libav

	GSTREAMER_ PLUGINS_ NET_ RESTRICTED := mms rtmp

	GSTREAMER_ PLUGINS_ GES := nle ges
