#pragma once

#include <stdint.h>

typedef uint16_t state_t;
typedef int16_t state_delta_t;
typedef state_t encoding_table_t;
typedef uint8_t nb_bits_delta_t;
typedef uint16_t nb_t;
typedef uint8_t message_t;

#define TABLE_SIZE_LOG 10
#define TABLE_SIZE 1024
#define ALPHABET_LENGTH 256

namespace ANS
{
	namespace StaticTable
	{
		const message_t states[TABLE_SIZE] = {
			0,	 21,  32,  32,	46,	 73,  97,  99,	101, 101, 103, 104, 105,
			109, 110, 111, 114, 115, 116, 116, 119, 135, 178, 221, 8,	29,
			32,	 39,  48,  80,	97,	 99,  101, 101, 103, 104, 107, 109, 110,
			111, 114, 115, 116, 116, 119, 143, 186, 229, 10,  32,  32,	43,
			56,	 85,  97,  100, 101, 101, 104, 105, 108, 109, 110, 111, 114,
			115, 116, 117, 121, 151, 194, 237, 10,	32,	 32,  44,  63,	92,
			97,	 100, 101, 101, 104, 105, 108, 110, 111, 111, 114, 115, 116,
			117, 121, 159, 202, 245, 10,  32,  32,	44,	 68,  97,  97,	100,
			101, 102, 104, 105, 108, 110, 111, 111, 114, 115, 116, 117, 124,
			167, 210, 253, 18,	32,	 32,  46,  73,	97,	 98,  101, 101, 102,
			104, 105, 108, 110, 111, 112, 115, 116, 116, 118, 132, 175, 218,
			5,	 26,  32,  37,	46,	 78,  97,  99,	101, 101, 103, 104, 105,
			109, 110, 111, 114, 115, 116, 116, 119, 140, 183, 226, 10,	32,
			32,	 40,  53,  84,	97,	 100, 101, 101, 104, 105, 108, 109, 110,
			111, 114, 115, 116, 117, 120, 148, 191, 234, 10,  32,  32,	44,
			60,	 89,  97,  100, 101, 101, 104, 105, 108, 110, 111, 111, 114,
			115, 116, 117, 121, 156, 199, 242, 10,	32,	 32,  44,  65,	97,
			97,	 100, 101, 101, 104, 105, 108, 110, 111, 111, 114, 115, 116,
			117, 121, 164, 207, 250, 15,  32,  32,	46,	 72,  97,  98,	100,
			101, 102, 104, 105, 108, 110, 111, 112, 114, 115, 116, 118, 129,
			172, 215, 2,   23,	32,	 34,  46,  75,	97,	 99,  101, 101, 103,
			104, 105, 109, 110, 111, 114, 115, 116, 116, 119, 137, 180, 223,
			10,	 31,  32,  39,	50,	 82,  97,  99,	101, 101, 103, 105, 107,
			109, 110, 111, 114, 115, 116, 116, 119, 145, 188, 231, 10,	32,
			32,	 44,  58,  87,	97,	 100, 101, 101, 104, 105, 108, 109, 110,
			111, 114, 115, 116, 117, 121, 153, 196, 239, 10,  32,  32,	44,
			64,	 94,  97,  100, 101, 101, 104, 105, 108, 110, 111, 111, 114,
			115, 116, 117, 121, 161, 204, 247, 12,	32,	 32,  46,  70,	97,
			97,	 100, 101, 102, 104, 105, 108, 110, 111, 112, 114, 115, 116,
			117, 126, 169, 212, 255, 20,  32,  32,	46,	 73,  97,  98,	101,
			101, 102, 104, 105, 109, 110, 111, 113, 115, 116, 116, 119, 134,
			177, 220, 7,   28,	32,	 39,  47,  79,	97,	 99,  101, 101, 103,
			104, 107, 109, 110, 111, 114, 115, 116, 116, 119, 142, 185, 228,
			10,	 32,  32,  42,	55,	 84,  97,  100, 101, 101, 104, 105, 108,
			109, 110, 111, 114, 115, 116, 117, 121, 150, 193, 236, 10,	32,
			32,	 44,  62,  91,	97,	 100, 101, 101, 104, 105, 108, 110, 111,
			111, 114, 115, 116, 117, 121, 158, 201, 244, 10,  32,  32,	44,
			67,	 97,  97,  100, 101, 102, 104, 105, 108, 110, 111, 111, 114,
			115, 116, 117, 123, 166, 209, 252, 17,	32,	 32,  46,  72,	97,
			98,	 101, 101, 102, 104, 105, 108, 110, 111, 112, 114, 115, 116,
			118, 131, 174, 217, 4,	 25,  32,  36,	46,	 77,  97,  99,	101,
			101, 103, 104, 105, 109, 110, 111, 114, 115, 116, 116, 119, 139,
			182, 225, 10,  32,	32,	 39,  52,  84,	97,	 100, 101, 101, 104,
			105, 108, 109, 110, 111, 114, 115, 116, 116, 119, 147, 190, 233,
			10,	 32,  32,  44,	59,	 88,  97,  100, 101, 101, 104, 105, 108,
			110, 111, 111, 114, 115, 116, 117, 121, 155, 198, 241, 10,	32,
			32,	 44,  65,  96,	97,	 100, 101, 101, 104, 105, 108, 110, 111,
			111, 114, 115, 116, 117, 121, 163, 206, 249, 14,  32,  32,	46,
			72,	 97,  98,  100, 101, 102, 104, 105, 108, 110, 111, 112, 114,
			115, 116, 118, 128, 171, 214, 1,   22,	32,	 33,  46,  74,	97,
			99,	 101, 101, 103, 104, 105, 109, 110, 111, 114, 115, 116, 116,
			119, 136, 179, 222, 9,	 30,  32,  39,	49,	 81,  97,  99,	101,
			101, 103, 105, 107, 109, 110, 111, 114, 115, 116, 116, 119, 144,
			187, 230, 10,  32,	32,	 44,  57,  86,	97,	 100, 101, 101, 104,
			105, 108, 109, 110, 111, 114, 115, 116, 117, 121, 152, 195, 238,
			10,	 32,  32,  44,	63,	 93,  97,  100, 101, 101, 104, 105, 108,
			110, 111, 111, 114, 115, 116, 117, 121, 160, 203, 246, 11,	32,
			32,	 45,  69,  97,	97,	 100, 101, 102, 104, 105, 108, 110, 111,
			112, 114, 115, 116, 117, 125, 168, 211, 254, 19,  32,  32,	46,
			73,	 97,  98,  101, 101, 102, 104, 105, 109, 110, 111, 112, 115,
			116, 116, 118, 133, 176, 219, 6,   27,	32,	 38,  46,  79,	97,
			99,	 101, 101, 103, 104, 106, 109, 110, 111, 114, 115, 116, 116,
			119, 141, 184, 227, 10,	 32,  32,  41,	54,	 84,  97,  100, 101,
			101, 104, 105, 108, 109, 110, 111, 114, 115, 116, 117, 121, 149,
			192, 235, 10,  32,	32,	 44,  61,  90,	97,	 100, 101, 101, 104,
			105, 108, 110, 111, 111, 114, 115, 116, 117, 121, 157, 200, 243,
			10,	 32,  32,  44,	66,	 97,  97,  100, 101, 102, 104, 105, 108,
			110, 111, 111, 114, 115, 116, 117, 122, 165, 208, 251, 16,	32,
			32,	 46,  72,  97,	98,	 101, 101, 102, 104, 105, 108, 110, 111,
			112, 114, 115, 116, 118, 130, 173, 216, 3,	 24,  32,  35,	46,
			76,	 97,  99,  101, 101, 103, 104, 105, 109, 110, 111, 114, 115,
			116, 116, 119, 138, 181, 224, 10,  32,	32,	 39,  51,  83,	97,
			99,	 101, 101, 104, 105, 107, 109, 110, 111, 114, 115, 116, 116,
			119, 146, 189, 232, 10,	 32,  32,  44,	59,	 87,  97,  100, 101,
			101, 104, 105, 108, 110, 111, 111, 114, 115, 116, 117, 121, 154,
			197, 240, 10,  32,	32,	 44,  65,  95,	97,	 100, 101, 101, 104,
			105, 108, 110, 111, 111, 114, 115, 116, 117, 121, 162, 205, 248,
			13,	 32,  32,  46,	71,	 97,  98,  100, 101, 102, 104, 105, 108,
			110, 111, 112, 114, 115, 116, 117, 127, 170, 213};

