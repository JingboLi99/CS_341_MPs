/**
 * vector
 * CS 341 - Spring 2023
 */
#include "vector.h"
#include <assert.h>
#include <stdio.h>
/**
 * 'INITIAL_CAPACITY' the initial size of the dynamically.
 */
const size_t INITIAL_CAPACITY = 8;
/**
 * 'GROWTH_FACTOR' is how much the vector will grow by in automatic reallocation
 * (2 means double).
 */
const size_t GROWTH_FACTOR = 2;

struct vector {
    /* The function callback for the user to define the way they want to copy
     * elements */
    copy_constructor_type copy_constructor;

    /* The function callback for the user to define the way they want to destroy
     * elements */
    destructor_type destructor;

    /* The function callback for the user to define the way they a default
     * element to be constructed */
    default_constructor_type default_constructor;

    /* Void pointer to the beginning of an array of void pointers to arbitrary
     * data. */
    void **array;

    /**
     * The number of elements in the vector.
     * This is the number of actual objects held in the vector,
     * which is not necessarily equal to its capacity.
     */
    size_t size;

    /**
     * The size of the storage space currently allocated for the vector,
     * expressed in terms of elements.
     */
    size_t capacity;
};

/**
 * IMPLEMENTATION DETAILS
 *
 * The following is documented only in the .c file of vector,
 * since it is implementation specfic and does not concern the user:
 *
 * This vector is defined by the struct above.
 * The struct is complete as is and does not need any modifications.
 *
 * The only conditions of automatic reallocation is that
 * they should happen logarithmically compared to the growth of the size of the
 * vector inorder to achieve amortized constant time complexity for appending to
 * the vector.
 *
 * For our implementation automatic reallocation happens when -and only when-
 * adding to the vector makes its new  size surpass its current vector capacity
 * OR when the user calls on vector_reserve().
 * When this happens the new capacity will be whatever power of the
 * 'GROWTH_FACTOR' greater than or equal to the target capacity.
 * In the case when the new size exceeds the current capacity the target
 * capacity is the new size.
 * In the case when the user calls vector_reserve(n) the target capacity is 'n'
 * itself.
 * We have provided get_new_capacity() to help make this less ambigious.
 */

static size_t get_new_capacity(size_t target) {
    /**
     * This function works according to 'automatic reallocation'.
     * Start at 1 and keep multiplying by the GROWTH_FACTOR untl
     * you have exceeded or met your target capacity.
     */
    size_t new_capacity = 1;
    while (new_capacity < target) {
        new_capacity *= GROWTH_FACTOR;
    }
    return new_capacity;
}

vector *vector_create(copy_constructor_type copy_constructor,
                      destructor_type destructor,
                      default_constructor_type default_constructor) {
    // your code here
    if (copy_constructor && destructor && default_constructor){
        //DEEP:
        void **arr = NULL; 
        arr = malloc(sizeof(void *) * INITIAL_CAPACITY);
        
        struct vector *vec = malloc(sizeof(struct vector));
        vec->copy_constructor = copy_constructor;
        vec->default_constructor = default_constructor;
        vec->array = arr;
        vec->destructor = destructor;
        vec->size = 0;
        vec->capacity = 8;

        return vec;

    }else{
        //SHALLOW:
        struct vector *shallow_vec = shallow_vector_create();
        return shallow_vec;
    }
    
    
    
}

void vector_destroy(vector *this) {
    assert(this);
    // your code here

    // needs to destroy all elements, user created or default
    if (this->array){
        size_t i = 0;
        for (;i < this->capacity; i++){
            if (this->array[i]){
                this->destructor(this->array[i]);
            }
        }
        free(this->array);
        this->array = NULL;
    }
    free(this);
    this = NULL;
}

void **vector_begin(vector *this) {
    return this->array + 0;
}

void **vector_end(vector *this) {
    return this->array + this->size;
}

size_t vector_size(vector *this) {
    assert(this);
    // your code here
    return this->size;
}

void vector_resize(vector *this, size_t n) {
    assert(this);
    // your code here
    //if the desired size if = current size, no change needed
    if (n == this->size){
        return;
    }
    // if n is smaller than current size,
    //destrou each additional item at each index of array
    else if (n < this->size){
        size_t i = n;
        for (; i < this->size; i ++){
            if (this->array[i]){
                this->destructor(this->array[i]);
                this->array[i] = NULL;
            }
        }
        //if size > n, some actual elements will have been wiped:
        this->size = n;
    }
    // n is larger than current size:
    else{
        
        // check if capacity is enough:
        if (n > this->capacity){
            //if not enough capacity, reserve more
            vector_reserve(this, n);
        }        
        //from current size limit to new size of n, create default elements
        size_t i = this->size;
        for (; i < n; i++){
            void * new_def_ele = this->default_constructor();
            this->array[i] = new_def_ele;
            this->size++;
        }
    }
}

size_t vector_capacity(vector *this) {
    assert(this);
    // your code here
    return this->capacity;
}

bool vector_empty(vector *this) {
    assert(this);
    // your code here
    return this->size == 0;
}

