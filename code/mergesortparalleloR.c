/* merge sort */
#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <papi.h>


int * merge(int *A, int asize, int *B, int bsize);
void swap(int *v, int i, int j);
void m_sort(int *A, int min, int max);

long_long startT,stopT;
int EventSet = PAPI_NULL;
long_long countCacheMiss;

int main(int argc, char **argv)
{
    int * data;
    
    int * chunk;
    int * other;
    int m, n;
    int id, p;
    int s = 0;
    int i;
    int step;
    MPI_Status status;
    
    n=atoi(argv[1]); //numero elementi array
    data = (int *)malloc(n*sizeof(int));
  
    //creazione array da ordinare
    for(i=0;i<n;i++){
            data[i] = random()%n;
            //printf("%d ", data[i]);
    }
   // printf(" \n ");
    
   // printf("papi init\n");
    if (PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT) exit(1);
    
    /*Create an EventSet */
    if (PAPI_create_eventset(&EventSet) != PAPI_OK)  exit(1);
    
    if (PAPI_add_event(EventSet,PAPI_L1_TCM) != PAPI_OK){
            	printf("Errore nell'aggiunta dell'evento cache miss level 1\n");
            	exit(1);
    }
    
   /*EventSet tipo di misura: cache miss di L2 */
   /* if (PAPI_add_event(EventSet, PAPI_L1_TCM) != PAPI_OK){
       printf("Errore nell'aggiunta dell'evento cache miss level 2\n");
       exit(1);
    }
   */
  
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p); //assegna alla p il numero di processi
    

    s = n/p; //fraziona l'array in parti uguali ed assegna ogni parte ad un processo, in quanto p indica il numero di processi
    
    //inizio conteggio cache miss 
    if(PAPI_start(EventSet) != PAPI_OK){
    	printf("Errore nell'avvio del conteggio \n");
    	exit(1);    
    }
    
    
   	
    startT = PAPI_get_real_usec(); //prelevo il tempo 
	printf("startT: %lld   ",  startT);
	
    chunk = (int *)malloc(s*sizeof(int));
        
	MPI_Scatter(data,s,MPI_INT,chunk,s,MPI_INT,0,MPI_COMM_WORLD);
    
    m_sort(chunk, 0, s-1);
      

    step = 1;
    while(step<p)
    {
        if(id%(2*step)==0)
        {
            if(id+step<p)
            {
                MPI_Recv(&m,1,MPI_INT,id+step,0,MPI_COMM_WORLD,&status);
                other = (int *)malloc(m*sizeof(int));
                MPI_Recv(other,m,MPI_INT,id+step,0,MPI_COMM_WORLD,&status);
                chunk = merge(chunk,s,other,m);
                s = s+m;
            }
        }
        else
        {
            int near = id-step;
            MPI_Send(&s,1,MPI_INT,near,0,MPI_COMM_WORLD);
            MPI_Send(chunk,s,MPI_INT,near,0,MPI_COMM_WORLD);
            break;
        }
        step = step*2;
    }
  

    stopT=PAPI_get_real_usec(); //fine tempo
    
    printf("id processo: %d, numero elementi: %d; %d processors; %lld secs\n", id, s, p, (stopT-startT));
   
/*    if(id==0)
    {
    
        for(i=0; i<s; i++)
            printf("%d ", chunk[i]);
            
        printf(" \n");
    }
*/   
   //stop dei contatori e inserimento dei risultati in countCacheMiss
   if(PAPI_stop(EventSet, &countCacheMiss) != PAPI_OK){
   	printf("Errore in stop e store del contatore \n");
   	exit(1);
   }
   
     printf ("Sono il rank %d, questi sono i miei cache miss=%lld\n", id, countCacheMiss);
   
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
}







int * merge(int *A, int asize, int *B, int bsize) {
    int ai, bi, ci, i;
    int *C;
    int csize = asize+bsize;

    ai = 0;
    bi = 0;
    ci = 0;

    /* printf("asize=%d bsize=%d\n", asize, bsize); */

    C = (int *)malloc(csize*sizeof(int));
    while ((ai < asize) && (bi < bsize)) {
        if (A[ai] <= B[bi]) {
            C[ci] = A[ai];
            ci++; ai++;
        } else {
            C[ci] = B[bi];
            ci++; bi++;
        }
    }

    if (ai >= asize)
        for (i = ci; i < csize; i++, bi++)
            C[i] = B[bi];
    else if (bi >= bsize)
        for (i = ci; i < csize; i++, ai++)
            C[i] = A[ai];

    for (i = 0; i < asize; i++)
        A[i] = C[i];
    for (i = 0; i < bsize; i++)
        B[i] = C[asize+i];

    /* showVector(C, csize, 0); */
    return C;
}

void swap(int *v, int i, int j)
{
    int t;
    t = v[i];
    v[i] = v[j];
    v[j] = t;
}

void m_sort(int *A, int min, int max)
{
    int *C;        /* dummy, just to fit the function */
    int mid = (min+max)/2;
    int lowerCount = mid - min + 1;
    int upperCount = max - mid;

    /* If the range consists of a single element, it's already sorted */
    if (max == min) {
        return;
    } else {
        /* Otherwise, sort the first half */
        m_sort(A, min, mid);
        /* Now sort the second half */
        m_sort(A, mid+1, max);
        /* Now merge the two halves */
        C = merge(A + min, lowerCount, A + mid + 1, upperCount);
    }
}