		const state_t new_x[TABLE_SIZE] = {
			0,	 0,	   192, 208, 0,	   0,	544, 512, 224, 240, 384, 288, 256,
			320, 352,  800, 256, 352,  864, 896, 768, 0,   0,	0,	 0,	  0,
			224, 512,  0,	0,	 576,  640, 256, 272, 512, 320, 256, 384, 384,
			832, 288,  384, 928, 960,  896, 0,	 0,	  0,   448, 240, 256, 0,
			0,	 0,	   608, 640, 288,  304, 352, 288, 832, 448, 416, 864, 320,
			416, 992,  448, 0,	 0,	   0,	0,	 512, 272, 288, 0,	 0,	  0,
			640, 704,  320, 336, 384,  320, 896, 448, 896, 928, 352, 448, 0,
			512, 64,   0,	0,	 0,	   576, 304, 320, 64,  0,	672, 704, 768,
			352, 640,  416, 352, 960,  480, 960, 992, 384, 480, 16,	 576, 0,
			0,	 0,	   0,	0,	 336,  352, 64,	 256, 736, 0,	368, 384, 768,
			448, 384,  0,	512, 0,	   128, 512, 32,  48,  512, 0,	 0,	  0,
			0,	 0,	   368, 0,	 128,  0,	768, 768, 400, 416, 640, 480, 416,
			512, 544,  16,	416, 544,  64,	80,	 0,	  0,   0,	0,	 640, 384,
			400, 0,	   0,	0,	 800,  832, 432, 448, 512, 448, 32,	 576, 576,
			32,	 448,  576, 96,	 640,  0,	0,	 0,	  0,   704, 416, 432, 128,
			0,	 0,	   832, 896, 464,  480, 544, 480, 64,  608, 48,	 64,  480,
			608, 112,  704, 128, 0,	   0,	0,	 768, 448, 464, 192, 512, 864,
			896, 960,  496, 512, 576,  512, 96,	 640, 80,  96,	512, 640, 128,
			768, 192,  0,	0,	 0,	   0,	480, 496, 192, 0,	928, 128, 0,
			528, 896,  608, 544, 128,  672, 112, 256, 544, 672, 144, 768, 0,
			0,	 0,	   0,	0,	 512,  0,	256, 0,	  960, 896, 544, 560, 768,
			640, 576,  640, 704, 128,  576, 704, 160, 176, 64,	0,	 0,	  0,
			832, 0,	   528, 768, 0,	   0,	992, 0,	  576, 592, 896, 608, 512,
			704, 736,  144, 608, 736,  192, 208, 128, 0,   0,	0,	 896, 544,
			560, 256,  0,	0,	 0,	   32,	608, 624, 672, 640, 160, 768, 768,
			160, 640,  768, 224, 832,  256, 0,	 0,	  0,   960, 576, 592, 320,
			0,	 0,	   16,	64,	 640,  656, 704, 672, 192, 800, 176, 192, 672,
			800, 240,  896, 320, 0,	   0,	0,	 0,	  608, 624, 320, 0,	  32,
			48,	 96,   672, 0,	 736,  704, 224, 832, 208, 384, 704, 832, 256,
			960, 0,	   0,	0,	 0,	   0,	640, 656, 384, 512, 64,	 256, 688,
			704, 64,   768, 736, 832,  864, 224, 0,	  864, 272, 288, 192, 0,
			0,	 0,	   0,	0,	 672,  0,	0,	 0,	  80,  64,	720, 736, 0,
			800, 768,  896, 896, 240,  736, 896, 304, 320, 256, 0,	 0,	  0,
			0,	 688,  704, 0,	 0,	   256, 96,	 128, 752, 768, 832, 768, 256,
			960, 928,  256, 768, 928,  336, 0,	 384, 0,   0,	0,	 32,  720,
			736, 384,  0,	0,	 112,  160, 784, 800, 864, 800, 288, 960, 272,
			288, 800,  960, 352, 32,   448, 0,	 0,	  0,   64,	752, 768, 448,
			0,	 128,  144, 192, 816,  128, 896, 832, 320, 992, 304, 320, 832,
			992, 368,  64,	0,	 0,	   0,	0,	 0,	  784, 800, 448, 256, 160,
			384, 832,  848, 192, 928,  864, 352, 0,	  336, 512, 864, 0,	  384,
			0,	 0,	   0,	0,	 0,	   0,	816, 0,	  512, 0,	176, 128, 864,
			880, 64,   960, 896, 0,	   16,	352, 896, 16,  400, 416, 320, 0,
			0,	 0,	   96,	832, 848,  128, 0,	 512, 192, 224, 896, 912, 992,
			928, 384,  32,	32,	 368,  928, 32,	 432, 448, 384, 0,	 0,	  0,
			128, 864,  880, 512, 0,	   0,	208, 256, 928, 944, 0,	 960, 416,
			48,	 384,  400, 960, 48,   464, 96,	 512, 0,   0,	0,	 160, 896,
			912, 576,  0,	0,	 224,  288, 960, 976, 16,  992, 448, 64,  416,
			432, 992,  64,	480, 128,  576, 0,	 0,	  0,   0,	928, 944, 576,
			512, 240,  512, 320, 992,  256, 32,	 0,	  480, 80,	448, 640, 0,
			80,	 496,  128, 0,	 0,	   0,	0,	 0,	  960, 0,	640, 0,	  256,
			192, 1008, 0,	128, 48,   16,	64,	 96,  464, 16,	96,	 512, 528,
			448, 0,	   0,	0,	 0,	   0,	976, 256, 0,   0,	272, 256, 8,
			16,	 192,  32,	0,	 96,   112, 480, 32,  112, 544, 560, 512, 0,
			0,	 0,	   192, 992, 1008, 640, 0,	 0,	  288, 352, 24,	 32,  64,
			48,	 512,  128, 128, 496,  48,	128, 576, 160, 640, 0,	 0,	  0,
			224, 0,	   8,	704, 512,  0,	304, 384, 40,  48,	80,	 64,  544,
			144, 512,  528, 64,	 144,  592, 192, 704, 0,   0,	0,	 0,	  16,
			24,	 0,	   0,	320, 336,  416, 56,	 320, 96,  80,	576, 160, 544,
			768, 80,   160, 608, 224,  0,	0,	 0,	  0,   0,	32,	 40,  704,
			768, 352,  640, 64,	 72,   384, 112, 96,  160, 176, 560, 896, 176,
			624, 640,  256, 0,	 0,	   0,	0,	 0,	  48,  0,	768, 512, 368,
			320, 80,   88,	256, 128,  0,	192, 192, 576, 96,	192, 656, 672,
			576, 0,	   0,	0,	 256,  56,	64,	 0,	  0,   768, 384, 448, 96,
			104, 144,  112, 608, 224,  208, 592, 112, 208, 688, 256, 768, 0,
			0,	 0,	   288, 72,	 80,   768, 0,	 0,	  400, 480, 112, 120, 160,
			128, 640,  224, 608, 624,  128, 224, 704, 288, 832, 0,	 0,	  0,
			320, 88,   96,	832, 0,	   416, 432, 512, 128, 448, 176, 144, 672,
			240, 640,  656, 144, 240,  720, 320, 0,	  0,   0,	0,	 0,	  104,
			112, 832,  768, 448, 768,  136, 144, 512, 192, 160, 704, 256, 672,
			0,	 160,  256, 736, 384,  0,	0,	 0,	  0,   0,	120, 0,	  896,
			0,	 464,  384, 152, 160,  320, 208, 176, 256, 272, 688, 176, 272,
			752, 768,  640, 0,	 0,	   0,	352, 128, 136, 384, 0,	 0,	  480,
			448, 168,  176, 224, 192,  128, 288, 288, 704, 192, 288, 784, 800,
			704, 0,	   0,	0,	 384,  144, 152, 896, 512, 512, 496, 544, 184,
			192, 240,  208, 736, 304,  720, 736, 208, 304, 816, 352, 896, 0,
			0,	 0,	   416, 160, 168,  960, 256, 0,	  512, 576, 200, 208, 256,
			224, 768,  320, 752, 768,  224, 320, 832, 384, 960, 0,	 0,	  0,
			0,	 176,  184, 960, 0,	   528, 896, 608, 216, 576, 272, 240, 800,
			336, 784,  64,	240, 336,  848, 416, 0,	  0,   0};

