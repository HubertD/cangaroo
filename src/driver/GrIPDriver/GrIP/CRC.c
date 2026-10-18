/*
  CRC.c - Simple library to calculate 8-, 16-, and 32-Bit CRC

  Copyright (c) 2018 Patrick F.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include "CRC.h"


//---- Prototypes ----//
static void CRC_CalculateCRC8Table(void);
static void CRC_CalculateCRC16Table(void);
static void CRC_CalculateCRC32Table(void);

static uint8_t CRC_ReverseBitOrder8(uint8_t value);
static uint32_t CRC_ReverseBitOrder32(uint32_t value);



#if (CRC_8_MODE == TABLE)
	static uint8_t CRC8Table[256u];
#endif

#if (CRC_16_MODE == TABLE)
	static uint16_t CRC16Table[256u];
#endif

#if (CRC_32_MODE == TABLE)
	static uint32_t CRC32Table[256u];
#endif



void CRC_Init(void)
{
	CRC_CalculateCRC8Table();
	CRC_CalculateCRC16Table();
	CRC_CalculateCRC32Table();
}


uint8_t CRC_CalculateCRC8(const uint8_t *Buffer, uint16_t Length)
{
	uint8_t retVal = 0u;
    uint16_t byteIndex = 0u;


	if(Buffer != NULL)
    {
#if (CRC_8_MODE == RUNTTIME)
		uint8_t bitIndex = 0u;

		retVal = CRC_8_INIT_VALUE;

		/* Do calculation procedure for each byte */
		for(byteIndex = 0u; byteIndex < Length; byteIndex++)
        {
			/* XOR new byte with temp result */
			retVal ^= (Buffer[byteIndex] << (CRC_8_RESULT_WIDTH - 8u));

			/* Do calculation for current data */
			for(bitIndex = 0u; bitIndex < 8u; bitIndex++)
            {
				if(retVal & (1u << (CRC_8_RESULT_WIDTH - 1u)))
                {
					retVal = (retVal << 1u) ^ CRC_8_POLYNOMIAL;
				}
				else
                {
					retVal = (retVal << 1u);
				}
			}
		}

		/* XOR result with specified value */
		retVal ^= CRC_8_XOR_VALUE;

#elif (CRC_8_MODE == TABLE)
		retVal = CRC_8_INIT_VALUE;

		for(byteIndex = 0u; byteIndex < Length; byteIndex++)
        {
			retVal = CRC8Table[(retVal) ^ Buffer[byteIndex]];
		}

		/* XOR result with specified value */
		retVal ^= CRC_8_XOR_VALUE;

#else
		/* Mode not implemented */
		retVal = 0x00u;

#endif
	}

	return retVal;
}


uint16_t CRC_CalculateCRC16(const uint8_t *Buffer, uint8_t Length)
{
   uint16_t retVal = 0u;
   uint8_t byteIndex = 0u;


    if(Buffer != NULL)
    {
#if (CRC_16_MODE==RUNTTIME)
		retVal = CRC_16_INIT_VALUE;

		/* Do calculation procedure for each byte */
		for(byteIndex = 0u; byteIndex < Length; byteIndex++)
        {
			/* XOR new byte with temp result */
			retVal ^= (Buffer[byteIndex] << (CRC_16_RESULT_WIDTH - 8u));

            uint8_t bitIndex = 0u;
			/* Do calculation for current data */
			for(bitIndex = 0u; bitIndex < 8u; bitIndex++)
            {
				if(retVal & (1u << (CRC_16_RESULT_WIDTH - 1u)))
                {
					retVal = (retVal << 1u) ^ CRC_16_POLYNOMIAL;
				}
				else
                {
					retVal = (retVal << 1u);
				}
			}
		}

		/* XOR result with specified value */
		retVal ^= CRC_16_XOR_VALUE;

#elif (CRC_16_MODE==TABLE)
		retVal = CRC_16_INIT_VALUE;

		/* Update the CRC using the data */
		for(byteIndex = 0u; byteIndex < Length; byteIndex++)
        {
			retVal = (retVal << 8u) ^ CRC16Table[(retVal >> 8u) ^ Buffer[byteIndex]];
		}

		/* XOR result with specified value */
		retVal ^= CRC_16_XOR_VALUE;
#else
		/* Mode not implemented */
		retVal = 0x0000u;

#endif
	}

   return retVal;
}


