/*
 * Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
 *
 * Copyright (C) 2013 Matti Schnurbusch (original code)
 *   - adjusted INET APIs MACAdress Class for 64 Bit values
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <ctype.h>
#include "MACAddressExt.h"
#include "InterfaceToken.h"

const MACAddressExt MACAddressExt::UNSPECIFIED_ADDRESS;
const MACAddressExt MACAddressExt::BROADCAST_LONG_ADDRESS("FF:FF:FF:FF:FF:FF:FF:FF");
const MACAddressExt MACAddressExt::MULTICAST_PAUSE_LONG_ADDRESS("01:80:C2:00:00:00:00:01");

unsigned char MACAddressExt::getLongAddressByte(unsigned int k) const
{
    if (k >= MAC_LONG_ADDRESS_SIZE)
    {
        throw cRuntimeError("Array of size 8 indexed with %d", k);
    }
    int offset = (MAC_LONG_ADDRESS_SIZE - k - 1) * 8;
    return 0xff & (longAddr >> offset);
}

void MACAddressExt::setLongAddressByte(unsigned int k, unsigned char addrbyte)
{
    if (k >= MAC_LONG_ADDRESS_SIZE)
    {
        throw cRuntimeError("Array of size 8 indexed with %d", k);
    }
    int offset = (MAC_LONG_ADDRESS_SIZE - k - 1) * 8;
    longAddr = (longAddr & (~(((uint64) 0xff) << offset))) | (((uint64) addrbyte) << offset);
}

bool MACAddressExt::tryParseLongAddr(const char *hexstr)
{
    if (!hexstr)
    {
        return false;
    }

    // check syntax
    int numHexDigits = 0;
    for (const char *s = hexstr; *s; s++)
    {
        if (isxdigit(*s))
            numHexDigits++;
        else if (*s != ' ' && *s != ':' && *s != '-')
            return false; // wrong syntax
    }
    if (numHexDigits != 2 * MAC_LONG_ADDRESS_SIZE)
        return false;

    // Converts hex string into the address
    // if hex string is shorter, address is filled with zeros;
    // Non-hex characters are discarded before conversion.
    longAddr = 0; // clear top 16 bits too that setLongAddressByte() calls skip
    unsigned int k = 0U;
    const char *s = hexstr;
    for (unsigned int pos = 0U; pos < MAC_LONG_ADDRESS_SIZE; pos++)
    {
        if (!s || !*s)
        {
            setLongAddressByte(pos, 0);
        }
        else
        {
            while (*s && !isxdigit(*s))
            {
                s++;
            }
            if (!*s)
            {
                setLongAddressByte(pos, 0);
                continue;
            }
            unsigned char d = isdigit(*s) ? (*s - '0') : islower(*s) ? (*s - 'a' + 10) : (*s - 'A' + 10);
            d = d << 4;
            s++;

            while (*s && !isxdigit(*s))
            {
                s++;
            }
            if (!*s)
            {
                setLongAddressByte(pos, 0);
                continue;
            }
            d += isdigit(*s) ? (*s - '0') : islower(*s) ? (*s - 'a' + 10) : (*s - 'A' + 10);
            s++;

            setLongAddressByte(pos, d);
            k++;
        }
    }
    return true;
}

void MACAddressExt::setLongAddress(const char *hexstr)
{
    if (!tryParseLongAddr(hexstr))
    {
        throw cRuntimeError("MACAddress: wrong address syntax '%s': 16 hex digits expected, with optional embedded spaces, hyphens or colons", hexstr);
    }
}

void MACAddressExt::getLongAddressBytes(unsigned char *addrbytes) const
{
    for (unsigned int i = 0U; i < MAC_LONG_ADDRESS_SIZE; i++)
    {
        addrbytes[i] = getLongAddressByte(i);
    }
}

void MACAddressExt::setLongAddressBytes(unsigned char *addrbytes)
{
    longAddr = 0; // clear top 16 bits too that setLongAddressByte() calls skip
    for (unsigned int i = 0U; i < MAC_LONG_ADDRESS_SIZE; i++)
    {
        setLongAddressByte(i, addrbytes[i]);
    }
}

// New (from 802.11 MACAddress.h) definition of str
std::string MACAddressExt::str() const
{
    char buf[24];
    char *s = buf;
    for (unsigned int i = 0U; i < MAC_LONG_ADDRESS_SIZE; i++, s += 3)
    {
        sprintf(s, "%2.2X:", getLongAddressByte(i));
    }
    *(s - 1) = '\0';
    return std::string(buf);
}

MACAddressExt MACAddressExt::generateMacAddressWithNodeIndex(unsigned int index)
{
    uint64_t genMacAddr = 0x0AAA000000000000 + (index & MAC_LONG_ADDRESS_MASK);
    MACAddressExt addr(genMacAddr);
    return addr;
}