		const encoding_table_t encoding_table[TABLE_SIZE] = {
			1024, 1667, 1286, 1929, 1548, 1167, 1810, 1429, 1048, 1691, 1072,
			1096, 1120, 1191, 1215, 1239, 1310, 1334, 1358, 1453, 1477, 1501,
			1572, 1596, 1620, 1715, 1739, 1834, 1858, 1882, 1953, 1977, 2001,
			1763, 1382, 2025, 1644, 1263, 1906, 1525, 1144, 1787, 1406, 1025,
			1668, 1287, 1930, 1549, 1168, 1811, 1430, 1049, 1692, 1311, 1026,
			1027, 1050, 1073, 1074, 1097, 1098, 1121, 1122, 1145, 1146, 1169,
			1192, 1193, 1216, 1217, 1240, 1241, 1264, 1265, 1288, 1312, 1335,
			1336, 1359, 1360, 1383, 1384, 1407, 1408, 1431, 1454, 1455, 1478,
			1479, 1502, 1503, 1526, 1527, 1550, 1573, 1574, 1597, 1598, 1621,
			1622, 1645, 1646, 1669, 1693, 1716, 1717, 1740, 1741, 1764, 1765,
			1788, 1789, 1812, 1835, 1836, 1859, 1860, 1883, 1884, 1907, 1908,
			1931, 1954, 1955, 1978, 1979, 2002, 2003, 2026, 2027, 1670, 1289,
			1932, 1551, 1170, 1813, 1051, 1313, 1432, 1575, 1694, 1956, 1194,
			1837, 1456, 1075, 1099, 1123, 1218, 1242, 1337, 1361, 1480, 1504,
			1599, 1623, 1718, 1742, 1861, 1885, 1980, 2004, 1766, 1028, 1147,
			1171, 1266, 1290, 1385, 1409, 1528, 1552, 1647, 1671, 1790, 1814,
			1909, 1933, 2028, 1433, 1052, 1695, 1314, 1957, 1576, 1195, 1838,
			1457, 1076, 1719, 1338, 1600, 1981, 1219, 1862, 1481, 1100, 1743,
			1362, 1243, 1624, 2005, 1886, 1505, 1124, 1767, 1386, 2029, 1267,
			1529, 1648, 1910, 1029, 1148, 1410, 1791, 1672, 1291, 1934, 1553,
			1172, 1434, 1815, 1053, 1696, 1315, 1958, 1196, 1458, 1577, 1839,
			1077, 1720, 1339, 1982, 1601, 1220, 1863, 1482, 1101, 1744, 1363,
			2006, 1625, 1030, 1054, 1078, 1102, 1125, 1126, 1149, 1173, 1197,
			1221, 1244, 1245, 1268, 1292, 1316, 1340, 1364, 1387, 1388, 1411,
			1435, 1459, 1483, 1506, 1507, 1530, 1554, 1578, 1602, 1626, 1649,
			1673, 1697, 1721, 1745, 1768, 1769, 1792, 1816, 1840, 1864, 1887,
			1888, 1911, 1935, 1959, 1983, 2007, 2030, 1150, 1269, 1412, 1531,
			1650, 1793, 1912, 2031, 1031, 1055, 1174, 1293, 1317, 1436, 1555,
			1674, 1698, 1817, 1936, 1960, 1079, 1103, 1127, 1198, 1222, 1246,
			1270, 1341, 1365, 1389, 1460, 1484, 1508, 1579, 1603, 1627, 1651,
			1722, 1746, 1770, 1841, 1865, 1889, 1984, 2008, 2032, 1032, 1033,
			1056, 1057, 1080, 1081, 1104, 1105, 1128, 1151, 1152, 1175, 1176,
			1199, 1200, 1223, 1224, 1247, 1248, 1271, 1294, 1295, 1318, 1319,
			1342, 1343, 1366, 1367, 1390, 1413, 1414, 1437, 1438, 1461, 1462,
			1485, 1486, 1509, 1532, 1533, 1556, 1557, 1580, 1581, 1604, 1605,
			1628, 1629, 1652, 1675, 1676, 1699, 1700, 1723, 1724, 1747, 1748,
			1771, 1794, 1795, 1818, 1819, 1842, 1843, 1866, 1867, 1890, 1913,
			1914, 1937, 1938, 1961, 1962, 1985, 1986, 2009, 2010, 2033, 1129,
			1153, 1272, 1391, 1415, 1510, 1534, 1653, 1772, 1796, 1891, 1915,
			2034, 1034, 1058, 1177, 1296, 1320, 1439, 1558, 1677, 1701, 1820,
			1939, 1035, 1059, 1082, 1106, 1130, 1154, 1178, 1201, 1225, 1249,
			1273, 1297, 1344, 1368, 1392, 1416, 1440, 1463, 1487, 1511, 1535,
			1559, 1582, 1606, 1630, 1654, 1678, 1725, 1749, 1773, 1797, 1821,
			1844, 1868, 1892, 1916, 1940, 1963, 1987, 2011, 2035, 1036, 1083,
			1107, 1131, 1155, 1179, 1202, 1226, 1250, 1274, 1298, 1321, 1345,
			1369, 1393, 1417, 1464, 1488, 1512, 1536, 1560, 1583, 1607, 1631,
			1655, 1679, 1702, 1726, 1750, 1774, 1798, 1845, 1869, 1893, 1917,
			1941, 1964, 1988, 2012, 2036, 1822, 1060, 1322, 1441, 1703, 1965,
			1084, 1108, 1132, 1156, 1203, 1227, 1251, 1275, 1346, 1370, 1394,
			1465, 1489, 1513, 1537, 1584, 1608, 1632, 1656, 1727, 1751, 1775,
			1846, 1870, 1894, 1918, 1989, 2013, 2037, 1037, 1061, 1085, 1180,
			1204, 1299, 1323, 1347, 1418, 1442, 1466, 1561, 1585, 1680, 1704,
			1728, 1799, 1823, 1847, 1942, 1966, 1038, 1062, 1086, 1109, 1133,
			1157, 1181, 1205, 1228, 1252, 1276, 1300, 1324, 1348, 1371, 1395,
			1419, 1443, 1467, 1490, 1514, 1538, 1562, 1586, 1609, 1633, 1657,
			1681, 1705, 1729, 1752, 1776, 1800, 1824, 1848, 1871, 1895, 1919,
			1943, 1967, 1990, 2014, 2038, 1039, 1063, 1087, 1110, 1111, 1134,
			1135, 1158, 1182, 1206, 1229, 1230, 1253, 1254, 1277, 1301, 1325,
			1349, 1372, 1373, 1396, 1420, 1444, 1468, 1491, 1492, 1515, 1516,
			1539, 1563, 1587, 1610, 1611, 1634, 1635, 1658, 1682, 1706, 1730,
			1753, 1754, 1777, 1801, 1825, 1849, 1872, 1873, 1896, 1897, 1920,
			1944, 1968, 1991, 1992, 2015, 2016, 2039, 1159, 1278, 1397, 1540,
			1659, 1778, 1802, 1921, 2040, 1421, 1040, 1064, 1088, 1112, 1136,
			1183, 1207, 1231, 1255, 1279, 1302, 1326, 1350, 1374, 1398, 1445,
			1469, 1493, 1517, 1541, 1564, 1588, 1612, 1636, 1660, 1683, 1707,
			1731, 1755, 1779, 1826, 1850, 1874, 1898, 1922, 1945, 1969, 1993,
			2017, 2041, 1041, 1065, 1089, 1113, 1137, 1160, 1184, 1208, 1232,
			1256, 1280, 1303, 1327, 1351, 1375, 1399, 1422, 1446, 1470, 1494,
			1518, 1542, 1565, 1589, 1613, 1637, 1661, 1684, 1708, 1732, 1756,
			1780, 1803, 1827, 1851, 1875, 1899, 1923, 1946, 1970, 1994, 2018,
			2042, 1042, 1043, 1066, 1067, 1090, 1114, 1138, 1161, 1162, 1185,
			1186, 1209, 1233, 1257, 1281, 1304, 1305, 1328, 1329, 1352, 1376,
			1400, 1423, 1424, 1447, 1448, 1471, 1495, 1519, 1543, 1566, 1567,
			1590, 1591, 1614, 1638, 1662, 1685, 1686, 1709, 1710, 1733, 1757,
			1781, 1804, 1805, 1828, 1829, 1852, 1876, 1900, 1924, 1947, 1948,
			1971, 1972, 1995, 2019, 2043, 1091, 1115, 1139, 1210, 1234, 1258,
			1353, 1377, 1401, 1472, 1496, 1520, 1615, 1639, 1734, 1758, 1782,
			1853, 1877, 1901, 1996, 2020, 2044, 1163, 1282, 1544, 1663, 1806,
			1925, 1044, 1068, 1187, 1306, 1330, 1425, 1449, 1568, 1592, 1687,
			1711, 1830, 1949, 1973, 1211, 1092, 1116, 1235, 1259, 1354, 1378,
			1473, 1497, 1616, 1640, 1735, 1759, 1854, 1878, 1997, 2021, 1902,
			1521, 1140, 1783, 1402, 2045, 1664, 1283, 1926, 1545, 1164, 1807,
			1426, 1045, 1688, 1307, 1950, 1569, 1188, 1831, 1450, 1069, 1712,
			1331, 1974, 1593, 1212, 1855, 1474, 1093, 1736, 1355, 1998, 1617,
			1236, 1879, 1498, 1117, 1760, 1379, 2022, 1641, 1260, 1903, 1522,
			1141, 1784, 1403, 2046, 1665, 1284, 1927, 1546, 1165, 1808, 1427,
			1046, 1689, 1308, 1951, 1570, 1189, 1832, 1451, 1070, 1713, 1332,
			1975, 1594, 1213, 1856, 1475, 1094, 1737, 1356, 1999, 1618, 1237,
			1880, 1499, 1118, 1761, 1380, 2023, 1642, 1261, 1904, 1523, 1142,
			1785, 1404, 2047, 1666, 1285, 1928, 1547, 1166, 1809, 1428, 1047,
			1690, 1309, 1952, 1571, 1190, 1833, 1452, 1071, 1714, 1333, 1976,
			1595, 1214, 1857, 1476, 1095, 1738, 1357, 2000, 1619, 1238, 1881,
			1500, 1119, 1762, 1381, 2024, 1643, 1262, 1905, 1524, 1143, 1786,
			1405};

