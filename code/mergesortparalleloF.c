/* merge sort */
#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <papi.h>


int * merge(int *A, int asize, int *B, int bsize);
void swap(int *v, int i, int j);
void m_sort(int *A, int min, int max);

long_long startT,stopT;  //tempi di esecuzione
long_long countCacheMiss;  //contatore cache miss

int EventSet = PAPI_NULL;
//unsigned int native = 0x0;
int retval;   // valore ritorno papi

//START
int main(int argc, char **argv)
{
    int * data;
    
    int * chunk;
    int * other;
    int m,n;
    int id,p;
    int s = 0;
    int i;
    int step;
    MPI_Status status;
    
    n=atoi(argv[1]);
    data = (int *)malloc(n*sizeof(int));
    
    //inizializza l'array
    for(i=0;i<n;i++)
    {
        data[i] = random()%n;
        //printf("%d ", data[i]); //VISUALIZZA ARRAY INIZIALE
    }
    //printf("\n");
    

    // PAPI: INIZIALIZZAZIONE
    if (PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT) {
        printf("Errore init PAPI\n");
        exit(1);
    }
    // PAPI: CREAZIONE EVENTSET
    if (PAPI_create_eventset(&EventSet) != PAPI_OK) {
        printf("Errore creazione EventSet PAPI\n");
        exit(1);
    }
    
    // valore di ritorno errori
    // PAPI: AGGIUNTA FUNZIONE CONTATORE CACHE MISS
    //nelle VM può ritornare errore -7 (PAPI_NOEVNT) - Event doesn't exist
    retval = PAPI_add_event(EventSet,PAPI_L2_TCM);
    printf(PAPI_strerror(retval));
    printf("\n");
    
    // PAPI: FUNZIONI DI AMBIENTE (slide10)
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&id);
    MPI_Comm_size(MPI_COMM_WORLD,&p);
    
    //serve a calcolare di quanto sarà grande il 'pezzo' per ogni processo
    s = n/p;
    
    // PAPI: inizio conteggio cache miss
    retval = PAPI_start(EventSet) != PAPI_OK;
    printf(PAPI_strerror(retval));
    printf("\n");
    //if (PAPI_start(EventSet) != PAPI_OK) {
    //    printf("Errore avvio conteggio cache miss\n");
    //    exit(1);
    //}
    
    
    // PAPI: preleva il tempo per ogni thread
    startT=PAPI_get_real_usec();
    //printf("%lld;; ", startT);
    
    // chunk è la memoria da allocare per singolo processo
    // saranno i working set dei singoli processi (gather-scatter)
    chunk = (int *)malloc(s*sizeof(int));
    // slide 30, SCATTER prende "data" lo divide in "s" parti
    // i processi ricevono "chunk" che avrà s elementi
    MPI_Scatter(data,s,MPI_INT,chunk,s,MPI_INT,0,MPI_COMM_WORLD);
    // ogni processo si ordina la parte sua
    m_sort(chunk, 0, s-1);
    
    step = 1;
    while(step<p)
    {
        if(id%(2*step)==0)
        {
            if(id+step<p)
            {
                //slide 24
                //riceve prima la dimensione da allocare dal processo sorgente 'id+step'
                MPI_Recv(&m,1,MPI_INT,id+step,0,MPI_COMM_WORLD,&status);
                //alloca lo spazio
                other = (int *)malloc(m*sizeof(int));
                //riceve tutti gli elementi
                MPI_Recv(other,m,MPI_INT,id+step,0,MPI_COMM_WORLD,&status);
                //merge ritornerà un altro array ordinato che è la somma dei due
                //cioè di chunk (s elementi) e other (m elementi)
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
  
    // PAPI: stop del timer
    stopT=PAPI_get_real_usec();
    
    // PAPI: stop dei contatori
    if(PAPI_stop(EventSet, &countCacheMiss) != PAPI_OK){
        printf("Errore in stop e store del contatore \n");
        exit(1);
    }

    // VISUALIZZA REPORT
    printf("id:%d; miss:%d; s:%d; p:%d processors; %lld ms\n",id,countCacheMiss,s,p,(stopT-startT));
    
    // VISUALIZZA ARRAY ORDINATO       
    /*
    if(id==0)
    {
        for(i=0;i<s;i++)
            printf("%d ",chunk[i]);
    }
    */
   
    MPI_Barrier(MPI_COMM_WORLD);
   
    MPI_Finalize();

}

// ALGORITMO MERGE SORT
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
