#ifndef __TYPES_H__
#define __TYPES_H__

#ifndef FALSE
#define FALSE 0x00
#endif

#ifndef TRUE
#define TRUE 0x01
#endif

#define BV(bit) (1 << (bit))
#define CLEARBIT(BIT) (BIT = 0)
#define SETBIT(BIT) (BIT = 1)
#define TOGGLEBIT(BIT) (BIT ^= 1)

typedef unsigned char uint8_t;		/**< Unsigned 8 bit intgerer. */
typedef unsigned int uint16_t;	/**< Unsigned 16 bit intgerer. */
typedef unsigned long int uint32_t;	/**< Unsigned 32 bit intgerer. */
typedef signed char int8_t;		/**< signed 8 bit intgerer. */
typedef signed int int16_t;	/**< signed 16 bit intgerer. */
typedef signed long int int32_t;	/**< signed 32 bit intgerer. */

#endif //__TYPES_H__
