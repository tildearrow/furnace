/*
	The function to compute the cos tables at runtime or for pregeneration.

	copyright ?-2021 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Michael Hipp (in tabinit.c).

	The type clreal is either double or float, COS() is cos() or cosf() for runtime.
*/

void INT123_init_costabs(void)
{
	clreal *cpnts[] = { cos64,cos32,cos16,cos8,cos4 };
	int i,k,kr,divv;
	clreal *costab;

	for(i=0;i<5;i++)
	{
		kr=0x10>>i; divv=0x40>>i;
		costab = cpnts[i];
		for(k=0;k<kr;k++)
			costab[k] = 1.0 / (2.0 * COS(M_PI * ((clreal) k * 2.0 + 1.0) / (clreal) divv));
	}
}
