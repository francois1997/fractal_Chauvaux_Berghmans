#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "libfractal/fractal.h"
#include "libfractal/main.h"
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>
#include <stdbool.h>
#include <limits.h>
#include<stdint.h>

//#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <SDL/SDL.h>



void clean_all();
int create_all(int etat);
void printallname(struct nameacceslist *list);
int readfile(int argc, char *argv[], int begin);
void * lecture(void* parametre);
struct fractal * split(char* line);
int verifyduplicatename(char* name, struct nameacceslist *list);
int addtolistname(char* name, struct nameacceslist *list);
int removetolistname(const char* name, struct nameacceslist *list);
void freelistname(struct nameacceslist *list);;
int buf_init(struct buff *buf, int n);
void buf_clean(struct buff *buffer);
void buf_insert(struct buff *buffer, struct fractal *item);
int buf_isempty(struct buff *buffer,struct fractal **f);
void setendofprogram(struct programend *f);
int isendofprogram(struct programend *f);
int verify_end(struct buff *buffer,struct fractal **f);
int verify_endproducteur(struct buff *buffer,struct fractal **f);
int fractalhighmodify(struct fractalHigh *f, struct fractal *frac, int average);
struct fractal *getfractalhigh(struct fractalHigh *f);
struct fractal* buf_remove(struct buff *buffer);
void listthread_free(struct listthread *list);
pthread_t *removethread(struct listthread *list);
int insertthread(struct listthread *list,void* funct);
int thread_moyenne();
int thread_all();
void *producer(void *parametre);
void *producermoyenne(void *parametre);


int max_thread;
struct buff *buffer = NULL;
struct buff *listfractal = NULL;
struct nameacceslist *accesname = NULL;
struct numberlecteur *otherfile = NULL;
struct numberlecteur *otherproducteur = NULL;
struct programend *end = NULL;
struct programend *endoflecture = NULL;
struct programend *endofproducteur = NULL;
struct fractalHigh *high = NULL;
struct listthread *producerthread = NULL;
struct listthread *consumerthread = NULL;

int main(int argc, char *argv[])
{
    int err;
    if(argc <=1)
    {
          //error(err,"not enouth arguments");
          return -1;
    }
    if(strcmp(argv[1],"-d")==0)
    {
        if(strcmp(argv[2],"--maxthreads")==0)
        {
            printf("dans maxthread \n");
            max_thread = atoi(argv[3]);
            printf("max_thread vaut :%d \n",max_thread);
            if(max_thread < 1)
            {
                printf("bad max_thread number");
                //error(err,"bad max_thread number");
                clean_all();
                return -1;
            }
            create_all(1);
            err = readfile(argc,argv,4);
            if(err == -1)
            {
                printf("error during read file");
                clean_all();
                return -1;
            }
            err = thread_all();
            if(err == -1)
            {
                printf("error during thread calculating");
                clean_all();
                return -1;
            }
            clean_all();
            return 0;
        }
        else
        {
            max_thread = -1; //Si pas de nombre maximum de thread
            create_all(1);
            err = readfile(argc,argv,2);
            if(err == -1)
            {
                printf("error during read file");
                clean_all();
                return -1;
            }
            err = thread_all();
            if(err == -1)
            {
                printf("error during thread calculating");
                clean_all();
                return -1;
            }
            clean_all();
            return 0;
        }
    }
    else
    {
        if(strcmp(argv[1],"--maxthreads")==0)
        {
            max_thread = atoi(argv[2]);
            if(max_thread < 1)
            {
                printf("bad max thread number");
                //error(err,"bad max_thread number");
                clean_all();
                return -1;
            }
            create_all(2);
            err = readfile(argc,argv,3);
            if(err == -1)
            {
                printf("error during read file");
                clean_all();
                return -1;
            }
            err = thread_moyenne();
            if(err == -1)
            {
                printf("error during thread calculating");
                clean_all();
                return -1;
            }
            clean_all();
            return 0;
        }
        else
        {
            max_thread = -1;
            create_all(2);
            err = readfile(argc,argv,1);
            if(err == -1)
            {
                printf("error during read file");
                clean_all();
                return -1;
            }
            err = thread_moyenne();
            if(err == -1)
            {
                printf("error during thread calculating");
                clean_all();
                return -1;
            }
            clean_all();
           return 0;
        }
    }
    return -1;
}

