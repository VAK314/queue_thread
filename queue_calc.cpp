#include <string.h>
#include "queue_calc.h"

template <typename Tud>
Q_cl<Tud>::Q_cl(unsigned int max_q)
{
#ifdef DEB_QDUMP
  char sNameFile[100];
  sprintf(sNameFile,"srv_Q_%d.log",max_q);
  fileout = fopen(sNameFile, "a");
  if (fileout) setbuf(fileout, NULL); // unbuffered
  fprintf(fileout,"new q size=%d",max_q);
#endif

  int i;
  ui_max_q = max_q;   
  qu = new  Q_unit<Tud>[ui_max_q];
  for(i=1;i<ui_max_q;i++)
  {
    memset(&(qu[i].u_data),0,sizeof(Tud));
    qu[i-1].next_u=&qu[i];
    qu[i].prev_u=&qu[i-1];
  }

  qu[0].prev_u=NULL;
  pFirstFree=&qu[0];
  pLastFree=&qu[ui_max_q-1];
  qu[ui_max_q-1].next_u=NULL;
  uiCountFree = ui_max_q;
  uiThreshold     = 0;
  uiThreshold_cur = 0;
  uiGetRespons    = 1;
  pFirst=NULL;
  pLast=NULL;
  pthread_mutex_init(&m,NULL);

}

template <typename Tud>
unsigned int Q_cl<Tud>::SetThreshold(unsigned int th_)
{
  pthread_mutex_lock(&m);
   uiThreshold = th_;
   uiThreshold_cur = th_;
#ifdef DEB_QDUMP
  fprintf(fileout,"set Threshold=%d\n",uiThreshold);
#endif
  pthread_mutex_unlock(&m);
};

template <typename Tud>
unsigned int Q_cl<Tud>::GetThreshold()
{
  return uiThreshold;
};


template <typename Tud>
unsigned int Q_cl<Tud>::GetCountUse()
{
  int iResult;
  pthread_mutex_lock(&m);
  iResult = ui_max_q - uiCountFree;
  pthread_mutex_unlock(&m);
  return iResult;
};

template <typename Tud>
Q_unit<Tud> * Q_cl<Tud>::GetFreeU()
{
  Q_unit<Tud> * pqu = NULL;

  pthread_mutex_lock(&m);
  if((uiCountFree>0)&&(pFirstFree!=NULL))
  {
    pqu = pFirstFree;
    pFirstFree = pqu->next_u;
    pqu->next_u=NULL;
    uiCountFree--;
    if((uiThreshold>0)&&(uiThreshold_cur!=uiThreshold)&&((ui_max_q - uiCountFree)>uiThreshold_cur*2))
    {
	uiGetRespons=1;
	if(uiThreshold_cur*2>uiThreshold)
	{
	    uiThreshold_cur=uiThreshold;
#ifdef DEB_QDUMP
  fprintf(fileout,"MAX Threshold_cur=%d\n",uiThreshold_cur);
#endif
	}
	else
	{
	    uiThreshold_cur=uiThreshold_cur*2;
#ifdef DEB_QDUMP
  fprintf(fileout,"UP Threshold_cur=%d\n",uiThreshold_cur);
#endif
	}
    }
  }
  pthread_mutex_unlock(&m);
  return pqu;
}

template <typename Tud>
int Q_cl<Tud>::PutFreeU(Q_unit<Tud> *pqu)
{
  int i_result;
  pthread_mutex_lock(&m);
  if((uiCountFree<ui_max_q)&&(pqu!=NULL))
  {
    pqu->next_u = pFirstFree;
    if(pFirstFree!=NULL)
    {
      pFirstFree->prev_u = pqu;
    }
    else
    {
      pLastFree=pqu;
    }
    pFirstFree=pqu;
    uiCountFree++;
    i_result =  1;
    if((uiThreshold>0)&&((ui_max_q - uiCountFree)<uiThreshold_cur))
    {
#ifdef DEB_QDUMP
  fprintf(fileout,"DOWN (%d) Threshold_cur=%d=>",(ui_max_q - uiCountFree),uiThreshold_cur);
#endif
      if(uiThreshold_cur>1) uiThreshold_cur=uiThreshold_cur/2;
      if(uiGetRespons==1)
      {
	uiGetRespons=0;
        i_result =  2;
      }
      else
      {
	i_result =  3;
      }
#ifdef DEB_QDUMP
  fprintf(fileout,"%d\n",uiThreshold_cur);
#endif
    }
  }
  else
  {
    i_result = 0;
  }
  pthread_mutex_unlock(&m);
  return i_result;
}

template <typename Tud>
Q_unit<Tud> * Q_cl<Tud>::GetU(unsigned long long iId)
{
  Q_unit<Tud> * pqu = NULL;
  pthread_mutex_lock(&m);
  Q_unit<Tud> * pSearch = pFirst;
  while(pSearch!=NULL)
  {
    if(pSearch->u_data.iId==iId)
    {
      pqu = pSearch;
      if(pSearch==pFirst)
      {
        pFirst = pqu->next_u;
        pqu->next_u=NULL;
      }
      else
      {
        if(pSearch->next_u!=NULL)
        { 
           pSearch->next_u->prev_u=pSearch->prev_u;
        }
        else
        {
          pLast=pSearch->prev_u;
        }
        pSearch->prev_u->next_u=pSearch->next_u; 

      }
      break;
    }
    else
    {
      pSearch=pSearch->next_u;
    }
  }
  pthread_mutex_unlock(&m);
  return pqu;
}

template <typename Tud>
Q_unit<Tud> * Q_cl<Tud>::GetU()
{
  Q_unit<Tud> * pqu = NULL;
  pthread_mutex_lock(&m);
  if(pFirst!=NULL)
  {
    pqu = pFirst;
    pFirst = pqu->next_u;
    pqu->next_u=NULL;
  }
  pthread_mutex_unlock(&m);
  return pqu;
}

template <typename Tud>
int Q_cl<Tud>::PutU(Q_unit<Tud> *pqu)
{
  int i_result;
  pthread_mutex_lock(&m);
  if(pqu!=NULL)
  {
    if(pFirst==NULL)
    {
      pFirst=pqu;
      pqu->prev_u=NULL;
    }
    else
    {
      pqu->prev_u=pLast;
      pLast->next_u=pqu;
    }
    pqu->next_u=NULL;
    pLast=pqu;
    i_result = 1;
  }
  else
  {
    i_result = 0;
  }
  pthread_mutex_unlock(&m);
  return i_result;
}

template <typename Tud>
int Q_cl<Tud>::WriteQ(Tud *pud)
{
  Q_unit<Tud> *pqu;
  if((pqu=GetFreeU())!=NULL)
  {
    memcpy(&(pqu->u_data),pud,sizeof(Tud));
    PutU(pqu);
    return 1;
  }
  return 0;
}

template <typename Tud>
int Q_cl<Tud>::ReadQ(Tud *pud)
{
  Q_unit<Tud> *pqu;
  if((pqu=GetU())!=NULL&&(pud!=NULL))
  {
    memcpy(pud,&(pqu->u_data),sizeof(Tud));
    return PutFreeU(pqu);
  }
  else
  {
    return 0;
  }
}

template <typename Tud>
int Q_cl<Tud>::ReadQ(Tud *pud,unsigned long long iId)
{
  Q_unit<Tud> *pqu;
  if((pqu=GetU(iId))!=NULL&&(pud!=NULL))
  {
    memcpy(pud,&(pqu->u_data),sizeof(Tud));
    return PutFreeU(pqu);
  }
  else
  {
    return 0;
  }
}


int main(){

}