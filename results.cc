//
// Created by wlanjie on 2018/2/7.
//

#include "results.h"

namespace mp4 {
// TODO name
const char*
ResultText(int result)
{
    switch (result) {
        case SUCCESS:                               return "SUCCESS";
        case FAILURE:                               return "FAILURE";
        case ERROR_OUT_OF_MEMORY:                   return "ERROR_OUT_OF_MEMORY";
        case ERROR_INVALID_PARAMETERS:              return "ERROR_INVALID_PARAMETERS";
        case ERROR_NO_SUCH_FILE:                    return "ERROR_NO_SUCH_FILE";
        case ERROR_PERMISSION_DENIED:               return "ERROR_PERMISSION_DENIED";
        case ERROR_CANNOT_OPEN_FILE:                return "ERROR_CANNOT_OPEN_FILE";
        case ERROR_EOS:                             return "ERROR_EOS";
        case ERROR_WRITE_FAILED:                    return "ERROR_WRITE_FAILED";
        case ERROR_READ_FAILED:                     return "ERROR_READ_FAILED";
        case ERROR_INVALID_FORMAT:                  return "ERROR_INVALID_FORMAT";
        case ERROR_NO_SUCH_ITEM:                    return "ERROR_NO_SUCH_ITEM";
        case ERROR_OUT_OF_RANGE:                    return "ERROR_OUT_OF_RANGE";
        case ERROR_INTERNAL:                        return "ERROR_INTERNAL";
        case ERROR_INVALID_STATE:                   return "ERROR_INVALID_STATE";
        case ERROR_LIST_EMPTY:                      return "ERROR_LIST_EMPTY";
        case ERROR_LIST_OPERATION_ABORTED:          return "ERROR_LIST_OPERATION_ABORTED";
        case ERROR_INVALID_RTP_CONSTRUCTOR_TYPE:    return "ERROR_INVALID_RTP_CONSTRUCTOR_TYPE";
        case ERROR_NOT_SUPPORTED:                   return "ERROR_NOT_SUPPORTED";
        case ERROR_INVALID_TRACK_TYPE:              return "ERROR_INVALID_TRACK_TYPE";
        case ERROR_INVALID_RTP_PACKET_EXTRA_DATA:   return "ERROR_INVALID_RTP_PACKET_EXTRA_DATA";
        case ERROR_BUFFER_TOO_SMALL:                return "ERROR_BUFFER_TOO_SMALL";
        case ERROR_NOT_ENOUGH_DATA:                 return "ERROR_NOT_ENOUGH_DATA";
        default:                                        return "UNKNOWN";
    }
}

}
