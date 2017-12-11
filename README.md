# NeulMessenger
NeulMessenger is a lightweight library to send messages to the OceanConnect platform from Huawei written in C++.
the OceanConnect platform from Huawei currently only supports their own proprietary messaging protocol, which is based on CoAP. It is implemented in their own chipsets but all other chipsets do not support this protocol, so they cannot be used with the OceanConnect platform. This library solves this, without using a full-blown CoAP protocol library which is often too heavy for extremely constrained devices. 

# Supported NB-IoT modules
Currently there are two implementations for NB-IoT modules: 
 - Boudica120Module is used for modules based on the Boudica 120 chipset from Huawei/Neul. These include the U-Blox Sara-N2xx series and Quectel BC95
 - MDM9206Module is used for modules based on the MDM9206 chipset from Qualcomm. These include the Quectel BG96
   
# Porting
To port this library to a different platform (i.e. not Arduino), the pure virtual functions in the NStream class need to implemented. Also check out platform.h for some platform specific typedefs and functions
To add support for a different module, the Module interface needs to implemented. 
