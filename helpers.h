#ifndef GUARD_HELPERS
#define GUARD_HELPERS

template<typename T>
void insertInArr(T k, T* arr, int n, int idx){
    for(int i = n; i > idx; i--){
        arr[i] = arr[i - 1];
    }
    arr[idx] = k;
}

#endif