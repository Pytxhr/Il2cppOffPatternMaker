#include "ByteHelper.h"

std::string ByteHelper::Byte2String(unsigned char byte)
{
    char bytePlaceHolder[3]{};

    sprintf_s(bytePlaceHolder, 3, "%02X", byte);

    return std::string(bytePlaceHolder);
}