void clean_all()
{
  printf("Function clean all variables \n");
  if(accesname != NULL)
  {
    freelistname(accesname);
  }
  if(listfractal != NULL)
  {
    buf_clean(listfractal);
    free(listfractal);
  }
  if(end != NULL)
  {
    sem_destroy(&(end->acces));
    free(end);
  }
  if(otherfile != NULL)
  {
    sem_destroy(&(otherfile->acces));
    free(otherfile);
  }
  if(endoflecture != NULL)
  {
    sem_destroy(&(endoflecture->acces));
    free(endoflecture);
  }
  if(producerthread != NULL)
  {
    listthread_free(producerthread);
  }
  if(buffer != NULL)
  {
    buf_clean(buffer);
  }
  if(otherproducteur != NULL)
  {
    sem_destroy(&(otherproducteur->acces));
    free(otherproducteur);
  }
  if(consumerthread != NULL)
  {
    listthread_free(consumerthread);
  }
  if(endofproducteur != NULL)
  {
    free(endofproducteur);
  }
  if(high != NULL)
  {
    if(high->high != NULL)
    {
      free(high->high);
    }
    free(high);
  }
}

int create_all(int etat)
{
  int err = 0;
    listfractal = (struct buff *)malloc(sizeof(struct buff));
    if(listfractal == NULL)
    {
      printf("Listfractal malloc fail. \n");
      return -1;
    }
    err = buf_init(listfractal, 100); //quelle taille ?
    if(err != 0)
    {
      printf("Listfractal creation fail. \n");
      free(listfractal);
      return -1;
    }





    accesname = (struct nameacceslist *)malloc(sizeof(struct nameacceslist));
    if(accesname == NULL)
    {
      printf("accesname malloc fail. \n");
      buf_clean(listfractal);
      free(listfractal);
      return -1;
    }
    accesname->head = NULL;
    err = sem_init(&(accesname->acces), 0, 1);
    if(err == -1){
      printf("semaphore init of accesname fail. \n");
      buf_clean(listfractal);
      free(listfractal);
      free(accesname);
      return -1;
    }




    end = (struct programend *)malloc(sizeof(struct programend));
    if(end == NULL)
    {
      printf("end malloc fail. \n");
      buf_clean(listfractal);
      free(listfractal);
      sem_destroy(&(accesname->acces));
      free(accesname);
      return -1;
    }
    err = sem_init(&(end->acces),0,1);
    if(err != 0)
    {
      printf("error during end semaphore creation");
      buf_clean(listfractal);
      free(listfractal);
      sem_destroy(&(accesname->acces));
      free(accesname);
      free(end);
      return -1;
    }


    otherfile = (struct numberlecteur * )malloc(sizeof(struct numberlecteur));
    if(otherfile == NULL)
    {
      printf("otherfile malloc fail. \n");
      buf_clean(listfractal);
      free(listfractal);
      sem_destroy(&(accesname->acces));
      free(accesname);
      sem_destroy(&(end->acces));
      free(end);
      return -1;
    }
    otherfile->number = 0;
    err = sem_init(&(otherfile->acces), 0, 1);
    if(err == -1){
      printf("error during otherfile semaphore creation");
      buf_clean(listfractal);
      free(listfractal);
      sem_destroy(&(accesname->acces));
      free(accesname);
      sem_destroy(&(end->acces));
      free(end);
      free(otherfile);
      return -1;
    }



    endoflecture = (struct programend *)malloc(sizeof(struct programend));
    if(endoflecture == NULL)
    {
      printf("endoflecture malloc fail. \n");
      buf_clean(listfractal);
      free(listfractal);
      sem_destroy(&(accesname->acces));
      free(accesname);
      sem_destroy(&(end->acces));
      free(end);
      sem_destroy(&(otherfile->acces));
      free(otherfile);
      return -1;
    }
    endoflecture->value = 0;
    err = sem_init(&(endoflecture->acces), 0, 1);
    if(err == -1){
      printf("error during endoflecture semaphore creation");
      buf_clean(listfractal);
      free(listfractal);
      sem_destroy(&(accesname->acces));
      free(accesname);
      sem_destroy(&(end->acces));
      free(end);
      sem_destroy(&(otherfile->acces));
      free(otherfile);
      free(endoflecture);
      return -1;
    }




    producerthread = (struct listthread *)malloc(sizeof(struct listthread));
    if(producerthread == NULL)
    {
      printf("producerthread malloc fail \n");
      buf_clean(listfractal);
      free(listfractal);
      sem_destroy(&(accesname->acces));
      free(accesname);
      sem_destroy(&(end->acces));
      free(end);
      sem_destroy(&(otherfile->acces));
      free(otherfile);
      sem_destroy(&(endoflecture->acces));
      free(endoflecture);
      return -1;
    }
    producerthread->head = NULL;
    producerthread->numberthread = 0;


    if(etat == 1) //Option avec -d
    {
      buffer = (struct buff*)malloc(sizeof(struct buff));
      if(buffer == NULL)
      {
        printf("Buffer malloc fail. \n");
        buf_clean(listfractal);
        free(listfractal);
        sem_destroy(&(accesname->acces));
        free(accesname);
        sem_destroy(&(end->acces));
        free(end);
        sem_destroy(&(otherfile->acces));
        free(otherfile);
        sem_destroy(&(endoflecture->acces));
        free(endoflecture);
        free(producerthread);
        return -1;
      }
      if(max_thread == -1)
      {
        err = buf_init(buffer, 100);
      }
      else
      {
        err = buf_init(buffer, max_thread);
      }
      if(err != 0)
      {
          printf("Error during buffer creation. \n");
          buf_clean(listfractal);
          free(listfractal);
          sem_destroy(&(accesname->acces));
          free(accesname);
          sem_destroy(&(end->acces));
          free(end);
          sem_destroy(&(otherfile->acces));
          free(otherfile);
          sem_destroy(&(endoflecture->acces));
          free(endoflecture);
          free(producerthread);
          free(buffer);
          return -1;
      }




      otherproducteur = (struct numberlecteur * )malloc(sizeof(struct numberlecteur));
      if(otherproducteur == NULL)
      {
        printf("otherproducteur malloc fail. \n");
        buf_clean(listfractal);
        free(listfractal);
        sem_destroy(&(accesname->acces));
        free(accesname);
        sem_destroy(&(end->acces));
        free(end);
        sem_destroy(&(otherfile->acces));
        free(otherfile);
        sem_destroy(&(endoflecture->acces));
        free(endoflecture);
        free(producerthread);
        buf_clean(buffer);
        return -1;
      }
      otherproducteur->number = 0;
      err = sem_init(&(otherproducteur->acces), 0, 1);
      if(err == -1){
        printf("error during otherproducteur semaphore creation");
        buf_clean(listfractal);
        free(listfractal);
        sem_destroy(&(accesname->acces));
        free(accesname);
        sem_destroy(&(end->acces));
        free(end);
        sem_destroy(&(otherfile->acces));
        free(otherfile);
        sem_destroy(&(endoflecture->acces));
        free(endoflecture);
        free(producerthread);
        buf_clean(buffer);
        free(otherproducteur);
        return -1;
      }




      consumerthread = (struct listthread *)malloc(sizeof(struct listthread));
      if(consumerthread == NULL)
      {
        printf("consumerthread malloc fail \n");
        buf_clean(listfractal);
        free(listfractal);
        sem_destroy(&(accesname->acces));
        free(accesname);
        sem_destroy(&(end->acces));
        free(end);
        sem_destroy(&(otherfile->acces));
        free(otherfile);
        sem_destroy(&(endoflecture->acces));
        free(endoflecture);
        free(producerthread);
        buf_clean(buffer);
        sem_destroy(&(otherproducteur->acces));
        free(otherproducteur);
        return -1;
      }
      consumerthread->head = NULL;
      consumerthread->numberthread = 0;


      endofproducteur = (struct programend *)malloc(sizeof(struct programend));
      if(endofproducteur == NULL)
      {
        printf("enofproducteur malloc fail \n");
        buf_clean(listfractal);
        free(listfractal);
        sem_destroy(&(accesname->acces));
        free(accesname);
        sem_destroy(&(end->acces));
        free(end);
        sem_destroy(&(otherfile->acces));
        free(otherfile);
        sem_destroy(&(endoflecture->acces));
        free(endoflecture);
        free(producerthread);
        buf_clean(buffer);
        sem_destroy(&(otherproducteur->acces));
        free(otherproducteur);
        free(consumerthread);
        return -1;
      }
      endofproducteur->value = 0;
      err = sem_init(&(endofproducteur->acces), 0, 1);
      if(err == -1)
      {
        printf("enofproducteur malloc fail \n");
        buf_clean(listfractal);
        free(listfractal);
        sem_destroy(&(accesname->acces));
        free(accesname);
        sem_destroy(&(end->acces));
        free(end);
        sem_destroy(&(otherfile->acces));
        free(otherfile);
        sem_destroy(&(endoflecture->acces));
        free(endoflecture);
        free(producerthread);
        buf_clean(buffer);
        sem_destroy(&(otherproducteur->acces));
        free(otherproducteur);
        free(consumerthread);
        free(endofproducteur);
        return -1;
      }
    }
    else //option sans -d
    {
      high = (struct fractalHigh *)malloc(sizeof(struct fractalHigh));
      if(high == NULL){
        printf("high malloc fail \n");
        buf_clean(listfractal);
        free(listfractal);
        sem_destroy(&(accesname->acces));
        free(accesname);
        sem_destroy(&(end->acces));
        free(end);
        sem_destroy(&(otherfile->acces));
        free(otherfile);
        sem_destroy(&(endoflecture->acces));
        free(endoflecture);
        free(producerthread);
        return -1;
      }
      err = sem_init(&(high->acces), 0, 1);      /* Au debut, n slots vides */
      if(err !=0)
      {
        printf("error during semaphore high creation");
        buf_clean(listfractal);
        free(listfractal);
        sem_destroy(&(accesname->acces));
        free(accesname);
        sem_destroy(&(end->acces));
        free(end);
        sem_destroy(&(otherfile->acces));
        free(otherfile);
        sem_destroy(&(endoflecture->acces));
        free(endoflecture);
        free(producerthread);
        free(high);
        return -1;
      }
    }
}