		const nb_bits_delta_t nb_bits_delta[TABLE_SIZE] = {
			10, 10, 4,	4,	6,	8,	5,	7,	4,	4,	7,	5,	5,	6,	5,	5,	5,
			5,	5,	5,	7,	10, 10, 10, 10, 10, 4,	8,	10, 10, 5,	7,	4,	4,
			7,	5,	8,	6,	5,	5,	5,	5,	5,	5,	7,	10, 10, 10, 6,	4,	4,
			10, 10, 10, 5,	6,	4,	4,	5,	5,	6,	6,	5,	5,	5,	5,	5,	6,
			6,	10, 10, 10, 6,	4,	4,	6,	9,	10, 5,	6,	4,	4,	5,	5,	6,
			5,	5,	5,	5,	5,	4,	6,	6,	10, 10, 10, 6,	4,	4,	6,	10, 5,
			5,	6,	4,	7,	5,	5,	6,	5,	5,	5,	5,	5,	4,	6,	10, 10, 10,
			10, 10, 4,	4,	6,	8,	5,	7,	4,	4,	7,	5,	5,	5,	5,	4,	7,
			5,	4,	4,	8,	10, 10, 10, 10, 10, 4,	10, 6,	10, 5,	7,	4,	4,
			7,	5,	5,	6,	5,	4,	5,	5,	4,	4,	6,	10, 10, 10, 6,	4,	4,
			10, 10, 8,	5,	6,	4,	4,	5,	5,	5,	6,	5,	4,	5,	5,	4,	6,
			10, 10, 10, 10, 6,	4,	4,	6,	10, 10, 5,	6,	4,	4,	5,	5,	5,
			5,	4,	4,	5,	5,	4,	6,	6,	10, 10, 10, 6,	4,	4,	6,	9,	5,
			5,	6,	4,	4,	5,	5,	5,	5,	4,	4,	5,	5,	4,	6,	6,	10, 10,
			10, 10, 4,	4,	6,	8,	5,	7,	5,	4,	7,	5,	5,	5,	5,	4,	7,
			5,	5,	4,	8,	10, 10, 10, 10, 10, 4,	10, 6,	10, 5,	7,	4,	4,
			7,	5,	5,	6,	5,	4,	5,	5,	4,	4,	6,	10, 10, 10, 6,	10, 4,
			8,	10, 10, 5,	6,	4,	4,	7,	5,	8,	6,	5,	4,	5,	5,	4,	4,
			6,	10, 10, 10, 6,	4,	4,	6,	10, 9,	4,	5,	4,	4,	5,	5,	5,
			6,	5,	4,	5,	5,	4,	6,	6,	10, 10, 10, 6,	4,	4,	6,	10, 10,
			4,	5,	4,	4,	5,	5,	5,	5,	4,	4,	5,	5,	4,	6,	6,	10, 10,
			10, 10, 4,	4,	6,	10, 4,	4,	5,	4,	6,	5,	5,	5,	5,	4,	7,
			5,	5,	4,	6,	10, 10, 10, 10, 10, 4,	4,	6,	8,	4,	7,	4,	4,
			6,	5,	5,	6,	5,	4,	10, 5,	4,	4,	6,	10, 10, 10, 10, 10, 4,
			7,	10, 9,	4,	6,	4,	4,	6,	5,	8,	6,	5,	4,	5,	5,	4,	4,
			6,	10, 10, 10, 5,	4,	4,	10, 10, 8,	4,	5,	4,	4,	5,	5,	5,
			6,	5,	4,	5,	5,	4,	5,	6,	10, 10, 10, 5,	4,	4,	6,	10, 10,
			4,	5,	4,	4,	5,	5,	5,	5,	4,	4,	5,	5,	4,	5,	6,	10, 10,
			10, 5,	4,	4,	6,	10, 4,	4,	5,	4,	6,	5,	5,	5,	5,	4,	4,
			5,	5,	4,	5,	10, 10, 10, 10, 10, 4,	4,	6,	8,	4,	7,	4,	4,
			6,	5,	5,	5,	4,	4,	7,	5,	4,	4,	7,	10, 10, 10, 10, 10, 4,
			10, 6,	10, 4,	6,	4,	4,	6,	5,	5,	5,	4,	4,	5,	4,	4,	4,
			6,	10, 10, 10, 5,	4,	4,	7,	10, 8,	4,	5,	4,	4,	5,	5,	5,
			5,	4,	4,	5,	4,	4,	4,	6,	10, 10, 10, 5,	4,	4,	6,	9,	10,
			4,	5,	4,	4,	4,	5,	5,	4,	4,	4,	5,	4,	4,	5,	6,	10, 10,
			10, 5,	4,	4,	6,	8,	10, 4,	5,	4,	4,	4,	5,	5,	4,	4,	4,
			5,	4,	4,	5,	6,	10, 10, 10, 10, 4,	4,	6,	8,	4,	7,	5,	4,
			6,	4,	4,	5,	4,	4,	7,	4,	4,	4,	7,	10, 10, 10, 10, 10, 4,
			10, 6,	10, 4,	6,	4,	3,	6,	4,	4,	5,	4,	4,	4,	4,	4,	4,
			6,	10, 10, 10, 10, 10, 4,	7,	10, 10, 4,	6,	3,	3,	6,	4,	7,
			5,	4,	4,	4,	4,	4,	4,	6,	10, 10, 10, 5,	4,	4,	6,	10, 10,
			4,	5,	3,	3,	4,	4,	5,	5,	4,	4,	4,	4,	4,	5,	6,	10, 10,
			10, 5,	3,	3,	6,	9,	10, 4,	5,	3,	3,	4,	4,	5,	4,	4,	4,
			4,	4,	4,	5,	6,	10, 10, 10, 10, 3,	3,	10, 10, 4,	4,	5,	3,
			6,	4,	4,	5,	4,	4,	7,	4,	4,	4,	5,	10, 10, 10, 10, 10, 3,
			3,	6,	8,	4,	7,	3,	3,	6,	4,	4,	5,	4,	4,	7,	4,	4,	4,
			7,	10, 10, 10, 10, 10, 3,	10, 6,	9,	4,	6,	3,	3,	6,	4,	10,
			5,	4,	4,	4,	4,	4,	4,	6,	10, 10, 10, 5,	3,	3,	10, 10, 8,
			4,	5,	3,	3,	4,	4,	5,	5,	4,	4,	4,	4,	4,	5,	6,	10, 10,
			10, 5,	3,	3,	6,	10, 10, 4,	5,	3,	3,	4,	4,	5,	4,	4,	4,
			4,	4,	4,	5,	6,	10, 10, 10, 5,	3,	3,	6,	10, 4,	4,	5,	3,
			6,	4,	4,	5,	4,	4,	4,	4,	4,	4,	5,	10, 10, 10, 10, 10, 3,
			3,	6,	8,	4,	7,	3,	3,	6,	4,	4,	5,	4,	4,	6,	4,	4,	4,
			7,	10, 10, 10, 10, 10, 3,	10, 6,	10, 4,	6,	3,	3,	6,	4,	4,
			5,	4,	4,	4,	4,	4,	4,	6,	10, 10, 10, 5,	3,	3,	7,	10, 10,
			4,	6,	3,	3,	4,	4,	7,	5,	4,	4,	4,	4,	4,	4,	6,	10, 10,
			10, 5,	3,	3,	6,	9,	9,	4,	5,	3,	3,	4,	4,	5,	4,	4,	4,
			4,	4,	4,	5,	6,	10, 10, 10, 5,	3,	3,	6,	8,	10, 4,	5,	3,
			3,	4,	4,	5,	4,	4,	4,	4,	4,	4,	5,	6,	10, 10, 10, 10, 3,
			3,	6,	10, 4,	7,	5,	3,	6,	4,	4,	5,	4,	4,	6,	4,	4,	4,
			5,	10, 10, 10};

