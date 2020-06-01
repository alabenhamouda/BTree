#ifndef GUARD_HANDLE
#define GUARD_HANDLE

template <typename T>
class Handle{
    private:
        T* p;
        int* ref;
    public:
        Handle(): p{NULL}, ref{NULL} {}
        Handle(T* _p): p{_p}, ref{new int{1}} {}
        Handle(const Handle& h): p{h.p}, ref{h.ref} {++*ref;}
        ~Handle() {
            if(*ref == 1){
                delete p;
                delete ref;
            }
            else 
                --*ref;
        }
        Handle& operator =(const Handle& rhs){
            if(this != &rhs){
                if(p && p != rhs.p)
                    --*ref;
                if(*ref == 0){
                    delete p;
                    delete ref;
                }
                p = rhs.p;
                ref = rhs.ref;
                ++*ref;
            }
        }
        T& operator *() const { return *p; }
        T* operator ->() const { return p; }
};

#endif