void printallname(struct nameacceslist *list)
{
  sem_wait(&(list->acces));
  struct name *current = list->head;
    while(current != NULL)
    {
      printf("%s \n",current->name);
      current = (current->next);
    }
  sem_post(&(list->acces));
}

//MALLOC : listname,listfractal(buffer), semaphore acces,
int readfile(int argc, char *argv[], int begin)
{
  int err = 0;
  pthread_t lecteur[argc-begin];
  // Il faudrai attendre la fin des thread avant de supprimer les resources
  int trynumber = 0;
  int threadreaderfail = 0;
  for(int i = begin; i < argc;i++)
  {
    char * filename = *(argv+i);
    err = pthread_create(&lecteur[i-begin],NULL,(void *)&lecture,(void *)(*(argv+i)));
    printf("%s thread create \n",filename);
    if(err != 0)
    {
      if(trynumber > 10)
      {
          threadreaderfail++;
          if(threadreaderfail > argc-begin-1)
          {
            printf("Error during reader thread creation \n");
            setendofprogram(end);
            return -1;
          }
      }
      else
      {
        trynumber++;
        i--;
      }
    }
    trynumber = 0;
    sem_wait(&(otherfile->acces));
    (otherfile->number)++;
    sem_post(&(otherfile->acces));
  }

  for(int i = begin; i < argc;i++)
  {
    err = pthread_join(lecteur[i-begin], NULL);
    sem_wait(&(otherfile->acces));
    (otherfile->number)--;
    sem_post(&(otherfile->acces));
    if(err!=0)
    {
        printf("lecture pthread end with error");
        setendofprogram(end);
    }
  }
  printallname(accesname);
  return isendofprogram(end);
}


