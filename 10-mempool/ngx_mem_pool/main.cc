#include <iostream>

#include "ngx_mem_pool.h"

using namespace std;

int main (int argc, char *argv[]) {

    NgxMemPool mp(110);

    int arr_i_nb = 100;
    int *arr_i = (int *)mp.ngx_palloc(arr_i_nb * sizeof(int));

    for (int i = 0; i < arr_i_nb; ++i) {
        arr_i[i] = i;
    }

    int arr_i_nb2 = 100;
    int *arr_i2 = (int *)mp.ngx_palloc(arr_i_nb2 * sizeof(int));

    for (int i = 0; i < arr_i_nb2; ++i) {
        arr_i2[i] = i;
    }

    for (int i = 0; i < arr_i_nb; ++i) {
        cout << arr_i[i] << " ";
    }
    cout << endl;

    for (int i = 0; i < arr_i_nb; ++i) {
        cout << arr_i2[i] << " ";
    }
    cout << endl;

    return 0;
}
