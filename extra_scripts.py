Import('env')
# For 1MHz
env.Replace(FUSESCMD="avrdude $UPLOADERFLAGS -e -Uhfuse:w:0xDF:m -Uefuse:w:0xff:m -Ulfuse:w:0x64:m")
# For 8MHz
#env.Replace(FUSESCMD="avrdude $UPLOADERFLAGS -e -Uhfuse:w:0xDF:m -Uefuse:w:0xff:m -Ulfuse:w:0xE4:m")
# For 16Mhz
#env.Replace(FUSESCMD="avrdude $UPLOADERFLAGS -e -Uhfuse:w:0xDF:m -Uefuse:w:0xff:m -Ulfuse:w:0xf1:m")