//MALLOC : fermer fichier file
void * lecture(void* parametre)
{
  printf("%s",(char *)parametre);
  int err=0;
  char* filename = (char *)parametre;
  int file = open(filename,O_RDONLY);
  if(file==-1)
  {
    printf("error during file opening : %s",filename);
    pthread_exit(NULL);
  }
  else
  {
    int etat = 1;
    while(etat!=0)
    {
      int i = 0;
      char caractere = ' ';
      char ligne[64+32*2+2*sizeof(double)];
      while(caractere != '\n' && etat!=0)
      {
        etat = read(file,(void*)&caractere,sizeof(char));
        if(etat <= 0)
        {
          if(close(file)==-1)
          {
                  printf("error during file closing : %s",filename);
                  pthread_exit(NULL);
          }
          if(etat < 0)
          {
            printf("error during file reading : %s",filename);
          }
          pthread_exit(NULL);
        }
        printf("%c",caractere);
        *(ligne+i) = caractere;
        i++;
      }
      printf("\n");
      if((*ligne != '#') && (*ligne != '\n')) //il ne s'agit pas d'une ligne de commentaire ni d'une ligne vide
      {
        struct fractal *newfract = split(ligne);
        if(newfract == NULL)
        {
          if(close(file)==-1)
          {
                 printf("error during file closing : %s",filename);
                 pthread_exit(NULL);
          }
          printf("fail during fractal build");
          pthread_exit(NULL);
        }
        else
        {
          if(verifyduplicatename(newfract->name,accesname)==0)
          {
            buf_insert(listfractal, newfract);
            err = addtolistname(newfract->name,accesname);
            if(err != 0)
            {
              if(close(file)==-1)
              {
                      printf("error during file closing : %s",filename);
                      pthread_exit(NULL);
              }
              printf("fail during add name to list");
              pthread_exit(NULL);
            }
          }
          else
          {
            fractal_free(newfract);
          }
        }
      }
    }
    if(close(file)==-1)
    {
            printf("error during file closing : %s",filename);
            pthread_exit(NULL);
    }
    printf("fin lecture pour un thread \n");
    pthread_exit(NULL);
  }
}



