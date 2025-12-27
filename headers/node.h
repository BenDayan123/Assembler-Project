#ifndef NODE_H
#define NODE_H

typedef struct node
{
    char *key;
    char *content;
    struct node *next;
} node;

node *create_node(char *key, char *content);
void add_node(node *head, node *new);
node *search_node(node *head, char *key, int *found);
void free_node(node *n);
void free_nodes(node *head);

void print_list(node *head);
#endif