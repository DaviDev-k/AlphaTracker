#ifndef STACK_H
#define STACK_H

struct track { int Ax, Ay, Bx, By, len, dir, frm; };

struct node {
    int id;
    int count;
    track ray;
    bool alive;
    int life;
    node *next;
};

typedef node* stack;

void init      (stack &s);
void deinit    (stack &s);
void push      (stack &s, const track ray);
bool edit      (stack &s, int id, int counter);
bool edit      (stack &s, int id, bool alive);
bool edit      (stack &s, int id, const track ray);
bool pop       (stack &s);
bool pop       (stack &s, int id);
bool empty     (const stack &s);
bool top_count (const stack &s, int &count);
bool top_id    (const stack &s, int &id);
bool top_ray   (const stack &s, track &ray);
bool dig_life  (const stack &s, int id, int &life);
bool dig_count (const stack &s, int id, int &count);
bool dig_ray   (const stack &s, int id, track &ray);
bool count_act (const stack &s, int &count);
void print     (const stack &s);
void print     (const stack &s, int x, int y, int dim);
/*void print     (const stack &s, int id);*/
void save      (const stack &s, char path[]);


#endif // STACK_H