//Malloc: fractal
struct fractal * split(char* line)
{
   int i = 0;
   int position=0;
   int place=0;
   int numberarg=0;
   char *splitedline[5];
   char ligne1[65];
   char ligne2[65];
   char ligne3[65];
   char ligne4[65];
   char ligne5[65];
   *splitedline = ligne1;
   *(splitedline + 1) = ligne2;
   *(splitedline + 2) = ligne3;
   *(splitedline + 3) = ligne4;
   *(splitedline + 4) = ligne5;
   while(*(line+i) != '\n' && numberarg < 6 && position<65)
   {
     if(*(line+i) == ' ')
     {
       *((*(splitedline+place))+position) = '\0';
       position = 0;
       place++;
       numberarg++;
     }
     else
     {
       *((*(splitedline+place))+position) = *(line+i);
       position++;
     }
     i++;
   }

   if(*(line+i) == '\n')
   {
     *((*(splitedline+place))+position) = '\0';
     numberarg++;
   }
   if(numberarg==5)
   {
     const char* name = *splitedline;
     int width = atoi(*(splitedline+1));
     int height = atoi(*(splitedline+2));
     double a = atof(*(splitedline+3));
     double b = atof(*(splitedline+4));
     //struct fractal *newfract;// = fractal_new(name, width, height, a, b);
     struct fractal* newfract = fractal_new(name, width, height, a, b);
     return newfract;
   }
   else
   {
     return NULL; //invalide line
   }
}



int verifyduplicatename(char* name, struct nameacceslist *list)
{
  sem_wait(&(list->acces));
  struct name *current = list->head;
  while(current != NULL)
  {
    if(strcmp((name),current->name)==0)
    {
      sem_post(&(list->acces));
      return -1;
    }
    current = (current->next);
  }
  sem_post(&(list->acces));
  return 0;
}


// MALLOC : node,
int addtolistname(char* name, struct nameacceslist *list)
{
  sem_wait(&(list->acces));
  struct name *current = list->head;
  if(current == NULL)
  {
    struct name *node = (struct name *)malloc(sizeof(struct name));
    if(node == NULL)
    {
      return -1;
    }
    node->name = name;
    node->next = NULL;
    list->head = node;
    sem_post(&(list->acces));
  }
  else
  {
    while(current->next != NULL)
    {
      current = (current->next);
    }
    struct name *node = (struct name *)malloc(sizeof(struct name));
    if(node == NULL)
    {
      return -1;
    }
    current->next = node;
    node->name = name;
    node->next = NULL;
    sem_post(&(list->acces));
  }
}



int removetolistname(const char* name, struct nameacceslist *list)
{
  sem_wait(&(list->acces));
  struct name *current = list->head;
  if(current == NULL)
  {
    sem_post(&(list->acces));
    return -1; //not found
  }
  if(strcmp(current->name,name)==0) //it's the first one
  {
    if(current->next == NULL)
    {
        free(current);
        list->head = NULL;
        sem_post(&(list->acces));
        return 0;
    }
    else
    {
      struct name *suivant = current->next;
      free(current);
      list->head = suivant;
      sem_post(&(list->acces));
      return 0;
    }
  }
  else
  {
    while(current->next != NULL)
    {
      if(strcmp(current->next->name,name)==0)
      {
        struct name *suivant = current->next->next;
        free(current->next);
        current->next = suivant;
        sem_post(&(list->acces));
        return 0;
      }
      current = (current->next);
    }
    sem_post(&(list->acces));
    return -1;  //not found
  }
}

void freelistname(struct nameacceslist *list)
{
  struct name *head = list->head;
  if(head!=NULL)
  {
    struct name *current = head;
    struct name *suivant = head->next;
    printallname(accesname);
    while(current->next != NULL)
    {
      free(current);
      current = suivant;
      suivant = current->next;
    }
    free(current->name);
    free(current);
  }
  sem_destroy(&(list->acces));
  free(list);
}



/*
 * @pre buffer!=NULL, n>0
 * @post a construit un buffer partagé contenant n slots
 * MALLOC : buf, (empty, full)->semaphore
 */
