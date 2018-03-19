//
// Created by wlanjie on 2018/3/16.
//

#include "sl_config_descriptor.h"

SLConfigDescriptor::SLConfigDescriptor(Size headerSize) :
        Descriptor(DESCRIPTOR_TAG_SL_CONFIG, headerSize, 1),
        predefined(2) {

}

Result SLConfigDescriptor::writeFields(ByteStream &stream) {
    stream.writeUI08(predefined);
    return SUCCESS;
}
