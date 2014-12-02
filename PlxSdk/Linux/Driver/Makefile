
#PLX_SDK_DIR=~/work/MVV210_TVB597/tvb_lib_app/tvb595lan_ip/PlxSdk
include $(PLX_SDK_DIR)/Linux/Makefiles/MakeDriver.def

ifneq ($(KERNELRELEASE),)

    obj-m := $(ImageName).o

    $(ImageName)-objs := $(PLX_OBJECTS)

endif
