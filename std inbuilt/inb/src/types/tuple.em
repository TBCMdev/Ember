# tuples only support 4 values.
class<T, T1, T2?, T3?> tuple {
    T i1
    T1 i2,
    T2 i3 = null
    T3 i4 = null
[public]
    constructor(T _i1, T1 _i2, T2 _i3 = null, T3 _i4 = null) => i1(_i1), i2(_i2), i3(_i3), i4(_i4){

    }
    method get(int i) {
        return [i1, i2, i3, i4][i]
    }
    
}