		const nb_t nb[TABLE_SIZE] = {
			19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456,
			19456, 10816, 19456, 19456, 19456, 19456, 19456, 19456, 19456,
			19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456,
			19456, 19456, 19456, 19456, 19456, 6976,  19456, 19456, 19456,
			19456, 19456, 19456, 14848, 19456, 19456, 19456, 19456, 11264,
			19456, 11264, 19456, 19456, 19456, 19456, 19456, 19456, 19456,
			19456, 19456, 19456, 19456, 19456, 17408, 19456, 19456, 19456,
			17408, 19456, 16896, 19456, 19456, 19456, 19456, 19456, 19456,
			15360, 15360, 19456, 19456, 19456, 19456, 19456, 17408, 19456,
			19456, 19456, 19456, 15360, 19456, 19456, 17408, 19456, 19456,
			19456, 19456, 19456, 19456, 19456, 19456, 19456, 8672,	13312,
			12800, 10624, 6944,	 12672, 12928, 8928,  8960,	 19456, 15104,
			10432, 10944, 8864,	 8416,	13184, 19456, 8960,	 8864,	8352,
			10816, 14848, 12544, 19456, 11264, 19456, 19456, 19456, 19456,
			19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456,
			19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456,
			19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456,
			19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456,
			19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456,
			19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456,
			19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456,
			19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456,
			19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456,
			19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456,
			19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456,
			19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456,
			19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456,
			19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456, 19456,
			19456, 19456, 19456, 19456};

