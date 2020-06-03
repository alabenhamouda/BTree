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
        Handle(const Handle& h): p{h.p}, ref{h.ref} {
            if(ref)
                ++*ref;
        }
        ~Handle() {
            if(ref && *ref == 1){
                delete p;
                delete ref;
            }
            else if (ref)
                --*ref;
        }
        Handle& operator =(const Handle& rhs){
            if(this != &rhs){
                if(rhs.ref)
                    ++*rhs.ref;
                if(p && p != rhs.p)
                    --*ref;
                if(ref && *ref == 0){
                    delete p;
                    delete ref;
                }
                p = rhs.p;
                ref = rhs.ref;
            }
            return *this;
        }
        T& operator *() const { return *p; }
        T* operator ->() const { return p; }
        operator bool () const { return p; }
};

#endif