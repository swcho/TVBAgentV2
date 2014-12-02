# Search Linux Header First
#CXX=arm-linux-g++
#CC=arm-linux-g++
CC=g++
CXX=g++

CFLAGS = -g -fPIC
CFLAGS += -DTVB595V1 -DTVB380V4  
CFLAGS += -DPLX_LINUX
# this def. is to support b'd tvb497. CFLAGS += -DTVB597A_STANDALONE
#CFLAGS += -D_DBG_ON_TARGET_TH4435__
CFLAGS += -I. -I./usb
CFLAGS += -Iplatform/inc
CFLAGS += -Itvbmgr/inc

SOURCEFILES += 		lldmain.c
SOURCEFILES += 		dma_drv.cpp	
SOURCEFILES += 		wdm_drv_wrapper.cpp	
SOURCEFILES += 		wdm_drv.cpp	
SOURCEFILES += 		playcapt.cpp
SOURCEFILES += 		Chip_util.cpp	
#SOURCEFILES += 		TmccEnc2.c
SOURCEFILES += 		init_rs.c
SOURCEFILES += 		decode_rs.c
SOURCEFILES += 		decode_cif.c
SOURCEFILES += 		usb/NcRemotePci.c
SOURCEFILES += 		interp.cpp
SOURCEFILES += 		bd_init.cpp
SOURCEFILES += 		bd_lld_conf.cpp
SOURCEFILES += 		tvb360u.cpp
SOURCEFILES += 		lld_hwif.cpp
SOURCEFILES += 		dac_ctl.cpp
SOURCEFILES += 		dds_ctl.cpp
SOURCEFILES += 		attn_ctl.cpp
SOURCEFILES += 		sio_drv.cpp
SOURCEFILES += 		lld_log.cpp
SOURCEFILES += 		tvbmgr/mgr/ctl_mgr.cpp
SOURCEFILES += 		platform/src/tlv_msg.cpp

OBJSS = $(patsubst %.c, %.o, $(patsubst %.cpp, %.o, $(SOURCEFILES)))

all: tsplld0381.o
	
tsplld0381.o: $(OBJSS)
	cd ../libusb-0.1.12; make clean; make
	$(CXX) -shared -Wl,-soname,libtsplld0381.so.1 -o libtsplld0381.so.1.0.0 $(OBJSS) -lrt ../libusb-0.1.12/.libs/libusb.a ../libusb-0.1.12/.libs/libusbpp.a PlxApi.a

%.o: %.c
	$(CXX) $(CFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CFLAGS) -c -o $@ $<

tar: clean
	tar -czf TSP.tgz *.c *.h tsp_* Makefile