void vector_reserve(vector *this, size_t n) {
    assert(this);
    // your code here
    if (n > this->capacity){
        size_t new_cap = get_new_capacity(n);
        this->array = realloc(this->array, new_cap * sizeof(void *));
        this->capacity = new_cap;
    }
}

void **vector_at(vector *this, size_t position) {
    assert(this);
    // your code here
    assert(position < this->size);

    void **ele_ptr = &(this->array[position]);
    return ele_ptr;
}

void vector_set(vector *this, size_t position, void *element) {
    assert(this);
    // your code here
    assert(position < this->size); //make sure position is valid within size
    void * new_ele = this->copy_constructor(element);
    this->destructor(this->array[position]);
    this->array[position] = new_ele;

}

void *vector_get(vector *this, size_t position) {
    assert(this);
    // your code here
    assert(position < this->size); //make sure position is valid
    return this->array[position];
}

void **vector_front(vector *this) {
    assert(this);
    // your code here
    assert(!vector_empty(this));
    return vector_begin(this);
}

void **vector_back(vector *this) {
    // your code here
    assert(this);
    assert(!vector_empty(this));
    return this->array[this->size - 1];
}

void vector_push_back(vector *this, void *element) {
    assert(this);
    // your code here
    assert(this->array);
    if (this->size < this->capacity){
        // this->destructor(this->array[this->size]);
        this->array[this->size] = this->copy_constructor(element);
        this->size ++;
    }
    else{
        vector_reserve(this, this->size+1);
        // this->destructor(this->array[this->size+1]);
        this->array[this->size+1] = this->copy_constructor(element);
        this->size ++;
        //increase in capacity is already handled by reserve
    }
}

void vector_pop_back(vector *this) {
    assert(this);
    // your code here
    assert(!vector_empty(this));
    //destroy last valid element:
    this->destructor(vector_back(this));
    this->array[this->size-1] = NULL;
    this->size --;


}

void vector_insert(vector *this, size_t position, void *element) {
    assert(this);
    // your code here
    assert(position <= vector_size(this));
    //if there is no spare capacity, resize to increase capcity.
    if (this->size >= this->capacity){
        vector_reserve(this, this->capacity+1);
    }
    //if we are inserting at the last position in container size
    if (position == this->size){ 
        vector_push_back(this, element);
        //size adding is handled by vector_push_back
    }
    //if we are insert within existing elements in container size
    else{
        //destroy the default element at array[size] (last position)
        if (this->array[this->size]){
            this->destructor(this->array[this->size]);
        }
        //move each value to the next index
        size_t pos = position+1;
        void *temp1 = this->array[position];
        void *temp2 = NULL;
        for (;pos <= this->size; pos++){ //iterate to size, inclusive
            temp2 = this->array[pos];
            this->array[pos] = temp1;
            temp1 = temp2;
        }
        this->array[position] = this->copy_constructor(element);
        this->size ++;
    }

   
}

void vector_erase(vector *this, size_t position) {
    assert(this);
    assert(position < vector_size(this));
    // your code here
    //destroy element at position
    this->destructor(this->array[position]);
    size_t pos = position;
    //we leave out the last element in the iteration because its going to be set to default
    for (; pos < vector_size(this)-1; pos ++){
        this->array[pos] = this->array[pos+1];
    }
    this->array[this->size - 1] = NULL;
    this->size --;
}

void vector_clear(vector *this) {
    // your code here
    if (this){
        size_t i = 0;
        for (; i < this->size; i++){
            if (this->array[i]){
                this->destructor(this->array[i]);
                this->array[i] = NULL;
                this->size --;
            }
        }
    }
}

// The following is code generated:
vector *shallow_vector_create() {
    return vector_create(shallow_copy_constructor, shallow_destructor,
                         shallow_default_constructor);
}
vector *string_vector_create() {
    return vector_create(string_copy_constructor, string_destructor,
                         string_default_constructor);
}
vector *char_vector_create() {
    return vector_create(char_copy_constructor, char_destructor,
                         char_default_constructor);
}
vector *double_vector_create() {
    return vector_create(double_copy_constructor, double_destructor,
                         double_default_constructor);
}
vector *float_vector_create() {
    return vector_create(float_copy_constructor, float_destructor,
                         float_default_constructor);
}
vector *int_vector_create() {
    return vector_create(int_copy_constructor, int_destructor,
                         int_default_constructor);
}
vector *long_vector_create() {
    return vector_create(long_copy_constructor, long_destructor,
                         long_default_constructor);
}
vector *short_vector_create() {
    return vector_create(short_copy_constructor, short_destructor,
                         short_default_constructor);
}
vector *unsigned_char_vector_create() {
    return vector_create(unsigned_char_copy_constructor,
                         unsigned_char_destructor,
                         unsigned_char_default_constructor);
}
vector *unsigned_int_vector_create() {
    return vector_create(unsigned_int_copy_constructor, unsigned_int_destructor,
                         unsigned_int_default_constructor);
}
vector *unsigned_long_vector_create() {
    return vector_create(unsigned_long_copy_constructor,
                         unsigned_long_destructor,
                         unsigned_long_default_constructor);
}
vector *unsigned_short_vector_create() {
    return vector_create(unsigned_short_copy_constructor,
                         unsigned_short_destructor,
                         unsigned_short_default_constructor);
}
