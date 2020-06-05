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
                // save object pointer and reference count pointer
                T* _p = p;
                int* _ref = ref;
                p = rhs.p;
                ref = rhs.ref;
                if(ref)
                    ++*ref;
                if(_p && p != _p){ // if it was pointing to an object
                    --*_ref;
                    // if that object has no references now, delete it
                    if(*_ref == 0){
                        delete _p;
                        delete _ref;
                    }
                }
            }
            return *this;
        }
        T& operator *() const { return *p; }
        T* operator ->() const { return p; }
        operator bool () const { return p; }
};

#endif