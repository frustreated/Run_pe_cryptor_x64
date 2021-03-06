#include <stdint.h>
#include <string.h>

#define BLOCK_SIZE 16

static int rol(int a, int n)
{
	int t1, t2;
	n = n % (sizeof(a) * 8);  // нормализуем n
	t1 = a << n;   // двигаем а вправо на n бит, теряя старшие биты
	t2 = a >> (sizeof(a) * 8 - n); // перегоняем старшие биты в младшие
	return t1 | t2;  // объединяем старшие и младшие биты

}

static void xtea3_encipher(unsigned int num_rounds, uint32_t *v, uint32_t *k)
{
	unsigned int i;
	uint32_t a, b, c, d, sum = 0, t, delta = 0x9E3779B9;
	sum = 0;
	a = v[0] + k[0];
	b = v[1] + k[1];
	c = v[2] + k[2];
	d = v[3] + k[3];
	for (i = 0; i < num_rounds; i++) {
		a += (((b << 4) + rol(k[(sum % 4) + 4], b)) ^
			(d + sum) ^ ((b >> 5) + rol(k[sum % 4], b >> 27)));
		sum += delta;
		c += (((d << 4) + rol(k[((sum >> 11) % 4) + 4], d)) ^
			(b + sum) ^ ((d >> 5) + rol(k[(sum >> 11) % 4], d >> 27)));
		t = a; a = b; b = c; c = d; d = t;
	}
	v[0] = a ^ k[4];
	v[1] = b ^ k[5];
	v[2] = c ^ k[6];
	v[3] = d ^ k[7];
}

static void xtea3_decipher(unsigned int num_rounds, uint32_t *v, uint32_t *k)
{
	unsigned int i;
	uint32_t a, b, c, d, t, delta = 0x9E3779B9, sum = delta * num_rounds;
	d = v[3] ^ k[7];
	c = v[2] ^ k[6];
	b = v[1] ^ k[5];
	a = v[0] ^ k[4];
	for (i = 0; i < num_rounds; i++) {
		t = d; d = c; c = b; b = a; a = t;
		c -= (((d << 4) + rol(k[((sum >> 11) % 4) + 4], d)) ^
			(b + sum) ^ ((d >> 5) + rol(k[(sum >> 11) % 4], d >> 27)));
		sum -= delta;
		a -= (((b << 4) + rol(k[(sum % 4) + 4], b)) ^
			(d + sum) ^ ((b >> 5) + rol(k[sum % 4], b >> 27)));
	}
	v[3] = d - k[3];
	v[2] = c - k[2];
	v[1] = b - k[1];
	v[0] = a - k[0];
}

void Xtea3_Data_Crypt(char *inout, int len, bool encrypt, uint32_t *key)
{
	static unsigned char dataArray[BLOCK_SIZE];

	for (int i = 0; i < len / BLOCK_SIZE; i++)
	{
		memcpy(dataArray, inout, BLOCK_SIZE);
		
		if (encrypt)
			xtea3_encipher(32, (uint32_t*)dataArray, key);
		else
			xtea3_decipher(32, (uint32_t*)dataArray, key);

		memcpy(inout, dataArray, BLOCK_SIZE);
		inout = inout + BLOCK_SIZE;
	}

	if (len%BLOCK_SIZE != 0)
	{
		int mod = len % BLOCK_SIZE;
		int offset = (len / BLOCK_SIZE)*BLOCK_SIZE;
		uint32_t data[BLOCK_SIZE];
		memcpy(data, inout + offset, mod);
		if (encrypt)
			xtea3_encipher(32, (uint32_t*)data, key);
		else
			xtea3_decipher(32, (uint32_t*)data, key);
		memcpy(inout + offset, data, mod);
	}
}