uint32_t CRC_CalculateCRC32(const uint8_t *Buffer, uint8_t Length)
{
	uint32_t retVal = 0u;
	uint8_t byteIndex = 0u;


	if(Buffer != NULL)
    {
#if (CRC_32_MODE==RUNTTIME)
		retVal = CRC_32_INIT_VALUE;

        /* Do calculation procedure for each byte */
		for(byteIndex = 0u; byteIndex < Length; byteIndex++)
        {
            /* XOR new byte with temp result */
            retVal ^= (CRC_ReverseBitOrder8(Buffer[byteIndex]) << (CRC_32_RESULT_WIDTH - 8u));

            uint8_t bitIndex = 0u;
            /* Do calculation for current data */
			for(bitIndex = 0u; bitIndex < 8u; bitIndex++)
            {
                if(retVal & (1u << (CRC_32_RESULT_WIDTH - 1u)))
                {
                    retVal = (retVal << 1u) ^ CRC_32_POLYNOMIAL;
                }
                else
                {
                    retVal = (retVal << 1u);
                }
            }
        }

        /* XOR result with specified value */
		retVal ^= CRC_32_XOR_VALUE;

#elif (CRC_32_MODE==TABLE)
        uint8_t data = 0u;

		retVal = CRC_32_INIT_VALUE;

		for(byteIndex = 0u; byteIndex < Length; ++byteIndex)
        {
            data = CRC_ReverseBitOrder8(Buffer[byteIndex]) ^ (retVal >> (CRC_32_RESULT_WIDTH - 8u));
            retVal = CRC32Table[data] ^ (retVal << 8u);
        }

		/* XOR result with specified value */
		retVal ^= CRC_32_XOR_VALUE;

#else
		/* Mode not implemented */
		retVal = 0x00000000u;

#endif
	}

    /* Reflect result */
    retVal = CRC_ReverseBitOrder32(retVal);

	return retVal;
}


static void CRC_CalculateCRC8Table(void)
{
#if (CRC_8_MODE==TABLE)
	uint16_t i = 0u, j = 0u;

	for(i = 0u; i < 256u; ++i)
	{
		uint8_t curr = i;

		for(j = 0u; j < 8u; ++j)
		{
			if((curr & 0x80u) != 0u)
			{
				curr = (curr << 1u) ^ CRC_8_POLYNOMIAL;
			}
			else
			{
				curr <<= 1u;
			}
		}

		CRC8Table[i] = curr;
	}
#endif
}


static void CRC_CalculateCRC16Table(void)
{
#if (CRC_16_MODE==TABLE)
	uint16_t i = 0u, j = 0u;
	uint16_t result = 0u;
	uint16_t xor_flag = 0u;

	for(i = 0u; i < 256u; i++)
	{
		result = i << 8u;

		for(j = 0u; j < 8u; j++)
		{
			/* Flag for XOR if leftmost bit is set */
			xor_flag = result & 0x8000u;

			/* Shift CRC */
			result <<= 1u;

			/* Perform the XOR */
			if(xor_flag != 0u)
				result ^= CRC_16_POLYNOMIAL;
		}

		CRC16Table[i] = result;
	}
#endif
}


static void CRC_CalculateCRC32Table(void)
{
#if (CRC_32_MODE==TABLE)
    uint32_t remainder = 0u;

    for(uint32_t dividend = 0u; dividend < 256u; ++dividend)
    {
        remainder = dividend << (CRC_32_RESULT_WIDTH - 8u);

        for(uint8_t bit = 8u; bit > 0u; --bit)
        {
            if(remainder & (1u << (CRC_32_RESULT_WIDTH - 1u)))
            {
                remainder = (remainder << 1u) ^ CRC_32_POLYNOMIAL;
            }
            else
            {
                remainder = (remainder << 1u);
            }
        }

        CRC32Table[dividend] = remainder;
    }
#endif
}


static uint8_t CRC_ReverseBitOrder8(uint8_t value)
{
    value = (value & 0xF0) >> 4u | (value & 0x0F) << 4u;
    value = (value & 0xCC) >> 2u | (value & 0x33) << 2u;
    value = (value & 0xAA) >> 1u | (value & 0x55) << 1u;

    return value;
}


static uint32_t CRC_ReverseBitOrder32(uint32_t value)
{
    uint32_t reversed = 0u;

    for(uint8_t i = 31u; value; )
    {
        reversed |= (value & 1u) << i;
        value >>= 1u;
        -- i;
    }

    return reversed;
}
