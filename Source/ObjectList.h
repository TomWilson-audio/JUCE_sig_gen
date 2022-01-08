/*
  ==============================================================================

    ObjectCounter.h
    Created: 8 Jan 2022 5:30:07pm
    Author:  Tom Wilson

  ==============================================================================
*/

/*
 *  Maintains a Static List of (pointers to object) and Counter for All Instances of an Object type (Configure by Inheriting Object Count as a base class).
 */
#pragma once
template <typename T>
class ObjectList{
public:
    ObjectList(){
        ++count;
    }
    ~ObjectList(){
        --count;
        ObjectArray.clear();
    }
    unsigned int GetObjectInstanceCount(void){ return count; }
    
    void AddObjectToList( T* Object ){ ObjectArray.push_back(Object); }
    
    T* GetObjectFromList( unsigned int instance ){
        if( instance >= count ) return NULL;
        return ObjectArray.at(instance);
    }
private:
    static unsigned int count;
    static std::vector<T*> ObjectArray;
};

template<typename T>
unsigned int ObjectList<T>::count = 0;        //Zero Init the counter
template<typename T>
std::vector<T*> ObjectList<T>::ObjectArray;