		const state_delta_t start[ALPHABET_LENGTH] = {
			-1,	  0,	1,	  2,	3,	  4,	5,	  6,	7,	  8,	-13,
			32,	  33,	34,	  35,	36,	  37,	38,	  39,	40,	  41,	42,
			43,	  44,	45,	  46,	47,	  48,	49,	  50,	51,	  52,	-22,
			129,  130,	131,  132,	133,  134,	130,  141,	142,  143,	144,
			130,  161,	147,  178,	179,  180,	181,  182,	183,  184,	185,
			186,  187,	188,  189,	189,  192,	193,  194,	194,  197,	196,
			201,  202,	203,  204,	205,  206,	204,  208,	215,  216,	217,
			218,  219,	219,  222,	223,  224,	225,  223,	230,  231,	231,
			234,  235,	236,  237,	238,  239,	240,  241,	242,  195,	285,
			289,  287,	261,  404,	419,  400,	442,  521,	518,  499,	536,
			535,  564,	669,  686,	648,  685,	712,  807,	847,  845,	872,
			858,  889,	890,  891,	892,  893,	894,  895,	896,  897,	898,
			899,  900,	901,  902,	903,  904,	905,  906,	907,  908,	909,
			910,  911,	912,  913,	914,  915,	916,  917,	918,  919,	920,
			921,  922,	923,  924,	925,  926,	927,  928,	929,  930,	931,
			932,  933,	934,  935,	936,  937,	938,  939,	940,  941,	942,
			943,  944,	945,  946,	947,  948,	949,  950,	951,  952,	953,
			954,  955,	956,  957,	958,  959,	960,  961,	962,  963,	964,
			965,  966,	967,  968,	969,  970,	971,  972,	973,  974,	975,
			976,  977,	978,  979,	980,  981,	982,  983,	984,  985,	986,
			987,  988,	989,  990,	991,  992,	993,  994,	995,  996,	997,
			998,  999,	1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008,
			1009, 1010, 1011, 1012, 1013, 1014, 1015, 1016, 1017, 1018, 1019,
			1020, 1021, 1022};

