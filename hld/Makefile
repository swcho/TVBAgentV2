# Search Linux Header First

#CC=arm-linux-g++
#CXX=arm-linux-g++
CC=g++
CXX=g++
CFLAGS = -g -fPIC

PWD=`pwd`

CFLAGS += -DTSPHLD0390_EXPORTS -D_FILE_OFFSET_BITS=64
CFLAGS += -DIP_STREAM
#CFLAGS += -DTSPHLD_VLC
CFLAGS += -DERROR_INJECT -DLOOP_ADAPT
CFLAGS += -D_MULTI_PLP_MUX_0
#CFLAGS += -DSTANDALONE
CFLAGS += -I./dvbt2 -I./dvbc2 -I./corelib -I./network -I./isdbt -I../shmem -I../include -I./IPStreamer
#CFLAGS += -NDEBUG # do not generate "debug" code 
CFLAGS += -I../inc -Iinc  -I./

SOURCEFILES 	+= ../shmem/shmem.cpp	
SOURCEFILES 	+= ../shmem/semp.c	
SOURCEFILES 	+= ../shmem/shmemc.cpp
SOURCEFILES 	+= hld_adapt.cpp
SOURCEFILES 	+= hld_bd_log.cpp
SOURCEFILES 	+= hld_ctl_hwbuf.cpp
SOURCEFILES 	+= hld_fmtr_bert.cpp
SOURCEFILES 	+= hld_fmtr_cmmb.cpp
SOURCEFILES 	+= hld_fmtr_dvbt_.cpp
SOURCEFILES 	+= hld_fmtr_ip.cpp
SOURCEFILES 	+= hld_fmtr_iq.cpp
SOURCEFILES 	+= hld_fmtr_isdbs.cpp
SOURCEFILES 	+= hld_fmtr_atscmh.cpp
SOURCEFILES 	+= hld_fmtr_isdbt13.cpp
SOURCEFILES 	+= hld_fmtr_loopthru.cpp
SOURCEFILES 	+= hld_fmtr_tdmb.cpp
SOURCEFILES 	+= hld_fmtr_dvbc2.cpp
SOURCEFILES 	+= hld_fs_rdwr.cpp
SOURCEFILES 	+= hld_gvar.cpp
SOURCEFILES 	+= hld_lldcaller.cpp
SOURCEFILES 	+= hld_utils.cpp
SOURCEFILES 	+= hld_tmcc_rmx.cpp
SOURCEFILES 	+= hld_multi_rfout.cpp
SOURCEFILES 	+= LLDExports.cpp
SOURCEFILES 	+= LLDWrapper.cpp
#SOURCEFILES 	+= VLCWrapper.cpp
#SOURCEFILES 	+= VLCWrapperImpl.cpp
SOURCEFILES 	+= TSPHLD0390/hldcapture.cpp	
SOURCEFILES 	+= TSPHLD0390/hldmgr.cpp	
SOURCEFILES 	+= TSPHLD0390/hldmonitor.cpp	
SOURCEFILES 	+= TSPHLD0390/hldplayback.cpp	
# bla bla bla
SOURCEFILES 	+= IPStreamer/srv_rcver.cpp
SOURCEFILES 	+= IPStreamer/srv_rcver_utils.cpp
SOURCEFILES 	+= IPStreamer/srv_sock.cpp
SOURCEFILES 	+= dvbc2/dvbc2_bbframe_payload.cpp
SOURCEFILES 	+= dvbc2/DVBC2_MultiplePLP.cpp
SOURCEFILES 	+= dvbc2/dvbc2_timestamp_payload.cpp
SOURCEFILES 	+= dvbc2/dvbc2_input_processing.cpp
SOURCEFILES 	+= dvbc2/dvbc2_multiplexer.cpp
SOURCEFILES 	+= dvbc2/dvbtc_l1_payload.cpp
SOURCEFILES 	+= dvbc2/DVBC2_system.cpp
SOURCEFILES 	+= dvbc2/CUtility.cpp

#////////////////////////////////////////////////
# corelib of new design 
SOURCEFILES 	+= corelib/corelib.cpp
SOURCEFILES	+= corelib/hldplay.cpp
SOURCEFILES	+= corelib/input_buffer.cpp
SOURCEFILES	+= corelib/input_file.cpp
SOURCEFILES	+= corelib/input_ip.cpp
SOURCEFILES	+= corelib/log.cpp
SOURCEFILES	+= corelib/media_interface.cpp
SOURCEFILES	+= corelib/multiplexer.cpp
SOURCEFILES	+= corelib/output.cpp
SOURCEFILES	+= corelib/mpeg2ts.cpp
SOURCEFILES 	+= corelib/linux/threads.cpp
SOURCEFILES 	+= corelib/input_ts.cpp

#////////////////////////////////////////////////
# dvb-t2 multiplexer 
SOURCEFILES	+= dvbt2/utility.c
SOURCEFILES	+= dvbt2/cir_buf.c
SOURCEFILES	+= dvbt2/dvbt2_bbframe_payload.c
SOURCEFILES	+= dvbt2/dvbt2_l1_payload.c
SOURCEFILES	+= dvbt2/dvbt2_multipleplp.c
SOURCEFILES	+= dvbt2/dvbt2_system.c
SOURCEFILES	+= dvbt2/dvbt2_timestamp_payload.c
SOURCEFILES	+= dvbt2/ts_mux.c
SOURCEFILES	+= dvbt2/dvbt2_multiplexer.cpp
#////////////////////////////////////////////////
# isdbt multiplexer 
SOURCEFILES	+= isdbt/isdbt_multiplexer.cpp

# mpeg2ts remux
SOURCEFILES	+= corelib/remux_mpeg2ts.c

SOURCEFILES	+= network/data_reader_from_ip.cpp

OBJSS = $(patsubst %.c, %.o, $(patsubst %.cpp, %.o, $(SOURCEFILES)))

OBJS = tsphld0381.o
all: $(OBJS)

tsphld0381.o: $(OBJSS)
	$(CXX) -shared -Wl,-soname,libtsphld0381.so.1 -o libtsphld0381.so.1.0.0 $(OBJSS) -ldl

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CFLAGS) -c -o $@ $<

tar: clean
	tar -czf TSP.tgz *.c *.h tsp_* Makefile

clean:
	rm -f *.o TSPHLD0390/*.o ../shmem/*.o dvbt2/*.o corelib/*.o corelib/linux/*.o  isdbt/*.o network/*.o  ../shmem/*.o  ./IPStreamer/*.o  ./dvbc2/*.o

install:
#Linux	
	rm -f /usr/lib/libtsphld0381.so.1.0.0
	rm -f /usr/lib/libtsphld0381.so.1
	rm -f /usr/lib/libtsphld0381.so
	cp -f ./libtsphld0381.so.1.0.0 /usr/lib
	ln -s ./libtsphld0381.so.1.0.0 /usr/lib/libtsphld0381.so.1
	ln -s ./libtsphld0381.so.1.0.0 /usr/lib/libtsphld0381.so
#TVB597LAN	
	# create symbolic link	
	#ln -fs libtsphld0381.so.1.0.0 libtsphld0381.so.1
	#ln -fs libtsphld0381.so.1.0.0 libtsphld0381.so
	# override file 
	#cp -f ./libtsphld0381.so.1.0.0 ../tlib/
	#cp -fd ./libtsphld0381.so ../tlib/
	#cp -fd ./libtsphld0381.so.1 ../tlib/
