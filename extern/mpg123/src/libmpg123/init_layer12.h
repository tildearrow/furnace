/*
	The function to compute the layer I and II tables at runtime or for pregeneration.

	copyright ?-2021 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Michael Hipp (in layer2.c).

	The type clreal is either double or float. POW() is pow() or powf().
*/

void INT123_init_layer12(void)
{
	const clreal mulmul[27] =
	{
		0.0 , -2.0/3.0 , 2.0/3.0 ,
		2.0/7.0 , 2.0/15.0 , 2.0/31.0, 2.0/63.0 , 2.0/127.0 , 2.0/255.0 ,
		2.0/511.0 , 2.0/1023.0 , 2.0/2047.0 , 2.0/4095.0 , 2.0/8191.0 ,
		2.0/16383.0 , 2.0/32767.0 , 2.0/65535.0 ,
		-4.0/5.0 , -2.0/5.0 , 2.0/5.0, 4.0/5.0 ,
		-8.0/9.0 , -4.0/9.0 , -2.0/9.0 , 2.0/9.0 , 4.0/9.0 , 8.0/9.0
	};
	// void INT123_init_layer12_stuff()
	// real* INT123_init_layer12_table()
	for(int k=0;k<27;k++)
	{
		int i,j;
		clreal *table = layer12_table[k];
		for(j=3,i=0;i<63;i++,j--)
			*table++ = mulmul[k] * POW(2.0,(clreal) j / 3.0);
		*table++ = 0.0;
	}

	// void INT123_init_layer12()
	const unsigned char base[3][9] =
	{
		{ 1 , 0, 2 , } ,
		{ 17, 18, 0 , 19, 20 , } ,
		{ 21, 1, 22, 23, 0, 24, 25, 2, 26 }
	};
	int i,j,k,l,len;
	const int tablen[3] = { 3 , 5 , 9 };
	unsigned char *itable;
	unsigned char *tables[3] = { grp_3tab , grp_5tab , grp_9tab };

	for(i=0;i<3;i++)
	{
		itable = tables[i];
		len = tablen[i];
		for(j=0;j<len;j++)
		for(k=0;k<len;k++)
		for(l=0;l<len;l++)
		{
			*itable++ = base[i][l];
			*itable++ = base[i][k];
			*itable++ = base[i][j];
		}
	}
}
