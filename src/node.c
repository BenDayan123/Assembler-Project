#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../headers/node.h"
#include "../headers/utils.h"
#include "../headers/error.h"

node *create_node(char *value, char *content)
{
    node *temp = (node *)handle_malloc(sizeof(node));

    temp->value = value;
    temp->content = content;
    temp->next = NULL;

    return temp;
}

void add_node(node **head, node *new)
{
    int found = 0;
    node *search = search_node(*head, new->value, &found);
    if (found)
    {
        print_error(1); /* NEEDS TO BE CHANGED! */
        free(head);
        return;
    }
    else
    {
        if (search == NULL)
            *head = new;
        else
            search->next = new;
    }
}

node *search_node(node *head, char *value, int *found)
{
    *found = 0;
    if (head == NULL)
        return NULL;

    if (strcmp(value, head->value) == 0)
    {
        found = 1;
        printf("Node already exists with the value '%s'", value);
        return head;
    }

    if (head->next == NULL)
        return head;

    return search_node(head->next, value, found);
}

void free_node(node *n)
{
    free(n);
}

void free_nodes(node *head)
{
    while (head != NULL)
    {
        node *temp = head;
        head = head->next;
        free_node(temp);
    }
}