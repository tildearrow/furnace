/*
	The function to compute the layer III tables at runtime or for pregeneration.

	copyright ?-2021 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Michael Hipp (in layer3.c).

	The type clreal is either double or float.
*/

// init tables for layer-3 ... specific with the downsampling...
void INT123_init_layer3(void)
{
	int i,j,k,l;

	for(i=0;i<8207;i++)
	ispow[i] = POW((clreal)i,(clreal)4.0/3.0);

	for(i=0;i<8;i++)
	{
		const clreal Ci[8] = {-0.6,-0.535,-0.33,-0.185,-0.095,-0.041,-0.0142,-0.0037};
		clreal sq = sqrt(1.0+Ci[i]*Ci[i]);
		aa_cs[i] = 1.0/sq;
		aa_ca[i] = Ci[i]/sq;
	}

	for(i=0;i<18;i++)
	{
		win[0][i]    = win[1][i]    =
			0.5*SIN(M_PI/72.0 * (clreal)(2*(i+0) +1)) / COS(M_PI * (clreal)(2*(i+0) +19) / 72.0);
		win[0][i+18] = win[3][i+18] =
			0.5*SIN(M_PI/72.0 * (clreal)(2*(i+18)+1)) / COS(M_PI * (clreal)(2*(i+18)+19) / 72.0);
	}
	for(i=0;i<6;i++)
	{
		win[1][i+18] = 0.5 / COS( M_PI * (clreal) (2*(i+18)+19) / 72.0 );
		win[3][i+12] = 0.5 / COS( M_PI * (clreal) (2*(i+12)+19) / 72.0 );
		win[1][i+24] = 0.5 * SIN( M_PI / 24.0 * (clreal) (2*i+13) ) / COS( M_PI * (clreal) (2*(i+24)+19) / 72.0 );
		win[1][i+30] = win[3][i] = 0.0;
		win[3][i+6 ] = 0.5 * SIN( M_PI / 24.0 * (clreal) (2*i+1 ) ) / COS( M_PI * (clreal) (2*(i+6 )+19) / 72.0 );
	}

	for(i=0;i<9;i++)
	INT123_COS9[i] = COS( M_PI / 18.0 * (clreal) i);

	for(i=0;i<9;i++)
	INT123_tfcos36[i] = 0.5 / COS( M_PI * (clreal) (i*2+1) / 36.0 );

	for(i=0;i<3;i++)
	tfcos12[i] = 0.5 / COS( M_PI * (clreal) (i*2+1) / 12.0 );

	COS6_1 = COS( M_PI / 6.0 * (clreal) 1);
	COS6_2 = COS( M_PI / 6.0 * (clreal) 2);

	cos9[0]  = COS(1.0*M_PI/9.0);
	cos9[1]  = COS(5.0*M_PI/9.0);
	cos9[2]  = COS(7.0*M_PI/9.0);
	cos18[0] = COS(1.0*M_PI/18.0);
	cos18[1] = COS(11.0*M_PI/18.0);
	cos18[2] = COS(13.0*M_PI/18.0);

	for(i=0;i<12;i++)
	{
		win[2][i] = 0.5 * SIN( M_PI / 24.0 * (clreal) (2*i+1) ) / COS( M_PI * (clreal) (2*i+7) / 24.0 );
	}

	for(i=0;i<16;i++)
	{
		// Special-casing possibly troublesome values where t=inf or
		// t=-1 in theory. In practice, this never caused issues, but there might
		// be a system with enough precision in M_PI to raise an exception.
		// Actually, the special values are not excluded from use in the code, but
		// in practice, they even have no effect in the compliance tests.
		if(i > 11) // It's periodic!
		{
			tan1_1[i] = tan1_1[i-12];
			tan2_1[i] = tan2_1[i-12];
			tan1_2[i] = tan1_2[i-12];
			tan2_2[i] = tan2_2[i-12];
		} else if(i == 6) // t=inf
		{
			tan1_1[i] = 1.0;
			tan2_1[i] = 0.0;
			tan1_2[i] = M_SQRT2;
			tan2_2[i] = 0.0;
		} else if(i == 9) // t=-1
		{
			tan1_1[i] = -HUGE_VAL;
			tan2_1[i] = HUGE_VAL;
			tan1_2[i] = -HUGE_VAL;
			tan2_2[i] = HUGE_VAL;
		} else
		{
			clreal t = TAN( (clreal) i * M_PI / 12.0 );
			tan1_1[i] = t / (1.0+t);
			tan2_1[i] = 1.0 / (1.0 + t);
			tan1_2[i] = M_SQRT2 * t / (1.0+t);
			tan2_2[i] = M_SQRT2 / (1.0 + t);
		}
	}

	for(i=0;i<32;i++)
	{
		for(j=0;j<2;j++)
		{
			clreal base = POW(2.0,-0.25*(j+1.0));
			clreal p1=1.0,p2=1.0;
			if(i > 0)
			{
				if( i & 1 ) p1 = POW(base,(i+1.0)*0.5);
				else p2 = POW(base,i*0.5);
			}
			pow1_1[j][i] = p1;
			pow2_1[j][i] = p2;
			pow1_2[j][i] = M_SQRT2 * p1;
			pow2_2[j][i] = M_SQRT2 * p2;
		}
	}

	for(j=0;j<4;j++)
	{
		const int len[4] = { 36,36,12,36 };
		for(i=0;i<len[j];i+=2) win1[j][i] = + win[j][i];

		for(i=1;i<len[j];i+=2) win1[j][i] = - win[j][i];
	}

	for(j=0;j<9;j++)
	{
		const struct bandInfoStruct *bi = &bandInfo[j];
		short *mp;
		short cb,lwin;
		const unsigned char *bdf;
		int switch_idx;

		mp = map[j][0] = mapbuf0[j];
		bdf = bi->longDiff;
		switch_idx = (j < 3) ? 8 : 6;
		for(i=0,cb = 0; cb < switch_idx ; cb++,i+=*bdf++)
		{
			*mp++ = (*bdf) >> 1;
			*mp++ = i;
			*mp++ = 3;
			*mp++ = cb;
		}
		bdf = bi->shortDiff+3;
		for(cb=3;cb<13;cb++)
		{
			int l = (*bdf++) >> 1;
			for(lwin=0;lwin<3;lwin++)
			{
				*mp++ = l;
				*mp++ = i + lwin;
				*mp++ = lwin;
				*mp++ = cb;
			}
			i += 6*l;
		}
		mapend[j][0] = mp;

		mp = map[j][1] = mapbuf1[j];
		bdf = bi->shortDiff+0;
		for(i=0,cb=0;cb<13;cb++)
		{
			int l = (*bdf++) >> 1;
			for(lwin=0;lwin<3;lwin++)
			{
				*mp++ = l;
				*mp++ = i + lwin;
				*mp++ = lwin;
				*mp++ = cb;
			}
			i += 6*l;
		}
		mapend[j][1] = mp;

		mp = map[j][2] = mapbuf2[j];
		bdf = bi->longDiff;
		for(cb = 0; cb < 22 ; cb++)
		{
			*mp++ = (*bdf++) >> 1;
			*mp++ = cb;
		}
		mapend[j][2] = mp;
	}

	/* Now for some serious loopings! */
	for(i=0;i<5;i++)
	for(j=0;j<6;j++)
	for(k=0;k<6;k++)
	{
		int n = k + j * 6 + i * 36;
		i_slen2[n] = i|(j<<3)|(k<<6)|(3<<12);
	}
	for(i=0;i<4;i++)
	for(j=0;j<4;j++)
	for(k=0;k<4;k++)
	{
		int n = k + j * 4 + i * 16;
		i_slen2[n+180] = i|(j<<3)|(k<<6)|(4<<12);
	}
	for(i=0;i<4;i++)
	for(j=0;j<3;j++)
	{
		int n = j + i * 3;
		i_slen2[n+244] = i|(j<<3) | (5<<12);
		n_slen2[n+500] = i|(j<<3) | (2<<12) | (1<<15);
	}
	for(i=0;i<5;i++)
	for(j=0;j<5;j++)
	for(k=0;k<4;k++)
	for(l=0;l<4;l++)
	{
		int n = l + k * 4 + j * 16 + i * 80;
		n_slen2[n] = i|(j<<3)|(k<<6)|(l<<9)|(0<<12);
	}
	for(i=0;i<5;i++)
	for(j=0;j<5;j++)
	for(k=0;k<4;k++)
	{
		int n = k + j * 4 + i * 20;
		n_slen2[n+400] = i|(j<<3)|(k<<6)|(1<<12);
	}

#ifdef CALCTABLES
// Only needed for fixed point.
	for(int i=-256;i<118+4;i++)
		gainpow2[i+256] =
			DOUBLE_TO_REAL_SCALE_LAYER3(POW((clreal)2.0,-0.25 * (clreal) (i+210)),i+256);
#endif
}
