//
//  Q_cl - class, queue for communicate between threads 
//
#include "queue_calc.h"

int main(){
    struct unit_data
    {
        unsigned long long iId;
        char bCommand;
        unsigned char diff;
        char str_ch[20];
        char str_nb[20];
        char str_hb[41];
        char end_send_byte;
        //sem_t *psem_calc;
    };


    Q_cl<unit_data> q(20);
}