clean:
	rm -f *.o usb/*.o *~ ./*.o ./tvbmgr/mgr/*.o  ./platform/src/*.o 

install:
#LINUX
	
	rm -f /usr/lib/libtsplld0380.so.1.0.0
	rm -f /usr/lib/libtsplld0380.so.1
	rm -f /usr/lib/libtsplld0380.so
	cp -f ./libtsplld0381.so.1.0.0 /usr/lib/libtsplld0380.so.1.0.0
	ln -s ./libtsplld0381.so.1.0.0 /usr/lib/libtsplld0380.so.1
	ln -s ./libtsplld0381.so.1.0.0 /usr/lib/libtsplld0380.so

	rm -f /usr/lib/libtsplld0381.so.1.0.0
	rm -f /usr/lib/libtsplld0381.so.1
	rm -f /usr/lib/libtsplld0381.so
	cp -f ./libtsplld0381.so.1.0.0 /usr/lib
	ln -s ./libtsplld0381.so.1.0.0 /usr/lib/libtsplld0381.so.1
	ln -s ./libtsplld0381.so.1.0.0 /usr/lib/libtsplld0381.so
	rm -f /usr/lib/libtsplld0382.so.1.0.0
	rm -f /usr/lib/libtsplld0382.so.1
	rm -f /usr/lib/libtsplld0382.so
	cp -f ./libtsplld0381.so.1.0.0 /usr/lib/libtsplld0382.so.1.0.0
	ln -s ./libtsplld0382.so.1.0.0 /usr/lib/libtsplld0382.so.1
	ln -s ./libtsplld0382.so.1.0.0 /usr/lib/libtsplld0382.so
	rm -f /usr/lib/libtsplld0383.so.1.0.0
	rm -f /usr/lib/libtsplld0383.so.1
	rm -f /usr/lib/libtsplld0383.so
	cp -f ./libtsplld0381.so.1.0.0 /usr/lib/libtsplld0383.so.1.0.0
	ln -s ./libtsplld0383.so.1.0.0 /usr/lib/libtsplld0383.so.1
	ln -s ./libtsplld0383.so.1.0.0 /usr/lib/libtsplld0383.so	
	rm -f /usr/lib/libtsplld0384.so.1.0.0
	rm -f /usr/lib/libtsplld0384.so.1
	rm -f /usr/lib/libtsplld0384.so
	cp -f ./libtsplld0381.so.1.0.0 /usr/lib/libtsplld0384.so.1.0.0
	ln -s ./libtsplld0384.so.1.0.0 /usr/lib/libtsplld0384.so.1
	ln -s ./libtsplld0384.so.1.0.0 /usr/lib/libtsplld0384.so
	rm -f /usr/lib/libtsplld0385.so.1.0.0
	rm -f /usr/lib/libtsplld0385.so.1
	rm -f /usr/lib/libtsplld0385.so
	cp -f ./libtsplld0381.so.1.0.0 /usr/lib/libtsplld0385.so.1.0.0
	ln -s ./libtsplld0385.so.1.0.0 /usr/lib/libtsplld0385.so.1
	ln -s ./libtsplld0385.so.1.0.0 /usr/lib/libtsplld0385.so
	rm -f /usr/lib/libtsplld0386.so.1.0.0
	rm -f /usr/lib/libtsplld0386.so.1
	rm -f /usr/lib/libtsplld0386.so
	cp -f ./libtsplld0381.so.1.0.0 /usr/lib/libtsplld0386.so.1.0.0
	ln -s ./libtsplld0386.so.1.0.0 /usr/lib/libtsplld0386.so.1
	ln -s ./libtsplld0386.so.1.0.0 /usr/lib/libtsplld0386.so
	rm -f /usr/lib/libtsplld0387.so.1.0.0
	rm -f /usr/lib/libtsplld0387.so.1
	rm -f /usr/lib/libtsplld0387.so
	cp -f ./libtsplld0381.so.1.0.0 /usr/lib/libtsplld0387.so.1.0.0
	ln -s ./libtsplld0387.so.1.0.0 /usr/lib/libtsplld0387.so.1
	ln -s ./libtsplld0387.so.1.0.0 /usr/lib/libtsplld0387.so
	rm -f /usr/lib/libtsplld0388.so.1.0.0
	rm -f /usr/lib/libtsplld0388.so.1
	rm -f /usr/lib/libtsplld0388.so
	cp -f ./libtsplld0381.so.1.0.0 /usr/lib/libtsplld0388.so.1.0.0
	ln -s ./libtsplld0388.so.1.0.0 /usr/lib/libtsplld0388.so.1
	ln -s ./libtsplld0388.so.1.0.0 /usr/lib/libtsplld0388.so
	rm -f /usr/lib/libtsplld0389.so.1.0.0
	rm -f /usr/lib/libtsplld0389.so.1
	rm -f /usr/lib/libtsplld0389.so
	cp -f ./libtsplld0381.so.1.0.0 /usr/lib/libtsplld0389.so.1.0.0
	ln -s ./libtsplld0389.so.1.0.0 /usr/lib/libtsplld0389.so.1
	ln -s ./libtsplld0389.so.1.0.0 /usr/lib/libtsplld0389.so
	rm -f /usr/lib/libtsplld03810.so.1.0.0
	rm -f /usr/lib/libtsplld03810.so.1
	rm -f /usr/lib/libtsplld03810.so
	cp -f ./libtsplld0381.so.1.0.0 /usr/lib/libtsplld03810.so.1.0.0
	ln -s ./libtsplld03810.so.1.0.0 /usr/lib/libtsplld03810.so.1
	ln -s ./libtsplld03810.so.1.0.0 /usr/lib/libtsplld03810.so
	rm -f /usr/lib/libtsplld03811.so.1.0.0
	rm -f /usr/lib/libtsplld03811.so.1
	rm -f /usr/lib/libtsplld03811.so
	cp -f ./libtsplld0381.so.1.0.0 /usr/lib/libtsplld03811.so.1.0.0
	ln -s ./libtsplld03811.so.1.0.0 /usr/lib/libtsplld03811.so.1
	ln -s ./libtsplld03811.so.1.0.0 /usr/lib/libtsplld03811.so
	rm -f /usr/lib/libtsplld03812.so.1.0.0
	rm -f /usr/lib/libtsplld03812.so.1
	rm -f /usr/lib/libtsplld03812.so
	cp -f ./libtsplld0381.so.1.0.0 /usr/lib/libtsplld03812.so.1.0.0
	ln -s ./libtsplld03812.so.1.0.0 /usr/lib/libtsplld03812.so.1
	ln -s ./libtsplld03812.so.1.0.0 /usr/lib/libtsplld03812.so
	for i in `seq 13 23` ; 	do \
		rm -f /usr/lib/libtsplld038$$i.so.1.0.0 ; \
		rm -f /usr/lib/libtsplld038$$i.so.1 ; \
		rm -f /usr/lib/libtsplld038$$i.so ; \
		cp -f ./libtsplld0381.so.1.0.0  /usr/lib/libtsplld038$$i.so.1.0.0 ; \
		ln -s ./libtsplld03812.so.1.0.0 /usr/lib/libtsplld038$$i.so.1 ; \
		ln -s ./libtsplld03812.so.1.0.0 /usr/lib/libtsplld038$$i.so ; \
	done 
#TVB597LAN
	# create symbolic
	#cp -f ./libtsplld0389.so.1.0.0 ../tlib/
	#ln -fs libtsplld0389.so.1.0.0 libtsplld0389.so.1
	#ln -fs libtsplld0389.so.1.0.0 libtsplld0389.so
	#mv -f ./libtsplld0389.so ../tlib/	
	#mv -f ./libtsplld0389.so.1 ../tlib/
