#ifndef NODE_H
#define NODE_H

typedef struct
{
    char *value;
    char *content;
    struct node *next;
} node;

node *create_node(char *value, char *content);
void add_node(node **head, node *new);
node *search_node(node *head, char *value, int *found);
void free_node(node *n);
void free_nodes(node *head);

#endif