int buf_init(struct buff *buf, int n)
{
    printf("INIT ...");
    int err;
    buf->buf = (struct fractal **)malloc(sizeof(struct fractal*)*n);
    if((buf->buf)==NULL)
    {
       return -1;
    }
    buf->length = n;                       /* Buffer content les entiers */
    buf->begin = buf->end = 0;        /* Buffer vide si front == rear */
    err = sem_init(&(buf->empty), 0, n);      /* Au debut, n slots vides */
    if(err !=0)
    {
      printf("error during semaphore creation");
      free(buf->buf);
      return -1;
    }
    err = sem_init(&(buf->full), 0, 0);      /* Au debut, rien a consommer */
    if(err !=0)
    {
      printf("error during semaphore creation");
      free(buf->buf);
      sem_destroy(&(buf->empty));
      return -1;
    }
}



/*
 * @pre fract!=NULL
 * @post libere le buffer et la structure
 */
void buf_clean(struct buff *buffer)
{
    struct fractal *f;
    int ret = buf_isempty(buffer,&f);
    printf("%d",ret);
    while(ret == 0 && f != NULL)
    {
      printf("Il reste une fractal : %s \n",fractal_get_name(f));
      fractal_free(f);
      ret = buf_isempty(buffer,&f);
    }
    free(buffer->buf);
    sem_destroy(&(buffer->empty));
    sem_destroy(&(buffer->full));
    free(buffer);
}

/* @pre buffer!=NULL
 * @post ajoute item a la fin du buffer partage. Ce buffer est gere
 *       comme une queue FIFO
 */
void buf_insert(struct buff *buffer, struct fractal *item)
{
    sem_wait(&(buffer->empty));
    pthread_mutex_lock(&buffer->mutex);
    struct fractal **val = buffer->buf;
    val[((buffer->end)+1)%(buffer->length)] = item;
    buffer->end = (buffer->end + 1)%(buffer->length);
    pthread_mutex_unlock(&buffer->mutex);
    sem_post(&(buffer->full));
}

int buf_isempty(struct buff *buffer,struct fractal **f)
{
  int err = 0;
  err = sem_trywait(&(buffer->full));
  if(err != 0)
  {
    *f = NULL;
    return -1;
  }
  else
  {
    pthread_mutex_lock(&buffer->mutex);
    struct fractal *retour = ((buffer->buf)[(buffer->begin)+1]);
    ((buffer->buf)[(buffer->begin)+1]) = NULL;
    buffer->begin = (buffer->begin + 1)%(buffer->length);
    pthread_mutex_unlock(&buffer->mutex);
    sem_post(&(buffer->empty));
    *f = retour;
    return 0;
  }
}

void setendofprogram(struct programend *f)
{
  sem_wait(&(f->acces));
  f->value = -1;
  sem_post(&(f->acces));
}

int isendofprogram(struct programend *f)
{
  sem_wait(&(f->acces));
  int val = f->value;
  sem_post(&(f->acces));
  return val;
}

int verify_end(struct buff *buffer,struct fractal **f)
{

  int etat = buf_isempty(buffer,f);
  sem_wait(&(otherfile->acces));
    int etat2 = otherfile->number;
    sem_post(&(otherfile->acces));
    if(etat == 0 || etat2 != 0) //encore des fractal dans buffer ou encore des fichiers de lecture
    {
      return 0;
    }
    else
    {
      setendofprogram(endoflecture);
      printf("Fin du programme pour les producteurs \n");
      return -1;
    }
}

int verify_endproducteur(struct buff *buffer,struct fractal **f)
{
  int etat = buf_isempty(buffer,f);
  int etat2 = isendofprogram(endoflecture);
  sem_wait(&(otherproducteur->acces));
  int etat3 = otherproducteur->number;
  sem_post(&(otherproducteur->acces));
  if(etat == 0 || etat3 != 0 || etat2 == 0) //encore des fractal dans buffer ou encore des fichiers de lecture
  {
    return 0;
  }
  else
  {
    setendofprogram(endofproducteur);
    printf("Fin du programme pour les consommateurs \n");
    return -1;
  }
}


int fractalhighmodify(struct fractalHigh *f, struct fractal *frac, int average)
{
  if(frac != NULL)
  {
    printf("DESTROY %s avec comme moyenne : %d ///////////////////////////////// ",fractal_get_name(frac),average);
  }

  printallname(accesname);
  sem_wait(&(f->acces));
  if(f->average < average)
  {
    printf("MOYENNE ACTUEL DE : %d \n", f->average);
    f->average = average;
    if(f->high != NULL)
    {
      printf("SUPPRIMER ,%d \n",average);
      removetolistname(fractal_get_name(f->high), accesname);
      fractal_free(f->high);
    }
    f->high = frac;
    sem_post(&(f->acces));
    return 1;
  }
  else
  {
    //fractal_free(frac);
    sem_post(&(f->acces));
    return 0;
  }
}

