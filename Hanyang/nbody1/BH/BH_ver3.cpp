#include <iostream>
#include <cstdio>
#include <ctime>
#include <cmath>
#include "utils.h"
#include "mt19937ar.h"
using namespace std;

#define FLIP(a)		(((a)==(0))?(1):(0))
#define SPIN(a)		(((a)==(0))?(1):(-1))
#define DIV(a,b)	((ABS(a)<=1.e-16 || ABS(b)<=1.e-16) ? (0.):(a/b))
#define TNS_ENERGY(lam_0,ep) ((lam_0 > 1.e-10) ? (-log(lam_0)/ep) : 1.e2)
#define ERROR(a,b)	((ABS(b-a)<1.e-13) ? (0.):(ABS((b-a)/MIN(ABS(a),ABS(b)))))
#define Route 100	//length of a name of output position
#define def_l 2		//a size of an unit cell of the lattice
#define n_max 2		//maximum number of particles in a site
#define def_n (1+n_max)	//physical dimension(occupancy on a site)
#define def_chi 18	//bond dimension chi

char File_Gamma[Route], File_lambda[Route], File_expectation[Route], File_correlator[Route], File_entangle[Route], File_spectrum[Route], File_GR[Route], File_8[Route];
//folder's name according to an initial condition//
//1) random, 2) load, 3) ini_1, 4) ini_2, ...
char *ini_folder="load\0";
bool already_exist_current_Tensors;
///////////////////////////////////////////////////
int L_initial=80, L_final=160, L_gap=20;
int def_L=L_initial;	//a size of the lattice(must be even #)
double t_initial=0.2, t_final=0.201, t_gap=0.1;
double mu_initial=0.2360, mu_final=0.2381, mu_gap=0.001;
double U=1., t=t_initial, mu=mu_initial, epsilon=0.02;
double NORM, SUM_norm, lamRSS;
double ****Gamma, **lambda;
double ****Table;
double *TEBD_a, *TEBD_u, *TEBD_vt;
double *TEBD_s, **backup_s;
////////Operator matrices////////
double **Op_identity, **Op_occupation, **Op_annihilation;
/////////////////////////////////
int count_converge, count_retry;
int thetadim=def_n*def_chi;
time_t time_start, time_end;
/////////////////// prototypes ///////////////////
void set_tensors();
void unset_tensors();
void initializing_tensors();	//initializing tensor Gamma & lambda
void set_operators();
void unset_operators();
void set_table();	//<m1 m2|exp{-tau*H}|n1 n2>
void unset_table();
void TEBD(double ****Gam, double **lam);
double normalizing_lambda(double div, double *w, int i);
bool end_TEBD();
double expectation_value(double **Operator, double ****Gam, double **lam, int rank, int r);
void printf_Tensors();
void printf_columnhead();
void printf_exit();
void printf_expectation();
void printf_correlator(double **oper);
void printf_entanglement();
void printf_spectrum();
void main_backup();
void main_spectrum();	//set the ini_folder to "random"
double entanglement_8ight(int n1, int n2, int n3, int n4, int n5, int n6, int n7, int n8);
void main_8ight();	//set the ini_folder to "load"
//////////////////////////////////////////////////
////////////////////// main //////////////////////
int main()
{
	main_8ight();
	return 0;
}
//////////////////////////////////////////////////
void set_tensors()
{
	Gamma=new double ***[def_l];
	lambda=new double *[def_l];
	backup_s=new double *[def_l];
	TEBD_a=new double [thetadim*thetadim];
	TEBD_u=new double [thetadim*thetadim];
	TEBD_s=new double [thetadim*thetadim];
	TEBD_vt=new double [thetadim*thetadim];
	for(int site=0;site<def_l;site++)
	{
		Gamma[site]=new double **[def_n];
		lambda[site]=new double [def_chi];
		backup_s[site]=new double [def_chi];
		for(int n=0;n<def_n;n++)
		{
			Gamma[site][n]=new double *[def_chi];
			for(int a=0;a<def_chi;a++)
				Gamma[site][n][a]=new double [def_chi];
		}
	}
}
void unset_tensors()
{
	for(int site=0;site<def_l;site++)
	{
		for(int n=0;n<def_n;n++)
		{
			for(int a=0;a<def_chi;a++)
				delete [] Gamma[site][n][a];
			delete [] Gamma[site][n];
		}
		delete [] Gamma[site];
		delete [] lambda[site];
		delete [] backup_s[site];
	}
	delete [] TEBD_a;
	delete [] TEBD_u;
	delete [] TEBD_s;
	delete [] TEBD_vt;
	delete [] Gamma;
	delete [] lambda;
	delete [] backup_s;
}
void initializing_tensors()
{
	double amp=5.;		//The amplitude of the initial condition
	double mean=0.;		//The mean value of the initial confition
	already_exist_current_Tensors=false;
	if(ini_folder=="random\0")
	{
		init_genrand((unsigned)time(NULL));
		for(int site=0;site<def_l;site++)
		for(int a2=0;a2<def_chi;a2++)
		{
			for(int n=0;n<def_n;n++)
			for(int a1=0;a1<def_chi;a1++)
				Gamma[site][n][a1][a2]=amp*2.*(genrand_real3()-0.5)+mean;
			lambda[site][a2]=genrand_real3()-0.5;
		}
	}
	else if(ini_folder=="load\0")
	{
		FILE *fp_Gam, *fp_lam;
		//load Tensors from the random folder//
		sprintf(File_lambda,"./Tensors/n_max=%d/random/t=%.5f,mu=%.5f,chi=%d_lambda.dat\0",n_max,t,mu,def_chi);
		sprintf(File_Gamma,"./Tensors/n_max=%d/random/t=%.5f,mu=%.5f,chi=%d_Gamma.dat\0",n_max,t,mu,def_chi);
		fp_lam=fopen(File_lambda,"r");
		fp_Gam=fopen(File_Gamma,"r");
		//if there is no current tensor file, let the initial condition set randomly//
		if(fp_Gam==NULL || fp_lam==NULL)
		{
			init_genrand((unsigned)time(NULL));
			for(int site=0;site<def_l;site++)
			for(int a2=0;a2<def_chi;a2++)
			{
				for(int n=0;n<def_n;n++)
				for(int a1=0;a1<def_chi;a1++)
					Gamma[site][n][a1][a2]=amp*2.*(genrand_real3()-0.5)+mean;
				lambda[site][a2]=genrand_real3()-0.5;
			}
		}
		else
		{
			already_exist_current_Tensors=true;
			//load old data to lambda//
			for(int a=0;a<def_chi;a++)
				fscanf(fp_lam," %lf\t%lf\n",&lambda[0][a],&lambda[1][a]);
			//load old data to Gamma//
			for(int n=0;n<def_n;n++)
			for(int a1=0;a1<def_chi;a1++)
			for(int a2=0;a2<def_chi;a2++)
				fscanf(fp_Gam," %lf\t%lf\n",&Gamma[0][n][a1][a2],&Gamma[1][n][a1][a2]);
		}
		fclose(fp_lam);
		fclose(fp_Gam);
		for(int i=0;i<10;i++)
			TEBD(Gamma,lambda);
	}
	else
	{
		FILE *fp_Gam, *fp_lam;
		//let FILE pointers direct each previous file//
		sprintf(File_lambda,"./Tensors/n_max=%d/%s/t=%.5f,mu=%.5f,chi=%d_lambda.dat\0",n_max,ini_folder,t,mu-mu_gap,def_chi);
		sprintf(File_Gamma,"./Tensors/n_max=%d/%s/t=%.5f,mu=%.5f,chi=%d_Gamma.dat\0",n_max,ini_folder,t,mu-mu_gap,def_chi);
		fp_lam=fopen(File_lambda,"r");
		fp_Gam=fopen(File_Gamma,"r");
		//check existence of a previous file//
		if(fp_Gam==NULL || fp_lam==NULL)
		{
			fclose(fp_lam);
			fclose(fp_Gam);
			//let FILE pointers direct each current file.//
			sprintf(File_lambda,"./Tensors/n_max=%d/%s/t=%.5f,mu=%.5f,chi=%d_lambda.dat\0",n_max,ini_folder,t,mu,def_chi);
			sprintf(File_Gamma,"./Tensors/n_max=%d/%s/t=%.5f,mu=%.5f,chi=%d_Gamma.dat\0",n_max,ini_folder,t,mu,def_chi);
			fp_lam=fopen(File_lambda,"r");
			fp_Gam=fopen(File_Gamma,"r");
			//if there are no current files neither, exit the program.//
			if(fp_Gam==NULL || fp_lam==NULL)
			{
				printf(" There are no old data. chi=%d, %s\n",def_chi,ini_folder);
				exit(1);
			}
			already_exist_current_Tensors=true;
		}
		//load old data to lambda//
		for(int a=0;a<def_chi;a++)
			fscanf(fp_lam," %lf\t%lf\n",&lambda[0][a],&lambda[1][a]);
		//load old data to Gamma//
		for(int n=0;n<def_n;n++)
		for(int a1=0;a1<def_chi;a1++)
		for(int a2=0;a2<def_chi;a2++)
			fscanf(fp_Gam," %lf\t%lf\n",&Gamma[0][n][a1][a2],&Gamma[1][n][a1][a2]);
		fclose(fp_lam);
		fclose(fp_Gam);
	}
}
void set_operators()
{
	Op_identity=new double *[def_n];
	Op_occupation=new double *[def_n];
	Op_annihilation=new double *[def_n];
	for(int i=0;i<def_n;i++)
	{
		Op_identity[i]=new double [def_n];
		Op_occupation[i]=new double [def_n];
		Op_annihilation[i]=new double [def_n];
	}
	for(int n=0;n<def_n;n++)
	for(int m=0;m<def_n;m++)
	{
		Op_identity[n][m]=(n==m)?(1.):(0.);
		Op_occupation[n][m]=(n==m)?((double)n):(0.);
		Op_annihilation[n][m]=(n==m-1)?(pow((double)m,0.5)):(0.);
	}
}
void unset_operators()
{
	for(int i=0;i<def_n;i++)
	{
		delete [] Op_identity[i];
		delete [] Op_occupation[i];
		delete [] Op_annihilation[i];
	}
	delete [] Op_identity;
	delete [] Op_occupation;
	delete [] Op_annihilation;
}
void set_table()
{
	double ****h_ij, ****h_prime, ****dummy;

	Table=new double ***[def_n];
	h_ij=new double ***[def_n];
	h_prime=new double ***[def_n];
	dummy=new double ***[def_n];
	for(int i=0;i<def_n;i++)
	{
		Table[i]=new double **[def_n];
		h_ij[i]=new double **[def_n];
		h_prime[i]=new double **[def_n];
		dummy[i]=new double **[def_n];
		for(int j=0;j<def_n;j++)
		{
			Table[i][j]=new double *[def_n];
			h_ij[i][j]=new double *[def_n];
			h_prime[i][j]=new double *[def_n];
			dummy[i][j]=new double *[def_n];
			for(int k=0;k<def_n;k++)
			{
				Table[i][j][k]=new double [def_n];
				h_ij[i][j][k]=new double [def_n];
				h_prime[i][j][k]=new double [def_n];
				dummy[i][j][k]=new double [def_n];
			}
		}
	}
	for(int m1=0;m1<def_n;m1++)
	for(int m2=0;m2<def_n;m2++)
	for(int n1=0;n1<def_n;n1++)
	for(int n2=0;n2<def_n;n2++)
	{
		double a=0., b=0., c=0.;
		if(m1==n1 && m2==n2)
			a=U*0.25*(n1*n1+n2*n2)-0.5*(U*0.5+mu)*(n1+n2);
		if(m1==n1+1 && m2==n2-1)
			b=-t*pow((double)((n1+1)*n2),0.5);
		if(m1==n1-1 && m2==n2+1)
			c=-t*pow((double)(n1*(n2+1)),0.5);
		h_ij[m1][m2][n1][n2]=a+b+c;
		h_prime[m1][m2][n1][n2]=0.;
		Table[m1][m2][n1][n2]=0.;
		if(m1==n1 && m2==n2)
		{
			h_prime[m1][m2][n1][n2]=1.;
			Table[m1][m2][n1][n2]=1.;
		}
	}
	for(int count=1;count<20;count++)
	{
		for(int m1=0;m1<def_n;m1++)
		for(int m2=0;m2<def_n;m2++)
		for(int n1=0;n1<def_n;n1++)
		for(int n2=0;n2<def_n;n2++)
		{
			dummy[m1][m2][n1][n2]=0.;
			for(int k1=0;k1<def_n;k1++)
			for(int k2=0;k2<def_n;k2++)
				dummy[m1][m2][n1][n2]+=h_prime[m1][m2][k1][k2]*h_ij[k1][k2][n1][n2];
		}
		for(int m1=0;m1<def_n;m1++)
		for(int m2=0;m2<def_n;m2++)
		for(int n1=0;n1<def_n;n1++)
		for(int n2=0;n2<def_n;n2++)
			Table[m1][m2][n1][n2]+=h_prime[m1][m2][n1][n2]=dummy[m1][m2][n1][n2]*DIV(-epsilon,(double)count);
	}
	for(int i=0;i<def_n;i++)
	{
		for(int j=0;j<def_n;j++)
		{
			for(int k=0;k<def_n;k++)
			{
				delete [] h_ij[i][j][k];
				delete [] h_prime[i][j][k];
				delete [] dummy[i][j][k];
			}
			delete [] h_ij[i][j];
			delete [] h_prime[i][j];
			delete [] dummy[i][j];
		}
		delete [] h_ij[i];
		delete [] h_prime[i];
		delete [] dummy[i];
	}
	delete [] h_ij;
	delete [] h_prime;
	delete [] dummy;
}
void unset_table()
{
	for(int i=0;i<def_n;i++)
	{
		for(int j=0;j<def_n;j++)
		{
			for(int k=0;k<def_n;k++)
				delete [] Table[i][j][k];
			delete [] Table[i][j];
		}
		delete [] Table[i];
	}
	delete [] Table;
}
void TEBD(double ****Gam, double **lam)
{
	int lda=thetadim, ldu=thetadim, ldvt=thetadim;

	for(int site=0;site<def_l;site+=2)
	{
		int lsite=(site-1+def_l)%def_l, rsite=(site+1)%def_l;
		for(int m1=0;m1<def_n;m1++)
		for(int m2=0;m2<def_n;m2++)
		for(int aL=0;aL<def_chi;aL++)
		for(int a2=0;a2<def_chi;a2++)
		{
			TEBD_a[(m1+aL*def_n)+lda*(m2+a2*def_n)]=0.;
			for(int n1=0;n1<def_n;n1++)
			for(int n2=0;n2<def_n;n2++)
			for(int a1=0;a1<def_chi;a1++)
				TEBD_a[(m1+aL*def_n)+lda*(m2+a2*def_n)] += Table[m1][m2][n1][n2]*lam[lsite][aL]*lam[site][a1]*lam[rsite][a2]*Gam[site][n1][aL][a1]*Gam[rsite][n2][a1][a2];
		}
		SUBLOUTINE_DGESVD(TEBD_a,TEBD_u,TEBD_s,TEBD_vt,thetadim,thetadim);
		for(int p=0;p<def_n;p++)
		for(int b0=0;b0<def_chi;b0++)
		for(int b1=0;b1<def_chi;b1++)
		{
			Gam[site][p][b0][b1]=DIV(TEBD_u[(p+b0*def_n)+ldu*(b1)],lam[lsite][b0]);
			Gam[rsite][p][b0][b1]=DIV(TEBD_vt[(b0)+ldvt*(p+b1*def_n)],lam[rsite][b1]);
		}
		for(int i=0;i<def_chi;i++)
		{
			backup_s[site][i]=TEBD_s[i];
			lam[site][i]=normalizing_lambda(TEBD_s[0],TEBD_s,i);
		}
		if(site+2==def_l)
			site=-1;
	}
}
double normalizing_lambda(double div, double *w, int i)
{
	return (div <= 0. || w[i] <= 0.) ? (0.) : (w[i]/div);
}
bool end_TEBD()
{
	for(int i=0;i<def_chi;i++)
		if(backup_s[0][i]>1.e-3 && ERROR(backup_s[0][i],backup_s[1][i])>1.e-9)
			return false;
	return true;
}
double expectation_value(double **Operator, double ****Gam, double **lam, int rank, int r)
{
	double **Bp1, **Bp2, **B1, **B2, **B12, **B21, **Bp12;
	double *svd_A, *svd_U, *svd_Vt, *S_B1, *S_B2, *S_Bp1, *S_Bp2, *S_B12, *S_B21, *S_Bp12;
	double **B_tot, **dummy;
	int chiSQ=def_chi*def_chi;
	
	Bp1=new double *[chiSQ];
	Bp2=new double *[chiSQ];
	B1=new double *[chiSQ];
	B2=new double *[chiSQ];
	B12=new double *[chiSQ];
	B21=new double *[chiSQ];
	Bp12=new double *[chiSQ];
	svd_A=new double [chiSQ*chiSQ]; svd_U=new double [chiSQ*chiSQ]; svd_Vt=new double [chiSQ*chiSQ];
	B_tot=new double *[chiSQ]; dummy=new double *[chiSQ];
	S_B1=new double [chiSQ]; S_B2=new double [chiSQ];
	S_Bp1=new double [chiSQ]; S_Bp2=new double [chiSQ];
	S_B12=new double [chiSQ]; S_B21=new double [chiSQ]; S_Bp12=new double [chiSQ];

	for(int k=0;k<chiSQ;k++)
	{
		Bp1[k]=new double [chiSQ];
		Bp2[k]=new double [chiSQ];
		B1[k]=new double [chiSQ];
		B2[k]=new double [chiSQ];
		B12[k]=new double [chiSQ];
		B21[k]=new double [chiSQ];
		Bp12[k]=new double [chiSQ];
		B_tot[k]=new double [chiSQ];
		dummy[k]=new double [chiSQ];
	}
	for(int i=0;i<chiSQ;i++)
	for(int j=0;j<chiSQ;j++)
	{
		Bp1[i][j]=0.;
		Bp2[i][j]=0.;
		B1[i][j]=0.;
		B2[i][j]=0.;
		B12[i][j]=0.;
		B21[i][j]=0.;
		Bp12[i][j]=0.;
		B_tot[i][j]=((i==j) ? (1.):(0.));
		dummy[i][j]=0.;
	}
	for(int ap1=0;ap1<def_chi;ap1++)
	for(int a1=0;a1<def_chi;a1++)
	for(int ap2=0;ap2<def_chi;ap2++)
	for(int a2=0;a2<def_chi;a2++)
	{
		for(int n=0;n<def_n;n++)
		for(int m=0;m<def_n;m++)
		{
			B1[ap1+def_chi*a1][ap2+def_chi*a2]+=Op_identity[n][m]*pow(lam[1][ap1]*lam[0][ap2]*lam[1][a1]*lam[0][a2],0.5)*Gam[0][n][ap1][ap2]*Gam[0][m][a1][a2];
			B2[ap1+def_chi*a1][ap2+def_chi*a2]+=Op_identity[n][m]*pow(lam[0][ap1]*lam[1][ap2]*lam[0][a1]*lam[1][a2],0.5)*Gam[1][n][ap1][ap2]*Gam[1][m][a1][a2];
			Bp1[ap1+def_chi*a1][ap2+def_chi*a2]+=Operator[n][m]*pow(lam[1][ap1]*lam[0][ap2]*lam[1][a1]*lam[0][a2],0.5)*Gam[0][n][ap1][ap2]*Gam[0][m][a1][a2];
			Bp2[ap1+def_chi*a1][ap2+def_chi*a2]+=Operator[n][m]*pow(lam[0][ap1]*lam[1][ap2]*lam[0][a1]*lam[1][a2],0.5)*Gam[1][n][ap1][ap2]*Gam[1][m][a1][a2];
			//B1[ap1+def_chi*a1][ap2+def_chi*a2]+=Op_identity[n][m]*pow(lam[1][ap1]*lam[0][ap2]*lam[1][a1]*lam[0][a2],0.5)*Gam[0][n][ap2][ap1]*Gam[0][m][a1][a2];
			//B2[ap1+def_chi*a1][ap2+def_chi*a2]+=Op_identity[n][m]*pow(lam[0][ap1]*lam[1][ap2]*lam[0][a1]*lam[1][a2],0.5)*Gam[1][n][ap2][ap1]*Gam[1][m][a1][a2];
			//Bp1[ap1+def_chi*a1][ap2+def_chi*a2]+=Operator[n][m]*pow(lam[1][ap1]*lam[0][ap2]*lam[1][a1]*lam[0][a2],0.5)*Gam[0][n][ap2][ap1]*Gam[0][m][a1][a2];
			//Bp2[ap1+def_chi*a1][ap2+def_chi*a2]+=Operator[n][m]*pow(lam[0][ap1]*lam[1][ap2]*lam[0][a1]*lam[1][a2],0.5)*Gam[1][n][ap2][ap1]*Gam[1][m][a1][a2];
		}
	}
	for(int i=0;i<chiSQ;i++)
	for(int j=0;j<chiSQ;j++)
	for(int k=0;k<chiSQ;k++)
	{
		B12[i][j]+=B1[i][k]*B2[k][j];
		B21[i][j]+=B2[i][k]*B1[k][j];
		Bp12[i][j]+=Bp1[i][k]*Bp2[k][j];
	}
	//// get a singular value(=eigenvalue) of each B matrix ////
	for(int i=0;i<chiSQ;i++)
	for(int j=0;j<chiSQ;j++)
		svd_A[i+j*chiSQ]=B1[i][j];
	SUBLOUTINE_DGESVD(svd_A,svd_U,S_B1,svd_Vt,chiSQ,chiSQ);
	for(int i=0;i<chiSQ;i++)
	for(int j=0;j<chiSQ;j++)
		svd_A[i+j*chiSQ]=B2[i][j];
	SUBLOUTINE_DGESVD(svd_A,svd_U,S_B2,svd_Vt,chiSQ,chiSQ);
	for(int i=0;i<chiSQ;i++)
	for(int j=0;j<chiSQ;j++)
		svd_A[i+j*chiSQ]=Bp1[i][j];
	SUBLOUTINE_DGESVD(svd_A,svd_U,S_Bp1,svd_Vt,chiSQ,chiSQ);
	for(int i=0;i<chiSQ;i++)
	for(int j=0;j<chiSQ;j++)
		svd_A[i+j*chiSQ]=Bp2[i][j];
	SUBLOUTINE_DGESVD(svd_A,svd_U,S_Bp2,svd_Vt,chiSQ,chiSQ);
	for(int i=0;i<chiSQ;i++)
	for(int j=0;j<chiSQ;j++)
		svd_A[i+j*chiSQ]=B12[i][j];
	SUBLOUTINE_DGESVD(svd_A,svd_U,S_B12,svd_Vt,chiSQ,chiSQ);
	for(int i=0;i<chiSQ;i++)
	for(int j=0;j<chiSQ;j++)
		svd_A[i+j*chiSQ]=B21[i][j];
	SUBLOUTINE_DGESVD(svd_A,svd_U,S_B21,svd_Vt,chiSQ,chiSQ);
	for(int i=0;i<chiSQ;i++)
	for(int j=0;j<chiSQ;j++)
		svd_A[i+j*chiSQ]=Bp12[i][j];
	SUBLOUTINE_DGESVD(svd_A,svd_U,S_Bp12,svd_Vt,chiSQ,chiSQ);
	for(int i=0;i<chiSQ;i++)
	for(int j=0;j<chiSQ;j++)
		dummy[i][j]=B1[i][j]-B2[i][j];
	////////////////////////////////////////////////////////////
	//check_matrix_symmetry("B1",B1,chiSQ);
	//check_matrix_symmetry("B2",B2,chiSQ);
	//check_matrix_symmetry("B1-B2",dummy,chiSQ);
	double norm=0.;
	////////// <psi|O(r)|psi> //////////
	if(rank==1 && r<def_L)
	{
		for(int site=0;site<def_L;site+=2)
		{
			for(int i=0;i<chiSQ;i++)
			for(int j=0;j<chiSQ;j++)
			{
				dummy[i][j]=0.;
				for(int k=0;k<chiSQ;k++)
				{
					if(site!=r && site!=r+1)
						dummy[i][j]+=B_tot[i][k]*B12[k][j];
					else if(site==r)
					for(int l=0;l<chiSQ;l++)
						dummy[i][j]+=B_tot[i][k]*Bp1[k][l]*B2[l][j];
					else if(site==r+1)
					for(int l=0;l<chiSQ;l++)
						dummy[i][j]+=B_tot[i][k]*B1[k][l]*Bp2[l][j];
				}
			}
			for(int i=0;i<chiSQ;i++)
			for(int j=0;j<chiSQ;j++)
				B_tot[i][j]=dummy[i][j];
		}
		for(int i=0;i<chiSQ;i++)
			norm+=B_tot[i][i];
	}
	/////////// <psi|O|psi> ///////////
	else if(rank==1)
	{
		//check_matrix_symmetry("Bp12",Bp12,chiSQ);
		//printf("S_Bp12[0]-1.=%.8e\n",S_Bp12[0]-1.);
		for(int site=0;site<def_L;site+=2)
		{
			for(int i=0;i<chiSQ;i++)
			for(int j=0;j<chiSQ;j++)
			{
				dummy[i][j]=0.;
				for(int k=0;k<chiSQ;k++)
					dummy[i][j]+=B_tot[i][k]*Bp12[k][j];
			}
			for(int i=0;i<chiSQ;i++)
			for(int j=0;j<chiSQ;j++)
				B_tot[i][j]=dummy[i][j];
		}
		for(int i=0;i<chiSQ;i++)
			norm+=B_tot[i][i];
		//for(int i=0;i<chiSQ;i++)
		//	norm-=pow(S_Bp12[i],def_L*0.5);
	}
	/////// <psi|O(0)O(r)|psi> ///////
	else if(rank==2)
	{
		if(r%2==1)
		{
			for(int i=0;i<chiSQ;i++)
			for(int j=0;j<chiSQ;j++)
				B_tot[i][j]=Bp1[i][j];
			for(int z=0;z<(int)((r-1)*0.5);z++)
			{
				for(int i=0;i<chiSQ;i++)
				for(int j=0;j<chiSQ;j++)
				{
					dummy[i][j]=0.;
					for(int k=0;k<chiSQ;k++)
						dummy[i][j]+=B_tot[i][k]*B21[k][j];
				}
				for(int i=0;i<chiSQ;i++)
				for(int j=0;j<chiSQ;j++)
					B_tot[i][j]=dummy[i][j];
			}
			for(int i=0;i<chiSQ;i++)
			for(int j=0;j<chiSQ;j++)
			{
				dummy[i][j]=0.;
				for(int k=0;k<chiSQ;k++)
					dummy[i][j]+=B_tot[i][k]*Bp2[k][j];
			}
			for(int i=0;i<chiSQ;i++)
			for(int j=0;j<chiSQ;j++)
				B_tot[i][j]=dummy[i][j];
			for(int z=0;z<(int)((def_L-r-1)*0.5);z++)
			{
				for(int i=0;i<chiSQ;i++)
				for(int j=0;j<chiSQ;j++)
				{
					dummy[i][j]=0.;
					for(int k=0;k<chiSQ;k++)
						dummy[i][j]+=B_tot[i][k]*B12[k][j];
				}
				for(int i=0;i<chiSQ;i++)
				for(int j=0;j<chiSQ;j++)
					B_tot[i][j]=dummy[i][j];
			}
		}
		else
		{
			for(int i=0;i<chiSQ;i++)
			for(int j=0;j<chiSQ;j++)
			{
				dummy[i][j]=0.;
				for(int k=0;k<chiSQ;k++)
					dummy[i][j]+=B2[i][k]*Bp1[k][j];
			}
			for(int i=0;i<chiSQ;i++)
			for(int j=0;j<chiSQ;j++)
				B_tot[i][j]=dummy[i][j];
			for(int z=0;z<(int)((r-2)*0.5);z++)
			{
				for(int i=0;i<chiSQ;i++)
				for(int j=0;j<chiSQ;j++)
				{
					dummy[i][j]=0.;
					for(int k=0;k<chiSQ;k++)
						dummy[i][j]+=B_tot[i][k]*B21[k][j];
				}
				for(int i=0;i<chiSQ;i++)
				for(int j=0;j<chiSQ;j++)
					B_tot[i][j]=dummy[i][j];
			}
			for(int i=0;i<chiSQ;i++)
			for(int j=0;j<chiSQ;j++)
			{
				dummy[i][j]=0.;
				for(int k=0;k<chiSQ;k++)
					dummy[i][j]+=B_tot[i][k]*B2[k][j];
			}
			for(int i=0;i<chiSQ;i++)
			for(int j=0;j<chiSQ;j++)
				B_tot[i][j]=dummy[i][j];
			for(int i=0;i<chiSQ;i++)
			for(int j=0;j<chiSQ;j++)
			{
				dummy[i][j]=0.;
				for(int k=0;k<chiSQ;k++)
					dummy[i][j]+=B_tot[i][k]*Bp1[k][j];
			}
			for(int i=0;i<chiSQ;i++)
			for(int j=0;j<chiSQ;j++)
				B_tot[i][j]=dummy[i][j];
			for(int z=0;z<(int)((def_L-r-2)*0.5);z++)
			{
				for(int i=0;i<chiSQ;i++)
				for(int j=0;j<chiSQ;j++)
				{
					dummy[i][j]=0.;
					for(int k=0;k<chiSQ;k++)
						dummy[i][j]+=B_tot[i][k]*B21[k][j];
				}
				for(int i=0;i<chiSQ;i++)
				for(int j=0;j<chiSQ;j++)
					B_tot[i][j]=dummy[i][j];
			}
		}
		for(int i=0;i<chiSQ;i++)
			norm+=B_tot[i][i];
	}
	///////////////////////////////////
	for(int k=0;k<chiSQ;k++)
	{
		delete [] Bp1[k];
		delete [] Bp2[k];
		delete [] B1[k];
		delete [] B2[k];
		delete [] B12[k];
		delete [] B21[k];
		delete [] Bp12[k];
		delete [] B_tot[k];
		delete [] dummy[k];
	}
	delete [] Bp1;
	delete [] Bp2;
	delete [] B1;
	delete [] B2;
	delete [] B12;
	delete [] B21;
	delete [] Bp12;
	delete [] svd_A; delete [] svd_U; delete [] svd_Vt;
	delete [] B_tot; delete [] dummy;
	delete [] S_B1; delete [] S_B2;
	delete [] S_Bp1; delete [] S_Bp2;
	delete [] S_B12; delete [] S_B21; delete [] S_Bp12;
	return norm;
}
void printf_Tensors()
{
	FILE *matrix, *lam;
		//File_Gamma//
	sprintf(File_Gamma,"./Tensors/n_max=%d/%s/t=%.5f,mu=%.5f,chi=%d_Gamma.dat\0",n_max,ini_folder,t,mu,def_chi);
	matrix=fopen(File_Gamma,"w");	fclose(matrix);
	matrix=fopen(File_Gamma,"a");
	//check the existence of FILE*//
	if(matrix==NULL)
	{
		printf(" The route of File_Gamma is wrong!\n");
		exit(1);
	}
	//////////////////////////////////
	for(int n=0;n<def_n;n++)
	for(int a1=0;a1<def_chi;a1++)
	for(int a2=0;a2<def_chi;a2++)
		fprintf(matrix,"%.14e\t%.14e\n",Gamma[0][n][a1][a2],Gamma[1][n][a1][a2]);
	fclose(matrix);
		//File_lambda//
	sprintf(File_lambda,"./Tensors/n_max=%d/%s/t=%.5f,mu=%.5f,chi=%d_lambda.dat\0",n_max,ini_folder,t,mu,def_chi);
	lam=fopen(File_lambda,"w");	fclose(lam);
	lam=fopen(File_lambda,"a");
	//check the existence of FILE*//
	if(lam==NULL)
	{
		printf(" The route of File_lambda is wrong!\n");
		exit(1);
	}
	//////////////////////////////////
	for(int a=0;a<def_chi;a++)
		fprintf(lam,"%.14e\t%.14e\n",lambda[0][a],lambda[1][a]);
	fclose(lam);
}
void printf_columnhead()
{
	FILE *expectation, *correlator, *entangle, *spectrum;
	time(&time_start);
		//File_expectation//
	sprintf(File_expectation,"./Data/n_max=%d/%s/t=%.5f,chi=%d_expectation.dat\0",n_max,ini_folder,t,def_chi);
	if(ini_folder=="load\0")
		sprintf(File_expectation,"./Data/n_max=%d/random/t=%.5f,chi=%d_expectation.dat\0",n_max,t,def_chi);
	expectation=fopen(File_expectation,"a");
	//check the existence of FILE*//
	if(expectation==NULL)
	{
		printf(" The route of File_expectation is wrong!\n");
		exit(1);
	}
	fprintf(expectation,"#start:\t");
	fprintf(expectation,ctime(&time_start));
	fprintf(expectation,"#L\tt\t\tmu\t\tchi\t<n>\t\t<b>\t\tnorm\t\tcount\tre\n");
	fclose(expectation);
	//////////////////////////////////
		//File_correlator//
	sprintf(File_correlator,"./Data/n_max=%d/%s/t=%.5f,chi=%d_correlator.dat\0",n_max,ini_folder,t,def_chi);
	if(ini_folder=="load\0")
		sprintf(File_correlator,"./Data/n_max=%d/random/t=%.5f,chi=%d_correlator.dat\0",n_max,t,def_chi);
	correlator=fopen(File_correlator,"a");
	//check the existence of FILE*//
	if(correlator==NULL)
	{
		printf(" The route of File_correlator is wrong!\n");
		exit(1);
	}
	fprintf(correlator,"#start:\t");
	fprintf(correlator,ctime(&time_start));
	fprintf(correlator,"#L\tchi\tt\t\tmu\t\tnorm\t\tr\t<corr>\t\t<corr>-<><>\n");
	fclose(correlator);
	//////////////////////////////////
	if(ini_folder!="load\0")
	{
			//File_entangle//
		sprintf(File_entangle,"./Data/n_max=%d/%s/t=%.5f,chi=%d_entropy.dat\0",n_max,ini_folder,t,def_chi);
		if(ini_folder=="load\0")
			sprintf(File_entangle,"./Data/n_max=%d/random/t=%.5f,chi=%d_entropy.dat\0",n_max,t,def_chi);
		entangle=fopen(File_entangle,"a");
		//check the existence of FILE*//
		if(entangle==NULL)
		{
			printf(" The route of File_entangle is wrong!\n");
			exit(1);
		}
		fprintf(entangle,"#start:\t");
		fprintf(entangle,ctime(&time_start));
		fprintf(entangle,"#t\t\tmu\t\tchi\tS_{1/2}\t\tenergy\n");
		fclose(entangle);
		//////////////////////////////////
			//File_spectrum//
		sprintf(File_spectrum,"./Data/n_max=%d/%s/t=%.5f,chi=%d_spectrum.dat\0",n_max,ini_folder,t,def_chi);
		if(ini_folder=="load\0")
			sprintf(File_spectrum,"./Data/n_max=%d/random/t=%.5f,chi=%d_spectrum.dat\0",n_max,t,def_chi);
		spectrum=fopen(File_spectrum,"a");
		//check the existence of FILE*//
		if(spectrum==NULL)
		{
			printf(" The route of File_spectrum is wrong!\n");
			exit(1);
		}
		fprintf(spectrum,"#start:\t");
		fprintf(spectrum,ctime(&time_start));
		fprintf(spectrum,"#t\t\tmu\t\ti\ts[0][i]\t\ts[1][i]\t\tsum_RS\n");
		fclose(spectrum);
		//////////////////////////////////
	}
}
void printf_exit()
{
	FILE *expectation, *correlator, *entangle, *spectrum;
	int hour, min, sec;
	time(&time_end);
	hour=(int)((time_end-time_start)/3600.);
	min=(int)(((time_end-time_start)%3600)/60.);
	sec=(int)(((time_end-time_start)%3600)%60);
		//File_expectation//
	expectation=fopen(File_expectation,"a");
	fprintf(expectation,"#end:\t");
	fprintf(expectation,ctime(&time_end));
	fprintf(expectation,"#playtime: %02d:%02d:%02d\n",hour,min,sec);
	fclose(expectation);
		//File_correlator//
	correlator=fopen(File_correlator,"a");
	fprintf(correlator,"#end:\t");
	fprintf(correlator,ctime(&time_end));
	fprintf(correlator,"#playtime: %02d:%02d:%02d\n",hour,min,sec);
	fclose(correlator);
	if(ini_folder!="load\0")
	{
		//File_entangle//
		entangle=fopen(File_entangle,"a");
		fprintf(entangle,"#end:\t");
		fprintf(entangle,ctime(&time_end));
		fprintf(entangle,"#playtime: %02d:%02d:%02d\n",hour,min,sec);
		fclose(entangle);
			//File_spectrum//
		spectrum=fopen(File_spectrum,"a");
		fprintf(spectrum,"#end:\t");
		fprintf(spectrum,ctime(&time_end));
		fprintf(spectrum,"#playtime: %02d:%02d:%02d\n",hour,min,sec);
		fclose(spectrum);
	}
}
void printf_expectation()
{
	FILE *expectation;
	expectation=fopen(File_expectation,"a");
	double norm=expectation_value(Op_identity,Gamma,lambda,1,def_L);
	double expect_n=expectation_value(Op_occupation,Gamma,lambda,1,0)/norm;
	double expect_b=expectation_value(Op_annihilation,Gamma,lambda,1,0)/norm;
	fprintf(expectation,"%d\t%.5e\t%.5e\t%d\t%.8e\t%.8e\t%.8e\t%d\t%d\n",def_L,t,mu,def_chi,expect_n,expect_b,norm,count_converge,count_retry);
	fclose(expectation);
}
void printf_correlator(double **oper)
{
	FILE *corrator;
	double norm, expect, dummy_correlation;
	norm=expectation_value(Op_identity,Gamma,lambda,1,def_L);
	expect=expectation_value(oper,Gamma,lambda,1,0)/norm;
	int r=(int)(0.5*def_L);
	//for(int r=1;r<def_L;r++)
	{
		dummy_correlation=expectation_value(oper,Gamma,lambda,2,r)/norm;
		corrator=fopen(File_correlator,"a");
		fprintf(corrator,"%d\t%d\t%.5e\t%.5e\t%.8e\t%d\t%.8e\t%.8e\n",def_L,def_chi,t,mu,norm,r,dummy_correlation,dummy_correlation-expect*expect);
		fclose(corrator);
	}
}
void printf_entanglement()
{
	FILE *entangle;
	entangle=fopen(File_entangle,"a");
	double SQsum=0., entropy=0., energy;
	for(int i=0;i<def_chi;i++)
		SQsum+=backup_s[0][i]*backup_s[0][i];
	for(int i=0;i<def_chi;i++)
		entropy+=(backup_s[0][i]*backup_s[0][i])*log(SQsum/(backup_s[0][i]*backup_s[0][i]));
	energy=TNS_ENERGY(backup_s[0][0],epsilon);
	fprintf(entangle,"%.5e\t%.5e\t%d\t%.8e\t%.8e\n",t,mu,def_chi,entropy/(SQsum*log(2.)),energy);
	fclose(entangle);
}
void printf_spectrum()
{
	FILE *spectrum;
	double RSQ=0.;
	for(int i=0;i<def_chi;i++)
		RSQ+=backup_s[0][i]*backup_s[0][i];
	RSQ=pow(RSQ,0.5);
	spectrum=fopen(File_spectrum,"a");
	for(int i=0;i<def_chi;i++)
		fprintf(spectrum,"%.5e\t%.5e\t%d\t%.8e\t%.8e\t%.8e\n",t,mu,i,backup_s[0][i]/RSQ,backup_s[1][i]/RSQ,RSQ);
	fclose(spectrum);
}
void main_backup()
{
	int Thermal=1e3;	//thermalizing
	int ULofC=2e6;		//upper limit of a count_converge
	double LLofS0=0.83;	//lower limit of a backup_s[i][0]
	double sum;
	bool good_ini, good_fin;	//good initial condition, good final condition
	//printf_columnhead();
	set_tensors();
	set_operators();
	for(t=t_initial;t<=MAX(t_initial,t_final) && t>=MIN(t_initial,t_final);t+=t_gap)
	for(mu=mu_initial;mu<=MAX(mu_initial,mu_final) && mu>=MIN(mu_initial,mu_final);mu+=mu_gap)
	{
		set_table();
		good_ini=false;
		good_fin=false;
		count_converge=0;
		count_retry=0;
		initializing_tensors();
		if(!already_exist_current_Tensors)
		{
			while(!good_fin && count_converge<ULofC)
			{
				while(!good_ini)
				{
					initializing_tensors();
					for(count_converge=0;count_converge<Thermal;count_converge++)
						TEBD(Gamma,lambda);
					sum=0.;
					for(int i=0;i<def_chi;i++)
						sum+=backup_s[0][i]*backup_s[0][i];
					if(backup_s[0][0]/pow(sum,0.5)>LLofS0)
						good_ini=true;
				}
				sum=0.;
				for(int i=0;i<def_chi;i++)
					sum+=backup_s[0][i]*backup_s[0][i];
				if(backup_s[0][0]/pow(sum,0.5)<LLofS0)
				{
					good_ini=false;
					count_retry++;
					continue;
				}
				if(end_TEBD())
				{
					good_fin=true;
					continue;
				}
				TEBD(Gamma,lambda);
				count_converge++;
			}
			TEBD(Gamma,lambda); //To give Gamma and lambda exact values when the initial condition was loaded
			printf_Tensors();
			//printf_spectrum();
			//printf_entanglement();
		}
		/*for(def_L=L_initial;def_L<=MAX(L_initial,L_final) && def_L>=MIN(L_initial,L_final);def_L+=L_gap)
		{
			printf_expectation();
			printf_correlator(Op_occupation);
		}*/
		unset_table();
	}
	//printf_exit();
	unset_operators();
	unset_tensors();
}
void main_spectrum()
{
	ini_folder="random\0";
	bool good_fin;	//good final condition
	int ULofC=2e6;	//upper limit of a count_converge
	double sum;
	FILE *result;
	set_tensors();
	set_operators();
	set_table();
	sprintf(File_GR,"./Data/n_max=%d/t=%.5f,mu=%.5f.dat\0",n_max,t,mu);	//GRound state of these conditions
	result=fopen(File_GR,"a");
	fprintf(result,"#chi\tt\t\tmu\t\ti\ts[0][i]\t\ts[1][i]\t\tsum_RS\t\tenergy\t\tcount\n");
	fclose(result);
	for(int iter=0;iter<1;iter++)
	{
		initializing_tensors();
		good_fin=false;
		count_converge=0;
		while(!good_fin && count_converge<ULofC)
		{
			TEBD(Gamma,lambda);
			count_converge++;
			if(end_TEBD())
				good_fin=true;
		}
		sum=0.;
		for(int i=0;i<def_chi;i++)
			sum+=backup_s[0][i]*backup_s[0][i];
		sum=pow(sum,0.5);
		result=fopen(File_GR,"a");
		for(int i=0;i<def_chi;i++)
			fprintf(result,"%d\t%.5e\t%.5e\t%d\t%.8e\t%.8e\t%.8e\t%.8e\t%d\n",def_chi,t,mu,i,backup_s[0][i]/sum,backup_s[1][i]/sum,sum,TNS_ENERGY(backup_s[0][0],epsilon),count_converge);
		fclose(result);
	}
	printf_Tensors();	//printf_Tensors
	unset_table();
	unset_operators();
	unset_tensors();
}
double entanglement_8ight(int n1, int n2, int n3, int n4, int n5, int n6, int n7, int n8)
{
	double result=0.;
	for(int i=0;i<def_chi;i++)
	{
		double sum=0., diag;
		for(int a=0;a<def_chi;a++)
		for(int b=0;b<def_chi;b++)
		for(int c=0;c<def_chi;c++)
		for(int d=0;d<def_chi;d++)
		for(int e=0;e<def_chi;e++)
		for(int f=0;f<def_chi;f++)
		for(int g=0;g<def_chi;g++)
		{
			diag=lambda[0][a]*lambda[1][b]*lambda[0][c]*lambda[1][d]*lambda[0][e]*lambda[1][f]*lambda[0][g]*lambda[1][i];
			sum+=diag*Gamma[0][n1][i][a]*Gamma[1][n2][a][b]*Gamma[0][n3][b][c]*Gamma[1][n4][c][d]*Gamma[0][n5][d][e]*Gamma[1][n6][e][f]*Gamma[0][n7][f][g]*Gamma[1][n8][g][i];
		}
		result+=sum;
	}
	return (result*result);
}
void main_8ight()
{
	ini_folder="load\0";
	FILE *result;
	sprintf(File_8,"./Data/n_max=%d/t=%.5f,mu=%.5f_8ight.dat\0",n_max,t,mu);	//GRound state of these conditions
	result=fopen(File_8,"a");
	fprintf(result,"#t\t\tmu\t\tenergy\t\t|state>\t\t\tcoefficient\n");
	fclose(result);
	set_tensors();
	set_operators();
	set_table();
	initializing_tensors();
	for(int i=0;i<10;i++)
		TEBD(Gamma,lambda);
	double energy=TNS_ENERGY(backup_s[0][0],epsilon);
	// |n1 n2 n3 n4 n5 n6 n7 n8> //
	for(int n1=0;n1<def_n;n1++)
	for(int n2=0;n2<def_n;n2++)
	for(int n3=0;n3<def_n;n3++)
	for(int n4=0;n4<def_n;n4++)
	for(int n5=0;n5<def_n;n5++)
	for(int n6=0;n6<def_n;n6++)
	for(int n7=0;n7<def_n;n7++)
	for(int n8=0;n8<def_n;n8++)
	{
		result=fopen(File_8,"a");
		fprintf(result,"%.5e\t%.5e\t%.8e\t|%d %d %d %d %d %d %d %d>\t%.8e\n",t,mu,energy,n1,n2,n3,n4,n5,n6,n7,n8,entanglement_8ight(n1,n2,n3,n4,n5,n6,n7,n8));
		fclose(result);
	}
	unset_table();
	unset_operators();
	unset_tensors();
}
