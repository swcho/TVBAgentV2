//=================================================================	
//	tvb360u.c / Modulator Manager for TVB370/380/390/590(E,S)/595/597A
//
//	Copyright (C) 2009
//	Teleview Corporation
//
//	Author : 
//  	Last Modified : November, 2009
//=================================================================	

//=================================================================	
#ifdef WIN32
#include	<stdio.h>
#include	<stdlib.h>
#include	<conio.h>
#include	<windows.h>
#include	<winioctl.h>
#include	<io.h>
#include	<fcntl.h>
#include	<sys/stat.h>
#include	<commctrl.h>
#include	<stdio.h>
#include	<time.h>
#include	<setupapi.h>
#include	<memory.h>
#include	<math.h>
#else	// Linux
#define _FILE_OFFSET_BITS 64
#include	<stdio.h>
#include	<stdlib.h>
#include	<fcntl.h>
#include	<sys/stat.h>
#include	<time.h>
#include	<memory.h>
#include	<math.h>
#endif
#include	"interp.h"

/////////////////////////////////////////////////////////////////
CUtilInterp::CUtilInterp(void)
{
	dbg_notice = 0;
	dbg_warning = 0;
	dbg_noisy = 0;

}
CUtilInterp::~CUtilInterp()
{
}


//TVB593 - UMC FREQ. OPTIMIZE
void CUtilInterp::Cubic_Interp(double *x, double *y, int count, double *result, int result_len)
{
	int numRows = count;
	double dummy, tempx;
	double c2, c2t2, c3, c3t3, delta, del1, del2, h, xma, xmi;
	int xval_kount = result_len;
	int ir = 1, i, j, jfirst = 0, k, nj;
	double *xi, *yi, *dVec, *wk[2], *xInt, *yInt;
	int next[2];

	xi = (double*)calloc(numRows, sizeof(double)*numRows);
	yi = (double*)calloc(numRows, sizeof(double)*numRows);
	dVec = (double*)calloc(numRows, sizeof(double)*numRows);
	wk[0] = (double*)calloc(numRows, sizeof(double)*numRows);
	wk[1] = (double*)calloc(numRows, sizeof(double)*numRows);
	xInt = (double*)calloc(xval_kount, sizeof(double)*xval_kount);
	yInt = (double*)calloc(xval_kount, sizeof(double)*xval_kount);
	if ( !xi || !yi || !dVec || !wk[0] || !wk[1] || !xInt || !yInt )
	{
		if ( xi ) free(xi);
		if ( yi ) free(yi);
		if ( dVec ) free(dVec);
		if ( wk[0] ) free(wk[0]);
		if ( wk[1] ) free(wk[1]);
		if ( xInt ) free(xInt);
		if ( xInt ) free(yInt);

		printf("CSig::Cubic_Interp, fail to calloc.!!!\n");
	//	result = NULL;
		return;
	}

	for ( i = 0; i < xval_kount; i++ )
	{
		xInt[i] = i;
	}

	for (i = 0; i < numRows; i++) 
	{
		xi[i] = x[i];
		yi[i] = y[i];
	}
	

	 j = 1;
	 for (i = 0; i < (numRows - 1); i++)
	 {
		wk[0][j] = xi[j] - xi[i];
		wk[1][j] = (yi[j] - yi[i])/wk[0][j];
		j++;
	 }//End for i

	 if (numRows == 2)
	 {
		wk[1][0] = wk[0][0] = 1.0;
		dVec[0] = 2.0*wk[1][1];
	 }// End if (numRows == 2)
	 else 
	 { // else numRows > 2
		tempx = dummy = wk[0][1];
		wk[1][0] = wk[0][2];
		wk[0][0] = tempx + wk[0][2];
		//dummy *= dummy*wk[1][2];
		dummy *= (dummy*wk[1][2]);
		dVec[0] = ((tempx + 2.0*wk[0][0])*wk[1][1]*wk[0][2] + dummy)/wk[0][0];
	 } // End else numRows > 2

	 nj = numRows - 1;
	 for (i = 1; i < nj; i++)
	 {
		tempx = -(wk[0][i+1]/wk[1][i-1]);
		dVec[i] = tempx*dVec[i-1] + 3.0*(wk[0][i]*wk[1][i+1] + wk[0][i+1]*wk[1][i]);
		wk[1][i] = tempx*wk[0][i-1] + 2.0*(wk[0][i] + wk[0][i+1]);
	 }//End for i

	 if (numRows == 2)
	 {
		dVec[1] = wk[1][1];
	 } // End if numRows == 2
	 else 
	 { //else numRows != 2
		if (numRows == 3)
		{
			dVec[2] = 2.0*wk[1][2];
			wk[1][2] = 1.0;
			tempx = -(1.0/wk[1][1]);
		}// End if (numRows == 3)
		else 
		{
			tempx = wk[0][numRows-2] + wk[0][numRows-1];
			dummy = wk[0][numRows-1]*wk[0][numRows-1]*(yi[numRows-2] - yi[numRows-3]);
			dummy /= wk[0][numRows-2];
			dVec[numRows-1] = ((wk[0][numRows-1] + 2.0*tempx)*wk[1][numRows-1]*wk[0][numRows-2] + dummy)/tempx;
			tempx = -(tempx/wk[1][numRows-2]);
			wk[1][numRows-1] = wk[0][numRows-2];
		}//End else

		// Complete forward pass of Gauss Elimination

		wk[1][numRows-1] = tempx*wk[0][numRows-2] + wk[1][numRows-1];
		dVec[numRows-1] = (tempx*dVec[numRows-2] + dVec[numRows-1])/wk[1][numRows-1];

	} // End else numRows != 2

	 //Carry out back substitution

	for (i= numRows-2; i >= 0; i--)
	{
		dVec[i] = (dVec[i] - wk[0][i]*dVec[i+1])/wk[1][i];
	}//End for i
	 // End of PCHEZ

	 // Begin PCHEV
	 // Main loop. Go through and calculate interpolant at each xInt value
	 while (jfirst < xval_kount)
	 {
		// Locate all points in interval

		for (k = jfirst; k < xval_kount; k++)
		{
			if (xInt[k] >= xi[ir]) break;
		} // End for k loop

		if (k < xval_kount)
		{
			if (ir == (numRows-1))
				k = xval_kount;
		} // End if (k < xval_kount)

		nj = k - jfirst;

		// Skip evaluation if no points in interval

		if (nj > 0)
		{

			// Evaluate Cubic at xInt[k], j = jfirst (1) to k-1
			// =========================================================
			// Begin CHFDV

			xma = h = xi[ir] - xi[ir - 1];

			next[1] = next[0] = 0;
			xmi = 0.0;

			// Compute Cubic Coefficients (expanded about x1)

			delta = (yi[ir] - yi[ir - 1])/h;
			del1 = (dVec[ir - 1] - delta)/h;
			del2 = (dVec[ir] - delta)/h;

			//delta is no longer needed

			c2 = -(del1 + del1 + del2);
			c2t2 = c2 + c2;
			c3 = (del1 + del2)/h;

			// h, del1, and del2 are no longer needed

			c3t3 = c3 + c3 + c3;

			// Evaluation loop

			for (j = 0; j < nj; j++)
			{
				dummy = xInt[jfirst + j] - xi[ir - 1];
				yInt[jfirst + j] = yi[ir - 1] + dummy*(dVec[ir - 1] + dummy*(c2 + dummy*c3));
				if (dummy < xmi) next[0] = next[0] + 1;
				if (dummy > xma) next[1] = next[1] + 1;
				// Note the redundancy: if either condition is true, other is false
			} // End for j loop

			// End CHFDV

			// ========================================================

			if ((next[1] > 0) && (ir != (numRows - 1))) 
			{
			//	alert("Error Code 5005 for next[1]. No further action taken.");  // This option should never happen.
				return;
			} // End if ((next[1] > 0) && (ir != (numRows - 1)))

			if ((next[0] > 0)  && (ir != 1)) 
			{
				// xInt is not ordered relative to xi, so must adjust evaluation interval
				// First, locate first point to left of xi[ir - 1]
				for (j = jfirst; j < k; j++)
				{
					if (xInt[j] < xi[ir - 1])
						break;
				} //End for j
				if (j == k)
				{
				//	alert("Error Code 5005 for next[0]. No further action taken.");  // This option should never happen.
					return;
				}
				k = j; //Reset k. This will be the first new jfirst

				// Now find out how far to back up in the xi array

				for (j = 0; j < ir; j++)
				{
					if (xInt[k] < xi[j])
						break;
				} // End for j

				// The above loop should NEVER run to completion because xInt[k] < xi[ir - 1]

				// At this point, either xInt[k] < xi[0] or
				//                       xi[j-1] <= xInt[k] < xi[j]
				// Reset ir, recognizing that it will be incremented before cycling

				ir = (((j-1) > 0) ? (j-1) : 0);

			} // End if ((next[0] > 0)  && (ir != 1))

			jfirst = k;

		} // End if (nj > 0)

		ir++;
		if (ir >= numRows) break;

	  }// End while (jfirst < xval_kount)

	  // End PCHEV

	  /*
	  //TEST
	  {
		  for ( i = xval_kount/2; i < xval_kount; i++ )
		  {
			  yInt[i] += .075;
		  }
		  for ( i = 0; i < xval_kount/2; i++ )
		  {
			  yInt[i] -= .075;
		  }

		  for ( i = 0; i < 16; i++ )
		  {
			  yInt[i] += .09;
		  }

		  for ( i = xval_kount-32-1; i < xval_kount; i++ )
		  {
			  yInt[i] += .09;
		  }
	  }
	  */
	  memcpy(result, yInt, sizeof(double)*xval_kount);

	  free(xi);
	  free(yi);
	  free(xInt);
	  free(yInt);
	  free(dVec);
	  free(wk[0]);
	  free(wk[1]);
}

void CUtilInterp::DoInterp(int target, int *sample, int sample_count, int interp_len, double *result, int result_len, int interpolation_type)
{
	double fval;
	int i, val;
	int nSampleInterval = 6;
	double *xi, *yi, *interp;
	xi = (double*)calloc(interp_len, sizeof(double));
	yi = (double*)calloc(interp_len, sizeof(double));
	interp = (double*)calloc(result_len, sizeof(double));

	if ( !xi || !yi || !interp )
	{
		if ( xi ) free(xi);
		if ( yi ) free(yi);
		if ( interp ) free(interp);

		return;
	}

	for (i = 0; i < sample_count; i++) 
	{
		val = sample[i];
		fval = val;

		xi[i] = i*nSampleInterval;
		yi[i] = fval;
	}

	Cubic_Interp(xi, yi, sample_count, interp, interp_len);
	memcpy(result, interp, sizeof(double)*result_len);

	free(interp);
	free(xi);
	free(yi);
}


