#include <QCoreApplication>
#include <QFile>
#include <iostream>
#include <math.h>
#include <QtEndian>
#define SIZE_OF_BLOCK 64

using namespace std;

unsigned int shl(unsigned int a, unsigned int s)
{
    return ((a<<s)|(a>>(32-s)));
}

int main(int argc, char *argv[])
{
    //==========================================================================
    //Открываем файл
    QFile file(argv[1]);
    if (!file.open(QIODevice::ReadWrite))
    {
        cout << "Can't open file\n" << endl;
        exit(0);
    }
    //==========================================================================
    //Вычисляем, сколько байт необходимо дописать,
    //сколько в итоге будет блоков по 64 байта и запоминаем размер файла
    //дописываем байты в конец файла
    qint64 file_size = file.size();
    //cout << "file size\t\t = " << file_size << " bytes" << endl;

    int size_of_last_block = file_size % SIZE_OF_BLOCK;
    //cout << "size of last block\t = " << size_of_last_block << " bytes" << endl;

    int number_of_zero_bytes;
    if (size_of_last_block < 56) number_of_zero_bytes = 56 - size_of_last_block;
    else number_of_zero_bytes = SIZE_OF_BLOCK - size_of_last_block + 56;
    //cout << "number of zero bytes\t = " << number_of_zero_bytes << endl;

    qint64 num_of_blocks = (file_size + number_of_zero_bytes + 8) / SIZE_OF_BLOCK;
    //cout << "number of blocks\t = " << num_of_blocks << endl;

    //Дописываем в конец нужные байты
    file.seek(file_size);
    char buf;
    buf = 0x80;
    file.write(&buf, 1);
    buf = 0;
    for(int i = 0; i < number_of_zero_bytes - 1; i++)
    {
        file.write(&buf, 1);
    }

    qint64 bit_file_size = file_size*8;
    char bb[8];
    for(int i = 0; i < 8; i++)
    {
        bb[i] = bit_file_size & 0xFF;
        bit_file_size /= 256;
    }
    file.write(bb, 8);
    file.close();
    //==========================================================================
    //Мутим преобразования
    //НАчальные значения констант
    //Note: All variables are unsigned 32 bit and wrap modulo 2^32 when calculating

    //s specifies the per-round shift amounts
    unsigned int s[64] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
                 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

    //Use binary integer part of the sines of integers (Radians) as constants:
    //(Or just use the following table):
    unsigned int k[64] = {0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee ,
                 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501 ,
                 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be ,
                 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821 ,
                 0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa ,
                 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8 ,
                 0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed ,
                 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a ,
                 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c ,
                 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70 ,
                 0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05 ,
                 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665 ,
                 0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039 ,
                 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1 ,
                 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1 ,
                 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 };

    //Initialize variables:
     unsigned int h0 = 0x67452301;   //A
     unsigned int h1 = 0xefcdab89;   //B
     unsigned int h2 = 0x98badcfe;   //C
     unsigned int h3 = 0x10325476;   //D

    //Process the message in successive 512-bit chunks:
     FILE *ff = fopen(argv[1], "rb");
     unsigned int w[16], f, g, a, b, c, d, temp;
     for(qint64 i = 0 ; i < num_of_blocks; i++)
     {
         fread(w, 4, 16, ff);
        //Initialize hash value for this chunk:
        a = h0;
        b = h1;
        c = h2;
        d = h3;
        //Main loop:
        for(int i = 0; i < 64; i++)
        {
            if (i >= 0 && i <= 15)
            {
                f = (b & c) | ((~ b) & d);
                g = i;
            }
            else if (i >= 16 && i <= 31)
            {
                f = (d & b) | ((~ d) & c);
                g = (5*i + 1)%16;
            }
            else if (i >= 32 && i <= 47)
            {
                f = b ^ c ^ d;
                g = (3*i + 5)%16;
            }
            else if (i >= 48 && i <= 63)
            {
                f = c ^ (b | (~ d));
                g = (7*i)%16;
            }
            temp = d;
            d = c;
            c = b;
            b = b + shl(a + f + k[i] + w[g], s[i]);
            a = temp;
        }
        //Add this chunk's hash to result so far:
        h0 = h0 + a;
        h1 = h1 + b;
        h2 = h2 + c;
        h3 = h3 + d;
     }
    fclose(ff);
    file.resize(file_size);
    //И снова через )|(опу
    //О Qt всемогущий!!
    cout << "MD5 = " << hex << qToBigEndian(h0) << " " << hex << qToBigEndian(h1) << " " << hex << qToBigEndian(h2) << " " << hex << qToBigEndian(h3) << endl;
    return 0;
    //return a.exec();
}
