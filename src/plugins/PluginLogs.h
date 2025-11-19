//
// Created by VitorMM on 5/8/25.
//

#ifndef PLUGINLOGS_H
#define PLUGINLOGS_H

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80000000 ? '1' : '0'), \
  ((byte) & 0x40000000 ? '1' : '0'), \
  ((byte) & 0x20000000 ? '1' : '0'), \
  ((byte) & 0x10000000 ? '1' : '0'), \
  ((byte) & 0x08000000 ? '1' : '0'), \
  ((byte) & 0x04000000 ? '1' : '0'), \
  ((byte) & 0x02000000 ? '1' : '0'), \
  ((byte) & 0x01000000 ? '1' : '0'), \
  ((byte) & 0x00800000 ? '1' : '0'), \
  ((byte) & 0x00400000 ? '1' : '0'), \
  ((byte) & 0x00200000 ? '1' : '0'), \
  ((byte) & 0x00100000 ? '1' : '0'), \
  ((byte) & 0x00080000 ? '1' : '0'), \
  ((byte) & 0x00040000 ? '1' : '0'), \
  ((byte) & 0x00020000 ? '1' : '0'), \
  ((byte) & 0x00010000 ? '1' : '0'), \
  ((byte) & 0x00008000 ? '1' : '0'), \
  ((byte) & 0x00004000 ? '1' : '0'), \
  ((byte) & 0x00002000 ? '1' : '0'), \
  ((byte) & 0x00001000 ? '1' : '0'), \
  ((byte) & 0x00000800 ? '1' : '0'), \
  ((byte) & 0x00000400 ? '1' : '0'), \
  ((byte) & 0x00000200 ? '1' : '0'), \
  ((byte) & 0x00000100 ? '1' : '0'), \
  ((byte) & 0x00000080 ? '1' : '0'), \
  ((byte) & 0x00000040 ? '1' : '0'), \
  ((byte) & 0x00000020 ? '1' : '0'), \
  ((byte) & 0x00000010 ? '1' : '0'), \
  ((byte) & 0x00000008 ? '1' : '0'), \
  ((byte) & 0x00000004 ? '1' : '0'), \
  ((byte) & 0x00000002 ? '1' : '0'), \
  ((byte) & 0x00000001 ? '1' : '0')

#if !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__) && !defined(__DragonFly__) && !defined(__APPLE__)

#define PRINT_AS_8_BIT_HEX(ADDRESS) printf("0x%08x: 0x%02x\n", ADDRESS, nds->ARM7Read8(ADDRESS))
#define PRINT_AS_8_BIT_BIN(ADDRESS) printf("0x%08x: "BYTE_TO_BINARY_PATTERN"\n", ADDRESS, BYTE_TO_BINARY(nds->ARM7Read8(ADDRESS)))

#define PRINT_AS_16_BIT_HEX(ADDRESS) printf("0x%08x: 0x%04x\n", ADDRESS, nds->ARM7Read16(ADDRESS))
#define PRINT_AS_16_BIT_BIN(ADDRESS) printf("0x%08x: "BYTE_TO_BINARY_PATTERN"\n", ADDRESS, BYTE_TO_BINARY(nds->ARM7Read16(ADDRESS)))

#define PRINT_AS_32_BIT_HEX(ADDRESS) printf("0x%08x: 0x%08x\n", ADDRESS, nds->ARM7Read32(ADDRESS))
#define PRINT_AS_32_BIT_BIN(ADDRESS) printf("0x%08x: "BYTE_TO_BINARY_PATTERN"\n", ADDRESS, BYTE_TO_BINARY(nds->ARM7Read32(ADDRESS)))

#define log_getPixel(buffer, x, y, layer) buffer[(256*3 + 1)*(y) + (x) + 256*(layer)]

#define PRINT_PIXEL_COLOR(buffer,x,y) printf("PIXEL %d-%d: rgb(0x%02x, 0x%02x, 0x%02x) => 0x%08x\n", x,y,((log_getPixel(buffer,x,y,0) >> 0) & 0x3F), ((log_getPixel(buffer,x,y,0) >> 8) & 0x3F), ((log_getPixel(buffer,x,y,0) >> 16) & 0x3F), log_getPixel(buffer,x,y,0))

#else

#define PRINT_AS_8_BIT_HEX(ADDRESS)
#define PRINT_AS_8_BIT_BIN(ADDRESS)

#define PRINT_AS_16_BIT_HEX(ADDRESS)
#define PRINT_AS_16_BIT_BIN(ADDRESS)

#define PRINT_AS_32_BIT_HEX(ADDRESS)
#define PRINT_AS_32_BIT_BIN(ADDRESS)

#endif

#endif //PLUGINLOGS_H
