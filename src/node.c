#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../headers/node.h"
#include "../headers/utils.h"
#include "../headers/error.h"

node *create_node(char *key, char *content)
{
    node *temp = (node *)handle_malloc(sizeof(node));

    if (!key)
        key = "";
    if (!content)
        content = "";
    temp->key = (char *)malloc(strlen(key) + 1);
    temp->content = (char *)malloc(strlen(content) + 1);

    if (!(temp->key) || !(temp->content))
    {
        log_error(ERR_OPEN_FILE, 0, NULL, NULL);
        free_node(temp);
        return NULL;
    }
    strcpy(temp->key, key);
    strcpy(temp->content, content);
    temp->next = NULL;

    return temp;
}

void add_node(node *head, node *new_node)
{
    node *current = head;

    if (!head || !new_node)
        return;

    while (current->next != NULL)
        current = current->next;
    current->next = new_node;
}

node *search_node(node *head, char *key)
{
    node *current = head;

    while (current != NULL)
    {
        if (strcmp(current->key, key) == 0)
            return current;
        current = current->next;
    }
    return NULL;
}

void free_node(node *n)
{
    if (!n)
        return;
    if (n->key)
        free(n->key);
    if (n->content)
        free(n->content);
    free(n);
}

void free_nodes(node *head)
{
    node *temp;
    while (head != NULL)
    {
        temp = head;
        head = head->next;
        free_node(temp);
    }
}