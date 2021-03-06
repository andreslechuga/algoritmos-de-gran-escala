#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
extern void dgemm_(char *transaA, char *transaB,int *m,int *n,int *k,double *alpha,double *A,int *lda,double *B,int *ldb,double *beta,double *C,int *ldc);

void impMat(int dimren, int dimcol, double *mat){
    int i,j;
    for(i=0;i<dimren;i++){
	for(j=0;j<dimcol;j++){
	    printf("mat[%d] = %3.1f\t",(i*dimcol+j),mat[i*dimcol+j]);
	}
	printf("\n");
    }
    printf("\n");
}

void impMatTransp(int dimren, int dimcol, double *mat){
    int i,j;
    for(j=0;j<dimcol;j++){
	for(i=0;i<dimren;i++){
	    printf("mat[%d] = %3.1f\t",(j*dimren+i),mat[j*dimren+i]);
	}
	printf("\n");
    }
    printf("\n");
}

void* matrizAleatoria(int dimren, int dimcol,double *mat){
    int i,j;
    for(i=0;i<dimren;i++){
	for(j=0;j<dimcol;j++){
	    mat[i*dimcol+j]=rand()%5; //i*dimcol+j;
	}
    }
}

double* subMatrix(int dimren,int dimcol, int num, int nproc, double *mat){
    double *matsub;
    int i,j;
    matsub=malloc(sizeof(double)*dimren/nproc*dimcol/nproc);
    for(i=0;i<dimren/nproc;i++){
	for(j=dimcol/nproc*num;j<dimcol/nproc*(1+num);j++){
	    matsub[i*dimcol/nproc+(j-dimcol/nproc*num)]=mat[i*dimcol+j];
	}
    }
    return matsub;
}


void* zerosMat(int dimren,int dimcol,double *mat){
    int i,j;
    for(i=0;i<dimren;i++){
	for(j=0;j<dimcol;j++){
	    mat[i*dimcol+j]=0;
	}
    }
}


int mod (int a, int b){
    if(b < 0) 
	return mod(-a, -b);   
    int ret = a % b;
    if(ret < 0)
	ret+=b;
    return ret;
}

void main(int argc, char *argv[]){
    int id,np;
    int dim1,dim2,dim3;
    double *a,*b,*c;
    double *adist,*bdist,*cdist;

    MPI_Init(&argc,&argv);

    sscanf(argv[1],"%d",&dim1);
    sscanf(argv[2], "%d",&dim2);
    sscanf(argv[3],"%d",&dim3);

    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    MPI_Comm_size(MPI_COMM_WORLD, &np);

    if(id==0){
	a=malloc(sizeof(double)*dim1*dim2);
	b=malloc(sizeof(double)*dim2*dim3);
	c=malloc(sizeof(double)*dim1*dim3);
	matrizAleatoria(dim1,dim2,a);
	matrizAleatoria(dim2,dim3,b);
	printf("A = \n");
	impMat(dim1,dim2,a);
	printf("B = \n");
	impMat(dim2,dim3,b);
    }

    adist=malloc(sizeof(double)*(dim1*dim2)/np);
    bdist=malloc(sizeof(double)*(dim2*dim3)/np);


    MPI_Scatter(a,(dim1*dim2)/np,MPI_DOUBLE,adist,(dim1*dim2)/np,MPI_DOUBLE,0,MPI_COMM_WORLD);

    MPI_Scatter(b,(dim2*dim3)/np,MPI_DOUBLE,bdist,(dim2*dim3)/np,MPI_DOUBLE,0,MPI_COMM_WORLD);

    char TRANSA, TRANSB;
    int M, N, K;
    double ALPHA, BETA;

    TRANSA = 'N';
    TRANSB = 'N';
    ALPHA = 1.0;
    BETA = 1.0;

    M=dim1/np;
    K=dim2/np; 
    N=dim3;

    cdist=malloc(sizeof(double)*(dim1*dim3)/np);

    zerosMat(dim1/np,dim3,cdist);

    int j;
    double *asub;

    asub=malloc(sizeof(double)*(dim1/np*dim2/np));

    for(j=0;j<np;j++){

	asub=subMatrix(dim1,dim2,mod(id-j,np),np,adist);

	dgemm_(&TRANSA, &TRANSB, &N, &M, &K, &ALPHA, bdist, &N, asub, &K, &BETA, cdist, &N);
	/*
	   printf("---------------------------------------\n");
	   printf("A[%d,%d] = \n", id, j);
	   impMat(dim1/np,dim2/np,asub);
	//printf("Ai = \n");
	//impMat(dim1/np,dim2,adist);
	printf("B[%d, ] = \n", id);
	impMat(dim2/np,dim3,bdist);
	 */
	MPI_Send(bdist, (dim2*dim3)/np, MPI_DOUBLE, mod(id + 1,np), mod(id-j,np),MPI_COMM_WORLD);

	MPI_Recv(bdist,(dim2*dim3)/np, MPI_DOUBLE, mod(id - 1,np), mod(id-j-1,np), MPI_COMM_WORLD,MPI_STATUS_IGNORE);

    }

    printf("-----------------------------------------------------------------\nC[%d, ] = \n", id);
    impMatTransp(dim3,dim1/np,cdist);

    MPI_Gather(cdist, (dim3*dim1/np), MPI_DOUBLE, c, (dim3*dim1/np), MPI_DOUBLE, 0, MPI_COMM_WORLD);
    if(id==0){
	printf("=================================================================\n");
	printf("C = \n");
	impMatTransp(dim3, dim1, c);
	free(a);
	free(b);
    }

    free(adist);
    free(bdist);
    free(asub);

    MPI_Finalize();
}











