/****************************************************************
* NUMERICAL DECONVOLUTION OF A SUSPENDED CAPTOR'S MASS ACCELE-  *
* RATION RESPONSE SIGNAL  TO OBTAIN SPEED AT BASIS AND COMPUTE  *
* THE ACCELERATION SHOCK SPECTRUM OF THIS SPEED WITH OR WITHOUT *
* SIGNAL FILTERING OF HIGH FREQUENCIES                          *
* ------------------------------------------------------------- *
* The shock captor is considered as a damped elementary oscil-  *
* lator of one degree of freedom (M,K) with a resonance fre-    *
* quency F0 and a relative damping factor dzeta. Q is the       *
* quality factor used to compute the shock spectrum.            *
*                                                               *
*                            C++ version by J-P Moreau, Paris   *
*                                (in double precision)          *
*                                  (www.jpmoreau.fr)            *
*                                                               *
* SAMPLE RUN:                                                   *
*                                                               *
* Read data from file signal.dat                                *
* Write results to file vsignal.lst.                            *
****************************************************************/
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <basis.h>
#include <vmblock.h>


int      i,iordre,iordre1,ndata,nspec;
FILE     *fp_in, *fp_out;
REAL     df,dzeta,f0,fc,fc1,finf,fmax,fr,fsup,q,temp,temp1,ts;
char     Title[80];
char     input[13],output[13];
char     nom[9];

REAL     *ACC,*VIT,*S;
void     *vmblock;
                               

    void DECON(REAL frequence, REAL * Signal, REAL * x_y,
	           REAL Sampling_Incr, REAL Dzeta, int Ndata) {
      REAL Omega,Omega2,Q,OmegaQ,OmegaT,OmegaQT,
      q0,q1,q2,T,T2sur6,expA,yn,ypn,ynm1,
      xn,xpn,xnm1,xpnm1,xppn,xppnm1;
      int i;
      Omega=2*PI*frequence;
      Omega2=Omega*Omega;
      Q=1/(2*Dzeta);
      T=Sampling_Incr;
      T2sur6 =T*T/6;
      OmegaT =Omega*T;
      OmegaQ =Omega*Q;
      OmegaQT=Omega*Q*T;
      expA   =(exp(-OmegaQT));
      q2=OmegaQ*expA;
      q0=(1-expA)/(Omega*OmegaT)+T/2;
      q1=T+Q/Omega*expA-q0;
      yn=0;xn=0;
      xpn=0;xppn=Signal[1];
      x_y[1]=xpn;
      for (i=2; i<Ndata+1; i++) {
        ynm1=yn;xnm1=xn;xpnm1=xpn;xppnm1=xppn;
        xppn=Signal[i];
        ypn=xpnm1+q0*xppn+q1*xppnm1+q2*(xnm1-ynm1);
        xpn= (xpnm1+0.5*T*(xppn+xppnm1));
        xn=xnm1+T*xpnm1+T2sur6*(2*xppnm1+xppn);
        yn=xn+xppn/Omega2+(xpn-ypn)/OmegaQ;
        x_y[i]=ypn;
      }
    }

    void OSCIL(REAL FREQU,REAL * VIT,REAL * ACC,REAL T,REAL DZETA,int NDATA) {
      REAL ARG,COSARG,DELTA,EXPA,GOMEGA,OMEGA,OMEGA2,SINARG;
      REAL Q0,Q1,Q2,QSI,R0,R1,R2,R3,XPN,XPNP1,YPN,YPPN,YPNP1,YPPNP1;
      int   I;
      OMEGA= (TWO*PI*FREQU);
      OMEGA2=OMEGA*OMEGA;
      DELTA= (sqrt(ONE-DZETA*DZETA));
      EXPA=(exp(-OMEGA*DZETA*T));
      GOMEGA=OMEGA*DELTA;
      ARG=GOMEGA*T;
      SINARG=sin(ARG);
      COSARG=cos(ARG);
      QSI=DZETA/DELTA;
      Q0=(COSARG-QSI*SINARG)*EXPA;
      Q1=OMEGA2*SINARG/GOMEGA*EXPA;
      Q2=((ONE+(QSI*SINARG-COSARG)*EXPA)/T);
      R1=SINARG/GOMEGA*EXPA;
      R0=COSARG*EXPA+DZETA*OMEGA*R1;
      R2=((ONE-DZETA*OMEGA*T)*R1/T-COSARG*EXPA);
      R3= (ONE-R1/T);
      XPNP1=VIT[1];
      YPPNP1=ZERO;
      YPNP1=ZERO;
      ACC[1]=Q2*XPNP1;
                                                                                
      for (I=2; I<NDATA+1; I++) {
        YPN=YPNP1;
        YPPN=YPPNP1;
        XPN=XPNP1;
        XPNP1=VIT[I];
        YPPNP1=Q0*YPPN+Q1*(XPN-YPN)+Q2*(XPNP1-XPN);
        ACC[I]=YPPNP1;
        YPNP1=R0*YPN+R1*YPPN+R2*XPN+R3*XPNP1;
      }                                                                          
    }                                                                       
                                                                                
    void EXTREM(REAL * C,REAL *CMIN,REAL *CMAX,int NDATA) {
      int I;
      *CMIN=ZERO;
      *CMAX=ZERO;
      for (I=1; I<NDATA+1; I++) {
        if (C[I] > *CMAX)  *CMAX=C[I];
        if (C[I] < *CMIN)  *CMIN=C[I];
      }                                                                    
    }                                                                       


    void SPEC(REAL * VIT,REAL DZETA,REAL FMIN,REAL FMAX,int N,REAL T,int NFREQ,REAL * S) {
      REAL DELTAF,F,YMIN,YMAX; int I;
      DELTAF=(FMAX-FMIN)/(NFREQ-1);
      //the algorithm fails if FMIN=0
      if (FMIN < DELTAF)  FMIN=DELTAF;
      for (I=1; I<NFREQ+1; I++) {
        F=FMIN+(I-1)*DELTAF;
        OSCIL(F,VIT,ACC,T,DZETA,N);                                           
        EXTREM(ACC,&YMIN,&YMAX,N);                                              
        S[2*I-1]=YMAX;
        S[2*I]=YMIN;
      }
    }


