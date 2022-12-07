class Vec3{
    float x
    float y
    float z

[public]
    constructor(float _x, float _y, float _z) => x(_x), y(_y), z(_z) {}

    operation +(Vec3 a, Vec3 b){
        a.x += b.x
        a.y += b.y
        a.z += b.z

        return a
    }
    operation *(Vec3 a, Vec3 b){
        a.x *= b.x
        a.y *= b.y
        a.z *= b.z
        
        return a
    }
    operation /(Vec3 a, Vec3 b){
        a.x /= b.x
        a.y /= b.y
        a.z /= b.z
        
        return a
    }
    operation -(Vec3 a, Vec3 b){
        a.x -= b.x
        a.y -= b.y
        a.z -= b.z
        
        return a
    }
    method toString(){
        return 'x: &{a}, y: &{b}, z: &{c}'
    }
    
}