struct fractal *getfractalhigh(struct fractalHigh *f)
{
  sem_wait(&(f->acces));
  struct fractal *big =  f->high;
  sem_post(&(f->acces));
  return big;
}


/* @pre sbuf!=NULL
 * @post retire le dernier item du buffer partage
 */
struct fractal* buf_remove(struct buff *buffer)
{
    sem_wait(&(buffer->full));
    pthread_mutex_lock(&buffer->mutex);
    struct fractal *retour = ((buffer->buf)[(buffer->begin)+1]);
    ((buffer->buf)[(buffer->begin)+1]) = NULL;
    buffer->begin = (buffer->begin + 1)%(buffer->length);
    pthread_mutex_unlock(&buffer->mutex);
    sem_post(&(buffer->empty));
    return retour;
}

void listthread_free(struct listthread *list)
{
  struct thread *current = list->head;
  struct thread *next;
  while(current != NULL)
  {
    next = current->next;
    free(current->thread);
    free(current);
    current = next;
  }
  free(list);
}

pthread_t *removethread(struct listthread *list)
{
  if(list == NULL)
  {
    return NULL;
  }
  struct thread *head = list->head;
  if(head == NULL)
  {
    return NULL;
  }
  list->head = head->next;
  list->numberthread = list->numberthread - 1;
  pthread_t *retour = head->thread;
  free(head);
  return retour;
}

int insertthread(struct listthread *list,void* funct)
{
  int err = 0;
  if(list == NULL)
  {
    return -2;
  }
  struct thread *head = list->head;
  pthread_t *new = (pthread_t *)malloc(sizeof(pthread_t));
  if(new == NULL)
  {
    return -1;
  }
  struct thread *add = (struct thread*)malloc(sizeof(struct thread));
  if(add == NULL)
  {
    free(new);
    return -1;
  }
  add->thread = new;
  add->next = NULL;
  if(head == NULL)
  {
    err=pthread_create(new,NULL,funct,NULL);
    if(err!=0)
    {
        free(new);
        free(add);
        printf("pthread_create fail");
        return -1;
    }
    head = add;
    list->head = head;
    list->numberthread = list->numberthread + 1;
    return 0;
  }
  else
  {
    struct thread *current = head;
    while(current->next != NULL)
    {
      current = current->next;
    }
    err=pthread_create(new,NULL,funct,NULL);
    if(err!=0)
    {
        free(new);
        free(add);
        printf("pthread_create fail");
        return -1;
    }
    current->next = add;
    list->numberthread = list->numberthread + 1;
    return 0;
  }
}


//MALLOC : buffer, buff_init
int thread_moyenne()
{
    int err;
    int number = 0;
    if(fractalhighmodify == NULL)
    {
      printf("fractalhighmodify doesn't exist. \n");
      setendofprogram(end);
      return -1;
    }
    fractalhighmodify(high,NULL,INT_MIN);
    for(int i=0;((i<max_thread || max_thread < 0) && (isendofprogram(endoflecture)==0));i++) {
        err=insertthread(producerthread,(void*)&producermoyenne);
        if(err!=0)
        {
            printf("pthread_create producter fail. \n");
            //end of programme ?
            return -1;
        }
        number++;
    }

    int value = 0;
    int numberrecup =0;
    while(value == 0)
    {
        pthread_t *current = removethread(producerthread);
        if(current == NULL)
        {
          printf("Plus de thread");
          value = -1;
        }
        else
        {
          numberrecup ++;
          err = pthread_join(*current, NULL);
          free(current);
          printf("FAIT1 :) \n");
          if(err!=0)
          {
              printf("pthread end with error");
              setendofprogram(end);
              return -1;
          }
          printf("Thread Producteur récupèré \n");
        }
    }
    struct fractal *big = getfractalhigh(high);
    if(big == NULL)
    {
      printf("No fractal high\n");
      setendofprogram(end);
      return -1;
    }
    printf("Le nom de la fractal est : %s \n",fractal_get_name(big));
    err = write_bitmap_sdl(big, fractal_get_name(big));
    if(err != 0)
    {
      printf("Error with write bitmap function");
      return -1;
    }
    printf("thread créé : %d et thread recupere  : %d \n",number,numberrecup);
    return 0;
}


