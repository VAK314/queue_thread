#include <pthread.h>

#ifdef DEB_QDUMP
#include <stdio.h>
#endif

#ifndef CLASS_Q
#define CLASS_Q

template <typename Tud>
struct  Q_unit
{
  Q_unit * next_u;
  Q_unit * prev_u;
  Tud u_data;
//  unit_data u_data;
};


template <typename Tud>
class Q_cl
{
public:
  Q_cl(unsigned int max_q);
  ~Q_cl(){ delete[] qu;}
  int WriteQ(Tud *pud);
  int ReadQ(Tud *pud);
  int ReadQ(Tud *pud,unsigned long long iId);
  unsigned int GetCountUse();
  unsigned int SetThreshold(unsigned int th_);
  unsigned int GetThreshold();
private:
  unsigned int ui_max_q;
  pthread_mutex_t m;
  Q_unit<Tud> * GetFreeU();
  int PutFreeU(Q_unit<Tud> * pqu);
  Q_unit<Tud> * GetU(unsigned long long iId);
  Q_unit<Tud> * GetU();
  int PutU(Q_unit<Tud> * pqu);
  Q_unit<Tud> *qu;
  Q_unit<Tud> *pFirst, *pLast;
  Q_unit<Tud> *pFirstFree, *pLastFree;
  unsigned int uiCountFree;
  unsigned int uiThreshold;
  unsigned int uiThreshold_cur;
  unsigned int uiGetRespons;
#ifdef DEB_QDUMP
  FILE* fileout = NULL;
#endif
};
#endif