void main()  {

    printf("\n DECONVOLUTION OF A SIGNAL F(T)\n\n");
    printf(" Input data file name (without .dat): "); scanf("%s",nom);
    printf("\n");

    strcpy(output,"v");
    strcat(output,nom); strcat(output,".lst");
    strcpy(input,nom); strcat(input,".dat");

    fp_out=fopen(output,"w");
    fp_in=fopen(input,"r");

// read title (1 line maxi 80 CharaCters)
    fscanf(fp_in,"%s",Title);
    fprintf(fp_out," DECONVOLUTION OF SIGNAL: \n");
    fprintf(fp_out,"%s\n\n",Title);

// print resume of program's functions

    fprintf(fp_out," THIS PROGRAM ALLOWS TO:\n\n");
    fprintf(fp_out,"  - READ MEASUREMENTS IN A DATA FILE,\n");
    fprintf(fp_out,"  - FILTER THESE MEASUREMENTS (OPTIONAL),\n");
    fprintf(fp_out,"  - RECONSTRUCT THE SPEED AT BASIS BY DECONVOLUTION,\n");
    fprintf(fp_out,"  - CALCULATE THE ACCELERATION SHOCK SPECTRUM.\n\n");
                                                                                
// Frequency of captor
    fscanf(fp_in,"%lf",&f0);
    fprintf(fp_out," FREQUENCY OF CAPTOR: %6.1f HERTZ\n",f0);
                                                                                
// rod damping
    fscanf(fp_in,"%lf",&dzeta);
    fprintf(fp_out," DAMPING OF CAPTOR: %5.2f\n",dzeta);
                                                                                
// Value of Q for the shock spectrum
    fscanf(fp_in,"%lf",&q);                                                                    
    fprintf(fp_out," Q VALUE FOR THE SHOCK SPECTRUM: %4.0f\n\n",q);
                                                                                
// Number of frequencies for the shock spectrum
    fscanf(fp_in,"%d",&nspec);
    fprintf(fp_out," NUMBER OF FREQUENCIES FOR SPECTRUM: %d\n",nspec);
                                                                                
// FREQUENCIES MINI & MAXI OF SPECTRUM
    fscanf(fp_in,"%lf",&finf);                                                
    fscanf(fp_in,"%lf",&fsup);
    fprintf(fp_out," BEGIN FREQUENCY OF SPECTRUM: %6.1f Hz\n",finf);
    fprintf(fp_out," END FREQUENCY OF SPECTRUM  : %6.1f Hz\n\n",fsup);

// Read measurements
    fscanf(fp_in,"%d",&ndata);
    
    // dynamic allocations of vectors A,VIT,S
    vmblock = vminit();
    ACC  = (REAL *)vmalloc(vmblock, VEKTOR,  ndata+1, 0);
    VIT  = (REAL *)vmalloc(vmblock, VEKTOR,  ndata+1, 0);
    S    = (REAL *)vmalloc(vmblock, VEKTOR,  ndata+1, 0);
    if (! vmcomplete(vmblock)) {
      LogError ("Memory full !", 0, __FILE__, __LINE__);
      return;
    }	  

    //READ DATA;
    fprintf(fp_out,"  %d POINTS READ.\n",ndata);
    fscanf(fp_in,"%lf",&ts);                                                              
    fprintf(fp_out," SAMPLING DURATION: %f S.\n",ts);
    for (i=1; i<ndata+1; i++) {
      fscanf(fp_in," %lf %lf",&temp1,&temp);
      ACC[i]=temp;
    }
    fclose(fp_in);

    fmax= (0.5/ts);
    fprintf(fp_out," NYQUIST FREQUENCY: %6.1f\n",fmax);
                                                                               
// CALL DECONVOLUTION
    DECON(f0,ACC,VIT,ts,dzeta,ndata);
    if (fsup > fmax)  fsup=fmax;

//print speed results to output file
    fprintf(fp_out,"\n\n   3RD COLUMN: SPEED OBTAINED BY DECONVOLUTION FROM\n");
    fprintf(fp_out,"   UNFILTERED MASS ACCELERATION.\n");
    fprintf(fp_out,"   4TH COLUMN: SPEED OBTAINED BY DECONVOLUTION FROM\n");
    fprintf(fp_out,"   FILTERED MASS ACCELERATION IF IT EXISTS.\n\n");

    fprintf(fp_out,"    TIME (S)      ACC M/S2       BASIS SPEED M/S \n");
    fprintf(fp_out,"    ---------   ------------     --------------- \n");
    for (i=1; i<ndata+1; i++) 
    fprintf(fp_out, "    %8.6f    %12.6f    %12.6f\n",(i-1)*ts,ACC[i],VIT[i]);

// Compute shock spectrum
    SPEC(VIT,dzeta,finf,fsup,ndata,ts,nspec,S);

// print spectrum results to output file
    df=(fsup-finf)/(nspec-1);
    fprintf(fp_out,"\n\n\n THE ACCELERATION SHOCK SPECTRUM  IS COMPUTED FROM\n");
    fprintf(fp_out," THE FILTERED ACCELERATION IF IT EXISTS, ELSE FROM\n");
    fprintf(fp_out," THE UNFILTERED ACCELERATION.\n\n");
    
    fprintf(fp_out," FREQUENCY STEP OF SHOCK SPECTRUM:\n");
    fprintf(fp_out,"   DF = %6.2f\n\n",df);

    fprintf(fp_out,"   FREQUENCY HZ  SPECTRUM +     SPECTRUM -  IN M/S2\n");
    fprintf(fp_out,"   ------------  -----------    -----------\n");

    for (i=1; i<nspec+1; i++) {
      fr=finf+(i-1)*df;                                                          
      fprintf(fp_out,"   %8.2f    %12.6f    %12.6f\n", fr,S[2*i-1],S[2*i]);
    }
    fprintf(fp_out,"\n End of file %s\n",output);
    fclose(fp_out);
    printf(" Results in file %s\n\n",output);
    free(vmblock);
}
                                                                                      
// End of file deconv.cpp