//MALLOC : buffer, buff_init
int thread_all()
{
    int err = 0;
    int numberproducteur = 0;
    for(int i=0;((i<max_thread || max_thread < 0) && (isendofprogram(endoflecture)==0));i++) {
        err=insertthread(producerthread,(void*)&producer);
        if(err!=0)
        {
            printf("pthread_create fail\n");
        }
        else
        {
          sem_wait(&(otherproducteur->acces));
          (otherproducteur->number)++;
          sem_post(&(otherproducteur->acces));
          numberproducteur++;
        }
    }
    printf("numbre thread producteur créé : %d \n",numberproducteur);

    int numberconsumer = 0;
    for(int i=0;((i<max_thread || max_thread < 0) && (isendofprogram(endofproducteur)==0));i++) {
    //for(int i=0;(i<5);i++) {
        err = sem_trywait(&(buffer->full));
        if(err != 0)
        {
            //printf("pas encore de fractal à ditmapper");
            i--;
        }
        else
        {
          sem_post(&(buffer->full));
          err=insertthread(consumerthread,(void*)&consumer);
          // printf("Consommateur \n");
          if(err!=0)
          {
              printf("pthread_create fail\n");
          }
          else
          {
            numberconsumer++;
          }
        }
    }
    printf("Nombre de thread consumer créé : %d \n",numberconsumer);
    int numberproducteurrecup = 0;
    int value = 0;
    while(value == 0)
    {
        pthread_t *current = removethread(producerthread);
        if(current == NULL)
        {
          printf("Plus de thread producteur \n");
          value = -1;
        }
        else
        {
          numberproducteurrecup ++;
          err = pthread_join(*current, NULL);
          free(current);
          if(err!=0)
          {
              setendofprogram(end);
              printf("pthread end with error");
              return -1;
          }
        }
    }
    printf("numbre thread producteur récupèré : %d \n",numberproducteurrecup);


    value = 0;
    int numberconsumerrecup =0;
    while(value == 0)
    {
        pthread_t *current = removethread(consumerthread);
        if(current == NULL)
        {
          printf("Plus de thread consommateur \n");
          value = -1;
        }
        else
        {
          numberconsumerrecup ++;
          err = pthread_join(*current, NULL);
          free(current);
          if(err!=0)
          {
              setendofprogram(end);
              printf("pthread end with error");
              return -1;
          }
        }
    }
    printf("numbre thread consommateur récupèré : %d \n",numberconsumerrecup);
    return 0;
}



// Producteur
void *producer(void *parametre)
{
  struct fractal *f;
  while((isendofprogram(endoflecture) == 0) && (isendofprogram(end)== 0) && (verify_end(listfractal,&f) == 0))
  {
        printf("FRACTAL : %s \n",fractal_get_name(f));
        int val;
        for(int a=0; a<(fractal_get_width(f)*fractal_get_height(f));a++)
        {
            int x = a % (fractal_get_width(f));
            int y = a/(fractal_get_width(f));
            val = fractal_compute_value(f, x, y);
            fractal_set_value(f,x,y,val);
        }
        buf_insert(buffer, f);
        printf("Une fractal terminee avec succes \n");
  }
  sem_wait(&(otherproducteur->acces));
  (otherproducteur->number)--;
  sem_post(&(otherproducteur->acces));
  pthread_exit(NULL);
}

// Producteur
void *producermoyenne(void *parametre)
{
  printf("Producermoyenne \n");
  struct fractal *fractalhigh;
  int sum = 0;
  struct fractal *f;
  while((isendofprogram(endoflecture) == 0) && (isendofprogram(end) == 0) && (verify_end(listfractal,&f) == 0))
  {
    if(f != NULL)
    {
        //struct fractal *f = buf_remove(listfractal);
        int val;
        sum = 0;
        printf("largeur = %d et longueur = %d",fractal_get_width(f),(fractal_get_height(f)));
        for(int a=0; a<(fractal_get_width(f))*(fractal_get_height(f));a++)
        {
            int x = a % (f->width);
            int y = a/(f->width);
            val = fractal_compute_value(f, x, y);
            sum = sum + val;
            fractal_set_value(f,x,y,val);
        }
        sum = sum / (fractal_get_width(f))*(fractal_get_height(f));
        int retour = fractalhighmodify(high,f,sum);
        if(retour == 0)
        {
          removetolistname(fractal_get_name(f), accesname);
          fractal_free(f);
        }
        printf("Une fractal terminee avec succes \n");
     }
  }
  pthread_exit(NULL);
}



// Consommateur
void *consumer(void *parametre)
{
 struct fractal *fract;
 while((isendofprogram(endofproducteur) == 0) && (isendofprogram(end)  == 0) && (verify_endproducteur(buffer,&fract) == 0))
 {
   if(fract != NULL)
   {
     printf("une fractal bitmaper de nom : %s \n",fractal_get_name(fract));
     write_bitmap_sdl(fract, fractal_get_name(fract));
     removetolistname(fractal_get_name(fract), accesname);
     fractal_free(fract);
   }
 }
 pthread_exit(NULL);
}
