/*
 * MIT License
 *
 * Copyright (c) 2020 Jianhui Zhao <zhaojh329@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <netinet/in.h>
#include <arpa/inet.h>
#include <lauxlib.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>

#include "lua_compat.h"

/*
 * hexaddr("000011AC")
 * hexaddr("000011AC", "0000FFFF")
 */
static int lua_hexaddr(lua_State *L)
{
    const char *addr = lua_tostring(L, 1);
    const char *mask = lua_tostring(L, 2);
    char as[sizeof("255.255.255.255/32\0")];
    struct in_addr a;
    int bits;

    if (!addr) {
        lua_pushnil(L);
        return 1;
    }

    a.s_addr = strtoul(addr, NULL, 16);
    inet_ntop(AF_INET, &a, as, sizeof(as));

    if (mask) {
        for (a.s_addr = ntohl(strtoul(mask, NULL, 16)), bits = 0;
                a.s_addr & 0x80000000;
                a.s_addr <<= 1)
            bits++;

        sprintf(as + strlen(as), "/%d", bits);
    }

    lua_pushstring(L, as);
    return 1;
}

/*
 * hex6addr("fe80000000000000020c43fffe268a92")
 * hex6addr("fe80000000000000020c43fffe268a92", "80")
 */
static int lua_hex6addr(lua_State *L)
{
    const char *addr = lua_tostring(L, 1);
    const char *mask = lua_tostring(L, 2);
    char as[INET6_ADDRSTRLEN + sizeof("/128")];
    struct in6_addr a;
    int i;

#define hex(x) \
    (((x) <= '9') ? ((x) - '0') : \
        (((x) <= 'F') ? ((x) - 'A' + 10) : \
            ((x) - 'a' + 10)))

    if (!addr) {
        lua_pushnil(L);
        return 1;
    }


    for (i = 0; i < 16; i++, addr += 2)
        a.s6_addr[i] = (16 * hex(*addr)) + hex(*(addr + 1));

    inet_ntop(AF_INET6, &a, as, sizeof(as));

    if (mask)
        sprintf(as + strlen(as), "/%lu", strtoul(mask, NULL, 16));

    lua_pushstring(L, as);
    return 1;
}

static int calc_ip_prefix(uint32_t n)
{
    int prefix = 0;
    int i;

    n = ntohl(n);

    for (i = 31; i >= 0; i--) {
        if (n & (0x01 << i))
            prefix++;
        else
            break;
    }

    for (; i >= 0; i--) {
        if (n & (0x01 << i))
            return -1;
    }

    return prefix;
}

/*
 * ipcalc("192.168.2.1/24")
 * ipcalc({"192.168.2.1", "255.255.255.0"})
 *
 * {"ipaddr": "192.168.2.1", "netmask": "255.255.255.0", "broadcast": "192.168.2.255", "network": "192.168.2.0", "prefix": 24}
 */
static int lua_ipcalc(lua_State *L)
{
    char ipaddr[INET_ADDRSTRLEN] = "";
    char netmask[INET_ADDRSTRLEN] = "";
    char network[INET_ADDRSTRLEN] = "";
    char broadcast[INET_ADDRSTRLEN] = "";
    struct in_addr addr_ip, addr_mask;
    int prefix = 0;

    if (lua_isstring(L, 1)) {
        const char *cidr = luaL_checkstring(L, -1);
        char *slpos = strchr(cidr, '/');
        if (slpos) {
            if (slpos - cidr > INET_ADDRSTRLEN - 1)
                luaL_argerror(L, 1, "invalid addr");
            strncpy(ipaddr, cidr, slpos - cidr);
            prefix = atoi(slpos + 1);
        } else {
            strncpy(ipaddr, cidr, INET_ADDRSTRLEN - 1);
        }
    } else if (lua_istable(L, 1)) {
        lua_rawgeti(L, 1, 1);
        strncpy(ipaddr, luaL_checkstring(L, -1), INET_ADDRSTRLEN - 1);

        lua_rawgeti(L, 1, 2);

        switch(lua_type(L, -1)) {
        case LUA_TNUMBER:
            prefix = lua_tointeger(L, -1);
            break;
        case LUA_TSTRING:
            strncpy(netmask, luaL_checkstring(L, -1), INET_ADDRSTRLEN - 1);
        default:
            break;
        }
    } else {
        luaL_argerror(L, 1, "string or table expected");
    }

    if (!inet_aton(ipaddr, &addr_ip))
        luaL_argerror(L, 1, "invalid addr");

    if (netmask[0]) {
        if (!inet_aton(netmask, &addr_mask))
            luaL_argerror(L, 1, "invalid addr");
    } else {
        if (prefix > 0)
            addr_mask.s_addr = htonl(0xffffffff << (32 - prefix));
        else
            addr_mask.s_addr = 0;
        inet_ntop(AF_INET, &addr_mask, netmask, INET_ADDRSTRLEN);
    }

    prefix = calc_ip_prefix(addr_mask.s_addr);
    if (prefix < 0)
        luaL_argerror(L, 1, "invalid netmask");

    addr_ip.s_addr = addr_ip.s_addr & addr_mask.s_addr;
    inet_ntop(AF_INET, &addr_ip, network, INET_ADDRSTRLEN);

    addr_ip.s_addr = addr_ip.s_addr | ~addr_mask.s_addr;
    inet_ntop(AF_INET, &addr_ip, broadcast, INET_ADDRSTRLEN);

    lua_createtable(L, 0, 0);

    lua_pushstring(L, ipaddr);
    lua_setfield(L, -2, "ipaddr");

    lua_pushstring(L, netmask);
    lua_setfield(L, -2, "netmask");

    lua_pushstring(L, broadcast);
    lua_setfield(L, -2, "broadcast");

    lua_pushstring(L, network);
    lua_setfield(L, -2, "network");

    lua_pushinteger(L, prefix);
    lua_setfield(L, -2, "prefix");

    return 1;
}

static const luaL_Reg regs[] = {
    {"hexaddr",  lua_hexaddr},
    {"hex6addr", lua_hex6addr},
    {"ipcalc", lua_ipcalc},
    {NULL, NULL}
};

int luaopen_vuci_network(lua_State *L)
{
    luaL_newlib(L, regs);

    return 1;
}