		const state_t adj_start[ALPHABET_LENGTH] = {
			1023, 0,	1,	  2,	3,	  4,	5,	  6,	7,	  8,	1011,
			32,	  33,	34,	  35,	36,	  37,	38,	  39,	40,	  41,	42,
			43,	  44,	45,	  46,	47,	  48,	49,	  50,	51,	  52,	1002,
			129,  130,	131,  132,	133,  134,	130,  141,	142,  143,	144,
			130,  161,	147,  178,	179,  180,	181,  182,	183,  184,	185,
			186,  187,	188,  189,	189,  192,	193,  194,	194,  197,	196,
			201,  202,	203,  204,	205,  206,	204,  208,	215,  216,	217,
			218,  219,	219,  222,	223,  224,	225,  223,	230,  231,	231,
			234,  235,	236,  237,	238,  239,	240,  241,	242,  195,	285,
			289,  287,	261,  404,	419,  400,	442,  521,	518,  499,	536,
			535,  564,	669,  686,	648,  685,	712,  807,	847,  845,	872,
			858,  889,	890,  891,	892,  893,	894,  895,	896,  897,	898,
			899,  900,	901,  902,	903,  904,	905,  906,	907,  908,	909,
			910,  911,	912,  913,	914,  915,	916,  917,	918,  919,	920,
			921,  922,	923,  924,	925,  926,	927,  928,	929,  930,	931,
			932,  933,	934,  935,	936,  937,	938,  939,	940,  941,	942,
			943,  944,	945,  946,	947,  948,	949,  950,	951,  952,	953,
			954,  955,	956,  957,	958,  959,	960,  961,	962,  963,	964,
			965,  966,	967,  968,	969,  970,	971,  972,	973,  974,	975,
			976,  977,	978,  979,	980,  981,	982,  983,	984,  985,	986,
			987,  988,	989,  990,	991,  992,	993,  994,	995,  996,	997,
			998,  999,	1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008,
			1009, 1010, 1011, 1012, 1013, 1014, 1015, 1016, 1017, 1018, 1019,
			1020, 1021, 1022};

	} // namespace StaticTable
} // namespace ANS
